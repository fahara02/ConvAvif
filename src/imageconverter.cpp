#include "imageconverter.h"
#include <avif/avif.h>
#include <cstdlib>
#include <cmath>
#include <emscripten/bind.h>       // for emscripten::val, typed_memory_view
#include <emscripten/emscripten.h> // for EM_ASM
#include <stb_image.h>
#include <stb_image_resize2.h>
#include <vector>

std::shared_ptr<ImageBuffer> convert_image(const std::string &input_data,
                                           int width, int height,
                                           const EncodeConfig &config) {

  // Before resizing
  if (width <= 0 || height <= 0 || width > 8192 || height > 8192) {
    throw std::runtime_error("Invalid dimensions (1-8192px allowed)");
  }
  // Decode input image
  int w, h, channels;
  unsigned char *data = stbi_load_from_memory(
      reinterpret_cast<const stbi_uc *>(input_data.data()), input_data.size(),
      &w, &h, &channels, STBI_rgb_alpha);
  if (!data)
    throw std::runtime_error("Failed to load image");

  // Resize image to RGBA
  std::vector<unsigned char> resized_data(width * height * 4);
  printf("Resized buffer: %dx%d@4ch (expected %zu bytes)\n", width, height,
         resized_data.size());
  stbir_resize_uint8_linear(data, w, h, 0,
                            resized_data.data(), width, height, 0,
                            STBIR_RGBA);
  stbi_image_free(data);

  // Configure AVIF encoder
  avifEncoder *encoder = avifEncoderCreate();
  // Inside convert_image() after creating encoder
  if (!encoder) {
    // Handle encoder creation failure
    throw std::runtime_error("Failed to create AVIF encoder");
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
  encoder->quality = config.quality;
  encoder->qualityAlpha =
      (config.qualityAlpha == -1) ? config.quality : config.qualityAlpha;
  encoder->speed = config.speed;
  // Apply sharpness (0-7) via codec-specific option
  avifEncoderSetCodecSpecificOption(encoder, "sharpness", std::to_string(config.sharpness).c_str());
  // Map percent quality (0-100) to libavif quantizer (0=best..63=worst)
  int q = static_cast<int>(std::round((100 - config.quality) * MAX_QUANTIZER/ 100.0));
  encoder->minQuantizer = q;
  encoder->maxQuantizer = q;
  encoder->minQuantizerAlpha = q;
  encoder->maxQuantizerAlpha = q;
  encoder->tileRowsLog2 = config.tileRowsLog2;
  encoder->tileColsLog2 = config.tileColsLog2;

  // avifResult optionResult = avifEncoderSetCodecSpecificOption(
  //     encoder, "cq-level", std::to_string(config.quality).c_str());
  // if (optionResult != AVIF_RESULT_OK) {
  //   throw std::runtime_error("Failed to set quality: " +
  //                            std::string(avifResultToString(optionResult)));
  // }


  avifImage *image = avifImageCreate(width, height, RGB_DEPTH, config.pixelFormat);
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
  rgb.alphaPremultiplied =  AVIF_FALSE;
  rgb.ignoreAlpha =  false;
  avifResult convertResult = avifImageRGBToYUV(image, &rgb);
  if (convertResult != AVIF_RESULT_OK) {
    avifEncoderDestroy(encoder);
    avifImageDestroy(image);
    throw std::runtime_error("RGB->YUV conversion failed: " +
                             std::string(avifResultToString(convertResult)));
  }

  avifRWData output = AVIF_DATA_EMPTY;
  avifResult result = avifEncoderWrite(encoder, image, &output);

  if (result != AVIF_RESULT_OK) {
    avifEncoderDestroy(encoder);
    avifImageDestroy(image);
    avifRWDataFree(&output);
    throw std::runtime_error("AVIF encoding failed: " +
                             std::string(avifResultToString(result)));
  }
  printf("From cpp AVIF output size: %zu bytes\n",
         output.size); // Log output size
  avifEncoderDestroy(encoder);
  avifImageDestroy(image);

  if (result != AVIF_RESULT_OK) {
    avifRWDataFree(&output);
    throw std::runtime_error("AVIF encoding failed: " +
                             std::string(avifResultToString(result)));
  }

  // Copy AVIF data into a vector (ensures ownership)
  std::vector<uint8_t> output_data(output.data, output.data + output.size);
  avifRWDataFree(&output); // Free libavif's buffer

  return std::make_shared<ImageBuffer>(std::move(output_data));
}

// Replace convertImageFromVector with this version:
std::shared_ptr<ImageBuffer> convertImageWrapper(emscripten::val jsData,
                                                 int width, int height,
                                                 const EncodeConfig &config) {
  // Extract Uint8Array from JS value
  const auto length = jsData["length"].as<size_t>();
  std::vector<unsigned char> inputData(length);
  emscripten::val memoryView = emscripten::val(
      emscripten::typed_memory_view(inputData.size(), inputData.data()));
  memoryView.call<void>("set", jsData);

  try {
    std::string str(inputData.begin(), inputData.end());
    return convert_image(str, width, height, config);
  } catch (const std::exception &e) {
    EM_ASM({ throw new Error(UTF8ToString($0)); }, e.what());
    return nullptr;
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

  // Bind convertImage using optional_override to accept JS TypedArray directly
  emscripten::function("convertImage", &convertImageWrapper);
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