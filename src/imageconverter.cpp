#include "imageconverter.h"
#include <avif/avif.h>
#include <cstdlib>
#include <stb_image.h>
#include <stb_image_resize2.h>
#include <vector>
#include <emscripten/emscripten.h>  // for EM_ASM
#include <emscripten/bind.h>  // for emscripten::val, typed_memory_view

std::shared_ptr<ImageBuffer> convert_image(const std::string &input_data,
                                           int width, int height,
                                           const EncodeConfig &config) {
  // Decode input image
  int w, h, channels;
  unsigned char *data = stbi_load_from_memory(
      reinterpret_cast<const stbi_uc *>(input_data.data()), input_data.size(),
      &w, &h, &channels, STBI_rgb_alpha);
  if (!data)
    throw std::runtime_error("Failed to load image");

  // Resize image
  std::vector<unsigned char> resized_data(width * height * 4);
  stbir_resize_uint8_linear(data, w, h, 0, resized_data.data(), width, height,
                            0, STBIR_RGBA);
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
 encoder->quality = config.quality;    // Set main quality parameter
 encoder->qualityAlpha = config.quality; // If using alpha
 encoder->minQuantizer = config.quality;
 encoder->maxQuantizer = config.quality;
  encoder->speed = config.speed;
  avifResult optionResult = avifEncoderSetCodecSpecificOption(
    encoder,
    "cq-level",
    std::to_string(config.quality).c_str()
);
if (optionResult != AVIF_RESULT_OK) {
    throw std::runtime_error("Failed to set quality: " +
                           std::string(avifResultToString(optionResult)));
}

  avifImage *image = avifImageCreate(width, height, 8, config.pixelFormat);
   // Set color profile to sRGB/BT.709
   image->colorPrimaries = AVIF_COLOR_PRIMARIES_BT709;
   image->transferCharacteristics = AVIF_TRANSFER_CHARACTERISTICS_SRGB;
   image->matrixCoefficients = AVIF_MATRIX_COEFFICIENTS_BT709;
   image->yuvRange = AVIF_RANGE_FULL; // Use full range for web content
   avifRGBImage rgb;
   avifRGBImageSetDefaults(&rgb, image);
   rgb.format = AVIF_RGB_FORMAT_RGBA;
   rgb.depth = 8;
   rgb.pixels = resized_data.data();
   rgb.rowBytes = width * 4;
   rgb.alphaPremultiplied = AVIF_FALSE;
   avifResult convertResult = avifImageRGBToYUV(image, &rgb);
   if (convertResult != AVIF_RESULT_OK) {
     avifEncoderDestroy(encoder);
     avifImageDestroy(image);
     throw std::runtime_error("RGB->YUV conversion failed: " + 
                             std::string(avifResultToString(convertResult)));
   }

  avifRWData output = AVIF_DATA_EMPTY;
  avifResult result = avifEncoderWrite(encoder, image, &output);
  avifEncoderDestroy(encoder);
  avifImageDestroy(image);
  
  if (result != AVIF_RESULT_OK) {
    avifRWDataFree(&output);
    throw std::runtime_error("AVIF encoding failed: " +
                             std::string(avifResultToString(result)));
  }

  // Copy output to managed buffer
  uint8_t *output_data = static_cast<uint8_t *>(malloc(output.size));
  memcpy(output_data, output.data, output.size);
  avifRWDataFree(&output);

  return std::make_shared<ImageBuffer>(output_data, output.size);
}


// Replace convertImageFromVector with this version:
std::shared_ptr<ImageBuffer> convertImageWrapper(emscripten::val jsData, int width, int height, const EncodeConfig& config) {
  // Extract Uint8Array from JS value
  const auto length = jsData["length"].as<size_t>();
  std::vector<unsigned char> inputData(length);
  emscripten::val memoryView = emscripten::val(emscripten::typed_memory_view(
    inputData.size(), 
    inputData.data()
  ));
  memoryView.call<void>("set", jsData);

  try {
    std::string str(inputData.begin(), inputData.end());
    return convert_image(str, width, height, config);
  } catch (const std::exception& e) {
    EM_ASM({ throw new Error(UTF8ToString($0)); }, e.what());
    return nullptr;
  }
}

EMSCRIPTEN_BINDINGS(ImageConverter) {
 
  emscripten::class_<EncodeConfig>("EncodeConfig")
      .constructor<>()
      .property("quality", &EncodeConfig::quality)
      .property("speed", &EncodeConfig::speed)
      .property("pixelFormat", &EncodeConfig::pixelFormat)
      .property("codecChoice", &EncodeConfig::codecChoice);
      emscripten::enum_<EncodeConfig::CodecChoice>("CodecChoice")
      .value("AUTO", EncodeConfig::CodecChoice::AUTO)
      .value("AOM", EncodeConfig::CodecChoice::AOM)
      .value("SVT", EncodeConfig::CodecChoice::SVT);

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