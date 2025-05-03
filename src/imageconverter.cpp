/**
 * @file imageconverter.cpp
 * @brief Implementation of image conversion functionality to AVIF format
 * @author ConvAvif Developer
 * @date 2025
 */
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

/**
 * @brief Thread-local Uint8Array reference from JavaScript
 * 
 * Used for efficient data transfer between C++ and JavaScript
 */
thread_local const emscripten::val Uint8Array =
    emscripten::val::global("Uint8Array");

/**
 * @brief Converts an image to AVIF format with specified parameters
 * 
 * This function handles the complete conversion pipeline:
 * 1. Validates input dimensions
 * 2. Loads and decodes input image data
 * 3. Resizes image if necessary
 * 4. Configures the AVIF encoder based on provided settings
 * 5. Converts RGB data to YUV colorspace
 * 6. Encodes image to AVIF format
 * 
 * @param type The type of input image (e.g., PNG, JPEG)
 * @param input_data Vector containing the raw image data
 * @param width Desired width of the output image
 * @param height Desired height of the output image
 * @param config Encoding configuration parameters
 * 
 * @return Result object containing either the encoded image data or an Error
 */
Result convert_image(ImageType type, const std::vector<uint8_t> &input_data,
                     int width, int height, const EncodeConfig &config) {

  // Input validation for image dimensions
  if (width <= 0 || height <= 0 || width > 8192 || height > 8192) {
    return Error(ConverterError::INVALID_DIMENSIONS,
                 "Invalid dimensions (1-8192px allowed)", __func__);
  }

  // Load image using STB library
  int w, h, channels;
  unsigned char *data = stbi_load_from_memory(
      input_data.data(), input_data.size(), &w, &h, &channels, STBI_rgb_alpha);
  if (!data)
    return Error(ConverterError::IMAGE_LOAD_FAILED, "STB image load failed",
                 __func__);

  // Resize image if dimensions don't match requested size
  std::vector<unsigned char> processed_data;
  if (w == width && h == height) {
    processed_data.assign(data, data + (w * h * 4));
  } else {
    processed_data.resize(width * height * 4);
    stbir_resize_uint8_linear(data, w, h, 0, processed_data.data(), width,
                              height, 0, STBIR_RGBA);
  }

  stbi_image_free(data);
  config.printConfig();

  // Create and configure the AVIF encoder
  UniqueAvifEncoder encoder(avifEncoderCreate());
  if (!encoder) {
    return Error(ConverterError::ENCODER_CREATION_FAILED,
                 "Failed to create AVIF encoder", __func__);
  }

  // Set codec choice based on configuration
  switch (config.codecChoice) {
  case CodecChoice::AOM:
    encoder->codecChoice = AVIF_CODEC_CHOICE_AOM;
    break;
  case CodecChoice::SVT:
    encoder->codecChoice = AVIF_CODEC_CHOICE_SVT;
    break;
  default:
    encoder->codecChoice = AVIF_CODEC_CHOICE_AUTO;
    break;
  }

  // Configure tuning settings
  switch (config.tune) {
  case Tune::TUNE_PSNR: {
    Result optionResult =
        SetAvifOption(encoder.get(), "tune", "psnr", "PSNR Tuning", __func__);
    if (auto *error = std::get_if<Error>(&optionResult)) {
      return *error;
    }
    break;
  }
  case Tune::TUNE_SSIM: {
    Result optionResult =
        SetAvifOption(encoder.get(), "tune", "ssim", "SSIM Tuning", __func__);
    if (auto *error = std::get_if<Error>(&optionResult)) {
      return *error;
    }
    break;
  }
  default:
    break;
  }

  // Configure general encoder settings
  encoder->maxThreads = emscripten_num_logical_cores();
  encoder->quality = config.quality;
  encoder->qualityAlpha =
      (config.qualityAlpha == -1) ? config.quality : config.qualityAlpha;
  encoder->speed = config.getSpeed();

  // Set sharpness option
  Result sharpnessResult = SetAvifOption(
      encoder.get(), "sharpness", std::to_string(config.sharpness).c_str(),
      "Sharpness", __func__);
  if (auto *error = std::get_if<Error>(&sharpnessResult)) {
    return *error;
  }

  // Calculate and set quantizer values
  int q = static_cast<int>(
      std::round((100 - config.quality) * MAX_QUANTIZER / 100.0));
  int minQ = (config.minQuantizer == -1) ? q : config.minQuantizer;
  int maxQ = (config.maxQuantizer == -1) ? q : config.maxQuantizer;

  if (minQ > maxQ) {
    return Error(ConverterError::INVALID_QUANTIZER_VALUES,
                 "minQuantizer must be <= maxQuantizer", __func__);
  }

  encoder->minQuantizer = minQ;
  encoder->maxQuantizer = maxQ;
  encoder->tileRowsLog2 = config.tileRowsLog2;
  encoder->tileColsLog2 = config.tileColsLog2;

  // Special settings for AOM codec in high-quality mode (speed < 5)
  if (config.getSpeed() < 5 && encoder->codecChoice == AVIF_CODEC_CHOICE_AOM) {
    encoder->maxThreads = 1;

    Result rowMtResult = SetAvifOption(encoder.get(), "row-mt", "0",
                                       "Row Multi-Threading", __func__);
    if (auto *error = std::get_if<Error>(&rowMtResult)) {
      return *error;
    }

    Result tileColumnsResult = SetAvifOption(encoder.get(), "tile-columns", "0",
                                             "Tile Columns", __func__);
    if (auto *error = std::get_if<Error>(&tileColumnsResult)) {
      return *error;
    }

    Result tileRowsResult =
        SetAvifOption(encoder.get(), "tile-rows", "0", "Tile Rows", __func__);
    if (auto *error = std::get_if<Error>(&tileRowsResult)) {
      return *error;
    }

    Result frameParallelResult = SetAvifOption(encoder.get(), "frame-parallel",
                                               "0", "Frame Parallel", __func__);
    if (auto *error = std::get_if<Error>(&frameParallelResult)) {
      return *error;
    }
  }

  // Create and configure the AVIF image
  UniqueAvifImage image(
      avifImageCreate(width, height, RGB_DEPTH, config.pixelFormat));
  image->colorPrimaries = AVIF_COLOR_PRIMARIES_BT709;
  image->transferCharacteristics = AVIF_TRANSFER_CHARACTERISTICS_SRGB;
  image->matrixCoefficients = AVIF_MATRIX_COEFFICIENTS_BT709;
  image->yuvRange = AVIF_RANGE_FULL;

  // Set up RGB image for conversion to YUV
  avifRGBImage rgb;
  avifRGBImageSetDefaults(&rgb, image.get());
  rgb.chromaUpsampling = AVIF_CHROMA_UPSAMPLING_AUTOMATIC;
  rgb.format = AVIF_RGB_FORMAT_RGBA;
  rgb.depth = RGB_DEPTH;
  rgb.pixels = processed_data.data();
  rgb.rowBytes = width * avifRGBImagePixelSize(&rgb);
  rgb.alphaPremultiplied = AVIF_FALSE;
  rgb.ignoreAlpha = false;

  // Convert RGB to YUV
  avifResult convertResult = avifImageRGBToYUV(image.get(), &rgb);
  if (convertResult != AVIF_RESULT_OK) {
    return Error(ConverterError::CONVERSION_FAILED,
                 "RGB->YUV conversion failed: " +
                     std::string(avifResultToString(convertResult)),
                 __func__);
  }

  // Encode the image to AVIF format
  avifRWData output = AVIF_DATA_EMPTY;
  avifResult result = avifEncoderWrite(encoder.get(), image.get(), &output);
  if (result != AVIF_RESULT_OK) {
    std::string errorMsg =
        "Encoding failed: " + std::string(avifResultToString(result));
    if (result == AVIF_RESULT_OUT_OF_MEMORY) {
      errorMsg += " - Try higher speed values (5-10)";
    }
    return Error(avifToConverterError(result), errorMsg, __func__);
  }

  // Copy encoded data to result vector
  std::vector<uint8_t> output_data(output.data, output.data + output.size);
  avifRWDataFree(&output);

  return std::make_shared<ImageBuffer>(std::move(output_data));
}

/**
 * @brief JavaScript binding wrapper for image conversion
 * 
 * This function:
 * 1. Converts JavaScript data to C++ vector
 * 2. Detects the image type
 * 3. Validates the image can be processed
 * 4. Calls convert_image to perform the actual conversion
 * 5. Returns JavaScript object with result
 * 
 * @param jsData JavaScript array/TypedArray containing image data
 * @param width Desired width of the output image
 * @param height Desired height of the output image
 * @param config Encoding configuration parameters
 * 
 * @return JavaScript object containing either the encoded image buffer or error information
 */
emscripten::val convertImageDirect(emscripten::val jsData, int width,
                                   int height, const EncodeConfig &config) {
  try {
    // Convert JavaScript data to C++ vector
    std::vector<uint8_t> inputData =
        emscripten::convertJSArrayToNumberVector<uint8_t>(jsData);

    // Detect image type and validate it's supported
    ImageType type = ImageGuru::GetImageType(inputData);

    if (type == ImageType::UNKNOWN) {
      Error err(ConverterError::UNSUPPORTED_IMAGETYPE, "unsupported image type",
                __func__);
      return toJsError(err);
    }

    if (!ImageGuru::isSupported(type)) {
      Error err(ConverterError::UNSUPPORTED_IMAGETYPE,
                ImageGuru::typeToString(type) + "-type still not supported",
                __func__);
      return toJsError(err);
    }
    Result r = convert_image(type, inputData, width, height, config);
    jsResult result{r};
    emscripten::val jresult = emscripten::val::object();

    if (result.hasError()) {
      Error err = result.getError();
      return toJsError(err);
    } else if (result.hasImage()) {
      auto image = result.getImage();
      const auto &imageData = image->getRawData();
      emscripten::val uint8Array =
          emscripten::val::global("Uint8Array").new_(imageData.size());
      uint8Array.call<void>("set",
                            emscripten::val(emscripten::typed_memory_view(
                                imageData.size(), imageData.data())));
      jresult.set("data", uint8Array);
      jresult.set("success", true);

    } else {
      emscripten::val errorObj = emscripten::val::object();
      errorObj.set("code", static_cast<int>(ConverterError::UNKNOWN_ERROR));
      errorObj.set("message", "Unexpected result type");
      jresult.set("error", errorObj);
      jresult.set("success", false);
    }
    return jresult;
  } catch (const std::exception &e) {
    emscripten::val errorObj = emscripten::val::object();
    errorObj.set("code", static_cast<int>(ConverterError::UNKNOWN_ERROR));
    errorObj.set("message", e.what());
    emscripten::val jresult = emscripten::val::object();
    jresult.set("error", errorObj);
    jresult.set("success", false);
    return jresult;
  }
}
