#include "encoder_config.h"
#include "error.h"
#include "imagebuffer.h"
#include "imageconverter.h"
#include "result.h"
#include <emscripten/bind.h>

EMSCRIPTEN_BINDINGS(ImageBuffer) {
  emscripten::class_<ImageBuffer>("ImageBuffer")
      .smart_ptr<std::shared_ptr<ImageBuffer>>("shared_ptr<ImageBuffer>")
      .function("getData", &ImageBuffer::getData)
      .function("getSize", &ImageBuffer::getSize)
      .function("getRawData", &ImageBuffer::getRawData);
}

EMSCRIPTEN_BINDINGS(ImageConverter) {
  emscripten::enum_<SpeedPreset>("SpeedPreset")
      .value("Good", SpeedPreset::Good)
      .value("MemoryHungry", SpeedPreset::MemoryHungry)
      .value("RealTime", SpeedPreset::RealTime);

  emscripten::enum_<CodecChoice>("CodecChoice")
      .value("AUTO", CodecChoice::AUTO)
      .value("AOM", CodecChoice::AOM)
      .value("SVT", CodecChoice::SVT);

  emscripten::enum_<Tune>("Tune")
      .value("DEFAULT", Tune::TUNE_DEFAULT)
      .value("PSNR", Tune::TUNE_PSNR)
      .value("SSIM", Tune::TUNE_SSIM);

  emscripten::class_<SpeedRange>("SpeedRange")
      .constructor<>()
      .property("min", &SpeedRange::min)
      .property("max", &SpeedRange::max);

  emscripten::class_<Speed>("Speed")
      .constructor<CodecChoice, SpeedPreset>()
      .property("default_speed", &Speed::getDefault, &Speed::set,
                &Speed::default_speed)
      .property("speed_range", &Speed::getRange, &Speed::setSpeedRange)
      .property("preset", &Speed::preset_)
      .function("getDefault", &Speed::getDefault)
      .function("getRange", &Speed::getRange)
      .function("isValid", &Speed::isValid)
      .function("getPreset", &Speed::getPreset)
      .function("set", &Speed::set);

  emscripten::class_<EncodeConfig>("EncodeConfig")
      .constructor<>()
      .property("quality", &EncodeConfig::getQuality, &EncodeConfig::setQuality)
      .property("preset", &EncodeConfig::getPreset, &EncodeConfig::setPreset)
      .property("pixelFormat", &EncodeConfig::pixelFormat)
      .property("codecChoice", &EncodeConfig::getCodecChoice,
                &EncodeConfig::setCodecChoice)
      .property("minQuantizer", &EncodeConfig::getMinQuantizer,
                &EncodeConfig::setMinQuantizer)
      .property("maxQuantizer", &EncodeConfig::getMaxQuantizer,
                &EncodeConfig::setMaxQuantizer)
      .property("tileRowsLog2", &EncodeConfig::getTileRowsLog2,
                &EncodeConfig::setTileRowsLog2)
      .property("tileColsLog2", &EncodeConfig::getTileColsLog2,
                &EncodeConfig::setTileColsLog2)
      .property("tune", &EncodeConfig::tune)
      .property("sharpness", &EncodeConfig::sharpness)
      .function("getSpeed", &EncodeConfig::getSpeed)
      .function("updateSpeed", &EncodeConfig::updateSpeed);

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
      .value("OUT_OF_MEMORY", ConverterError::OUT_OF_MEMORY)
      .value("INVALID_QUANTIZER_VALUES",
             ConverterError::INVALID_QUANTIZER_VALUES)
      .value("UNKNOWN_ERROR", ConverterError::UNKNOWN_ERROR)
      .value("AVIF_ERROR_START", ConverterError::AVIF_ERROR_START);

  emscripten::class_<Error>("Error")
      .property("code", &Error::code)
      .property("message", &Error::message)
      .property("stackTrace", &Error::stackTrace);

  emscripten::class_<jsResult>("Result")
      .function("hasError", &jsResult::hasError)
      .function("hasImage", &jsResult::hasImage)
      .function("getError", &jsResult::getError)
      .function("getImage", &jsResult::getImage);
}