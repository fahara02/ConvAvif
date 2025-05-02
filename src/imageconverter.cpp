#include "imageconverter.h"
#include <avif/avif.h>
#include <cmath>
#include <cstdlib>
#include <emscripten/bind.h>       // for emscripten::val, typed_memory_view
#include <emscripten/emscripten.h> // for EM_ASM
#include <emscripten/heap.h>
#include <emscripten/threading.h>
#include <emscripten/val.h>
#include <stb_image.h>
#include <stb_image_resize2.h>
#include <vector>

thread_local const emscripten::val Uint8Array =
    emscripten::val::global("Uint8Array");

Result convert_image(const std::string &input_data, int width, int height,
                     const EncodeConfig &config) {

  // Before resizing
  if (width <= 0 || height <= 0 || width > 8192 || height > 8192) {
    return Result::Failure(ConverterError::INVALID_DIMENSIONS,
                           "Invalid dimensions (1-8192px allowed)", __func__);
  }
  // Decode input image
  int w, h, channels;
  unsigned char *data = stbi_load_from_memory(
      reinterpret_cast<const stbi_uc *>(input_data.data()), input_data.size(),
      &w, &h, &channels, STBI_rgb_alpha);
  if (!data)
    return Result::Failure(ConverterError::IMAGE_LOAD_FAILED,
                           "STB image load failed", __func__);

  // Resize image to RGBA
  std::vector<unsigned char> resized_data(width * height * 4);
  printf("Resized buffer: %dx%d@4ch (expected %zu bytes)\n", width, height,
         resized_data.size());
  stbir_resize_uint8_linear(data, w, h, 0, resized_data.data(), width, height,
                            0, STBIR_RGBA);
  stbi_image_free(data);

  // Configure AVIF encoder
  avifEncoder *encoder = avifEncoderCreate();
  // Inside convert_image() after creating encoder
  if (!encoder) {
    // Handle encoder creation failure
    return Result::Failure(ConverterError::ENCODER_CREATION_FAILED,
                           "Failed to create AVIF encoder", __func__);
  }

  // Set the codec choice based on config.codecChoice
  switch (config.codecChoice) {
  case EncodeConfig::AOM:
    encoder->codecChoice = AVIF_CODEC_CHOICE_AOM;
    break;
  case EncodeConfig::SVT:
    encoder->codecChoice = AVIF_CODEC_CHOICE_SVT;
    break;
  case EncodeConfig::AUTO:
  default:
    encoder->codecChoice = AVIF_CODEC_CHOICE_AUTO;
    break;
  }

  switch (config.tune) {
  case EncodeConfig::TUNE_PSNR:
    avifEncoderSetCodecSpecificOption(encoder, "tune", "psnr");
    break;
  case EncodeConfig::TUNE_SSIM:
    avifEncoderSetCodecSpecificOption(encoder, "tune", "ssim");
    break;
  default:
    break;
  }
  encoder->maxThreads = emscripten_num_logical_cores();
  encoder->quality = config.quality;
  encoder->qualityAlpha =
      (config.qualityAlpha == -1) ? config.quality : config.qualityAlpha;
  encoder->speed = config.speed;

  // Apply sharpness (0-7) via codec-specific option
  avifEncoderSetCodecSpecificOption(encoder, "sharpness",
                                    std::to_string(config.sharpness).c_str());

  // Map percent quality (0-100) to libavif quantizer (0=best..63=worst)
  int q = static_cast<int>(
      std::round((100 - config.quality) * MAX_QUANTIZER / 100.0));
  encoder->minQuantizer = q;
  encoder->maxQuantizer = q;
  encoder->minQuantizerAlpha = q;
  encoder->maxQuantizerAlpha = q;
  encoder->tileRowsLog2 = config.tileRowsLog2;
  encoder->tileColsLog2 = config.tileColsLog2;
  // Add a warning for low speed values
  if (config.speed < 5 && encoder->codecChoice == AVIF_CODEC_CHOICE_AOM) {
    printf("Warning: Using AOM codec with speed < 5 may require more memory "
           "than available in some WASM environments\n");
    encoder->maxThreads = 1; // Force single-threaded operation
    avifEncoderSetCodecSpecificOption(encoder, "row-mt", "0");
    avifEncoderSetCodecSpecificOption(encoder, "tile-columns", "0");
    avifEncoderSetCodecSpecificOption(encoder, "tile-rows", "0");
    avifEncoderSetCodecSpecificOption(encoder, "frame-parallel", "0");
  }

  // avifEncoderSetCodecSpecificOption(encoder, "cpu-used",
  //                                   std::to_string(config.speed).c_str());
  // avifEncoderSetCodecSpecificOption(encoder, "auto-alt-ref", "1");
  // avifEncoderSetCodecSpecificOption(encoder, "lag-in-frames", "0");

  avifImage *image;
  try {
    image = avifImageCreate(width, height, RGB_DEPTH, config.pixelFormat);

  } catch (const std::exception &e) {
    avifEncoderDestroy(encoder);
    return Result::Failure(ConverterError::OUT_OF_MEMORY,
                           "Memory allocation failed for AOM encoder" +
                               std::string(e.what()),
                           __func__);
  }
  // Set color profile to sRGB/BT.709
  image->colorPrimaries = AVIF_COLOR_PRIMARIES_BT709;
  image->transferCharacteristics = AVIF_TRANSFER_CHARACTERISTICS_SRGB;
  image->matrixCoefficients = AVIF_MATRIX_COEFFICIENTS_BT709;
  image->yuvRange = AVIF_RANGE_FULL; // Use full range for web content

  avifRGBImage rgb;
  avifRGBImageSetDefaults(&rgb, image);
  rgb.chromaUpsampling = AVIF_CHROMA_UPSAMPLING_AUTOMATIC;
  rgb.format = AVIF_RGB_FORMAT_RGBA;

  rgb.depth = RGB_DEPTH;
  rgb.pixels = resized_data.data();
  rgb.rowBytes = width * avifRGBImagePixelSize(&rgb);
  rgb.alphaPremultiplied = AVIF_FALSE;
  rgb.ignoreAlpha = false;

  avifResult convertResult = avifImageRGBToYUV(image, &rgb);
  if (convertResult != AVIF_RESULT_OK) {
    avifEncoderDestroy(encoder);
    avifImageDestroy(image);
    return Result::Failure(ConverterError::CONVERSION_FAILED,
                           "RGB->YUV conversion failed: " +
                               std::string(avifResultToString(convertResult)),
                           __func__);
  }

  avifRWData output = AVIF_DATA_EMPTY;
  avifResult result;
  try {
    result = avifEncoderWrite(encoder, image, &output);
    if (result != AVIF_RESULT_OK) {
      std::string errorMsg = "Encoding failed: ";
      errorMsg += avifResultToString(result);
      if (result == AVIF_RESULT_OUT_OF_MEMORY) {
        errorMsg += " - Try higher speed values (5-10)";
      }
      return Result::Failure(avifToConverterError(result), errorMsg, __func__);
    }
  } catch (const std::exception &e) {
    // Handle any unexpected exceptions during encoding
    avifEncoderDestroy(encoder);
    avifImageDestroy(image);
    return Result::Failure(
        ConverterError::ENCODING_FAILED,
        "Exception during AVIF encoding: " + std::string(e.what()), __func__);
  }

  printf("From cpp AVIF output size: %zu bytes\n",
         output.size); // Log output size
  avifEncoderDestroy(encoder);
  avifImageDestroy(image);

  // Copy AVIF data into a vector (ensures ownership)
  std::vector<uint8_t> output_data(output.data, output.data + output.size);
  avifRWDataFree(&output); // Free libavif's buffer

  return Result::Success(std::make_shared<ImageBuffer>(std::move(output_data)));
}

// New wrapper for JavaScript that directly returns a Uint8Array without complex
// class wrapping
emscripten::val convertImageDirect(emscripten::val jsData, int width,
                                   int height, const EncodeConfig &config) {
  try {
    const auto length = jsData["length"].as<size_t>();
    std::vector<unsigned char> inputData(length);
    emscripten::val memoryView = emscripten::val(
        emscripten::typed_memory_view(inputData.size(), inputData.data()));
    memoryView.call<void>("set", jsData);

    std::string str(inputData.begin(), inputData.end());

    // Create the return object structure
    emscripten::val result = emscripten::val::object();
    result.set("success", false); // Default to false, set true on success

    try {
      auto avifResult = convert_image(str, width, height, config);

      if (avifResult.error) {
        // Conversion failed with a specific error
        emscripten::val errorObj = emscripten::val::object();
        errorObj.set("code", static_cast<int>(avifResult.error->code));
        errorObj.set("message", avifResult.error->message);
        result.set("error", errorObj);
        return result;
      }

      if (!avifResult.image || !avifResult.image->getData() ||
          avifResult.image->getSize() == 0) {
        // Image data is missing or empty
        emscripten::val errorObj = emscripten::val::object();
        errorObj.set("code", static_cast<int>(ConverterError::UNKNOWN_ERROR));
        errorObj.set("message", "Image data is empty or missing");
        result.set("error", errorObj);
        return result;
      }

      // Extract the data from ImageBuffer to a JavaScript TypedArray
      const std::vector<uint8_t> &imageData = avifResult.image->getRawData();
      auto dataView = emscripten::val(
          emscripten::typed_memory_view(imageData.size(), imageData.data()));

      // Create a new Uint8Array and copy the data
      auto uint8Array =
          emscripten::val::global("Uint8Array").new_(imageData.size());
      uint8Array.call<void>("set", dataView);

      // Set the success data
      result.set("success", true);
      result.set("data", uint8Array);
      result.set("size", imageData.size());

      return result;
    } catch (const std::bad_alloc &e) {
      // Specific handling for memory allocation errors (common with low encoder
      // speeds)
      emscripten::val errorObj = emscripten::val::object();
      errorObj.set("code", static_cast<int>(ConverterError::OUT_OF_MEMORY));
      errorObj.set(
          "message",
          "Memory allocation failed - try using a higher speed value (5-10)");
      result.set("error", errorObj);
      return result;
    } catch (const std::exception &e) {
      // General exception handler
      emscripten::val errorObj = emscripten::val::object();
      errorObj.set("code", static_cast<int>(ConverterError::UNKNOWN_ERROR));
      errorObj.set("message", std::string("Exception: ") + e.what());
      result.set("error", errorObj);
      return result;
    }
  } catch (const std::exception &e) {
    // Outer try-catch for handling errors in setup/parameter processing
    emscripten::val result = emscripten::val::object();
    result.set("success", false);

    emscripten::val errorObj = emscripten::val::object();
    errorObj.set("code", static_cast<int>(ConverterError::UNKNOWN_ERROR));
    errorObj.set("message", std::string("Setup error: ") + e.what());
    result.set("error", errorObj);

    return result;
  }
}

EMSCRIPTEN_BINDINGS(ImageConverter) {

  emscripten::class_<EncodeConfig>("EncodeConfig")
      .constructor<>()
      .property("quality", &EncodeConfig::getQuality, &EncodeConfig::setQuality)
      .property("speed", &EncodeConfig::speed)
      .property("pixelFormat", &EncodeConfig::pixelFormat)
      .property("codecChoice", &EncodeConfig::codecChoice)
      .property("minQuantizer", &EncodeConfig::getMinQuantizer,
                &EncodeConfig::setMinQuantizer)
      .property("maxQuantizer", &EncodeConfig::getMaxQuantizer,
                &EncodeConfig::setMaxQuantizer)
      .property("tileRowsLog2", &EncodeConfig::getTileRowsLog2,
                &EncodeConfig::setTileRowsLog2)
      .property("tileColsLog2", &EncodeConfig::getTileColsLog2,
                &EncodeConfig::setTileColsLog2)
      .property("tune", &EncodeConfig::tune)
      .property("sharpness", &EncodeConfig::sharpness);
  emscripten::enum_<EncodeConfig::CodecChoice>("CodecChoice")
      .value("AUTO", EncodeConfig::CodecChoice::AUTO)
      .value("AOM", EncodeConfig::CodecChoice::AOM)
      .value("SVT", EncodeConfig::CodecChoice::SVT);

  emscripten::enum_<EncodeConfig::Tune>("Tune")
      .value("DEFAULT", EncodeConfig::TUNE_DEFAULT)
      .value("PSNR", EncodeConfig::TUNE_PSNR)
      .value("SSIM", EncodeConfig::TUNE_SSIM);

  emscripten::function("convertImageDirect", &convertImageDirect);
}

EMSCRIPTEN_BINDINGS(avif_enums) {
  emscripten::enum_<avifPixelFormat>("AvifPixelFormat")
      .value("YUV444", AVIF_PIXEL_FORMAT_YUV444)
      .value("YUV422", AVIF_PIXEL_FORMAT_YUV422)
      .value("YUV420", AVIF_PIXEL_FORMAT_YUV420)
      .value("YUV400", AVIF_PIXEL_FORMAT_YUV400);

  emscripten::enum_<avifCodecChoice>("AvifCodecChoice")
      .value("AUTO", AVIF_CODEC_CHOICE_AUTO)
      .value("AOM", AVIF_CODEC_CHOICE_AOM)
      .value("SVT", AVIF_CODEC_CHOICE_SVT)
      .value("DAV1D", AVIF_CODEC_CHOICE_DAV1D);
}

EMSCRIPTEN_BINDINGS(ErrorHandling) {
  emscripten::enum_<ConverterError>("ErrorCode")
      .value("OK", ConverterError::OK)
      .value("INVALID_DIMENSIONS", ConverterError::INVALID_DIMENSIONS)
      .value("IMAGE_LOAD_FAILED", ConverterError::IMAGE_LOAD_FAILED)
      .value("ENCODER_CREATION_FAILED", ConverterError::ENCODER_CREATION_FAILED)
      .value("CONVERSION_FAILED", ConverterError::CONVERSION_FAILED)
      .value("ENCODING_FAILED", ConverterError::ENCODING_FAILED)
      .value("INVALID_ARGUMENT", ConverterError::INVALID_ARGUMENT)
      .value("OUT_OF_MEMEORY", ConverterError::OUT_OF_MEMORY)
      .value("INVALID_QUANTIZER_VALUES",
             ConverterError::INVALID_QUANTIZER_VALUES)
      .value("UNKNOWN_ERROR", ConverterError::UNKNOWN_ERROR)
      .value("AVIF_ERROR_START", ConverterError::AVIF_ERROR_START);

  emscripten::class_<Error>("Error")
      .property("code", &Error::code)
      .property("message", &Error::message)
      .property("stackTrace", &Error::stackTrace);

  emscripten::class_<Result>("Result")
      .function("getImage", &Result::getImage,
                emscripten::allow_raw_pointer<ImageBuffer>())
      .function("getError", &Result::getError,
                emscripten::allow_raw_pointer<Error>());
}