#ifndef IMAGECONVERTER_H
#define IMAGECONVERTER_H
#include "imagebuffer.h"
#include <algorithm>
#include <avif/avif.h>
#include <string>
#include <variant>

enum class ImageType { JPEG, PNG, GIF, BMP, TIFF, WEBP, AVIF, UNKNOWN };

static constexpr int RGB_DEPTH = 8;
static constexpr int MIN_SPEED = 0;
static constexpr int MAX_SPEED = 9;
static constexpr int MAX_QUALITY = 99;
static constexpr int MAX_QUANTIZER = 63;
static constexpr int MAX_TILE_LOG2 = 6;
static constexpr int MAX_SHARPNESS = 7; // Sharpness range (0-7)

template <typename T> void setField(T &field, T minVal, T maxVal) {
  field = std::clamp(field, minVal, maxVal);
}

enum class ConverterError {
  // Success
  OK = 0,

  // Application-specific errors
  INVALID_DIMENSIONS = 100,
  IMAGE_LOAD_FAILED,
  ENCODER_CREATION_FAILED,
  CONVERSION_FAILED,
  ENCODING_FAILED,
  INVALID_ARGUMENT,
  OUT_OF_MEMORY,
  INVALID_QUANTIZER_VALUES,
  UNKNOWN_ERROR,

  // AVIF errors start at 200 (offset to avoid overlap)
  AVIF_ERROR_START = 200
};

class Error {
public:
  ConverterError code;
  std::string message;
  std::string stackTrace;

  Error(ConverterError c, const std::string &msg, const std::string &trace = "")
      : code(c), message(msg), stackTrace(trace) {}
};

// Convert avifResult to ConverterError
ConverterError avifToConverterError(avifResult result) {
  return static_cast<ConverterError>(
      static_cast<int>(ConverterError::AVIF_ERROR_START) +
      static_cast<int>(result));
}

// Get error message as string
std::string getErrorMessage(ConverterError error) {
  if (static_cast<int>(error) >=
      static_cast<int>(ConverterError::AVIF_ERROR_START)) {
    avifResult avifError = static_cast<avifResult>(
        static_cast<int>(error) -
        static_cast<int>(ConverterError::AVIF_ERROR_START));
    return avifResultToString(avifError);
  }

  switch (error) {
  case ConverterError::OK:
    return "Success";
  case ConverterError::INVALID_DIMENSIONS:
    return "Invalid dimensions (1-8192px allowed)";
  case ConverterError::IMAGE_LOAD_FAILED:
    return "Failed to load image";
  case ConverterError::ENCODER_CREATION_FAILED:
    return "Failed to create AVIF encoder";
  case ConverterError::CONVERSION_FAILED:
    return "Failed to convert";
  case ConverterError::ENCODING_FAILED:
    return "Failed to encode";
  case ConverterError::OUT_OF_MEMORY:
    return "Out of memory";
  case ConverterError::INVALID_QUANTIZER_VALUES:
    return "Invalid quantizer values (min > max)";
  default:
    return "Unknown error";
  }
}
using ImgaePtr = std::shared_ptr<ImageBuffer>;

using Result = std::variant<ImgaePtr, avifResult, Error>;

struct jsResult {
  // Avoided avifResult extracting as its not relevant for avif ok and errors
  // are already translated
  Result result;

  bool hasError() const { return std::holds_alternative<Error>(result); }
  bool hasImage() const { return std::holds_alternative<ImgaePtr>(result); }

  Error getError() const { return std::get<Error>(result); }
  ImgaePtr getImage() const { return std::get<ImgaePtr>(result); }
};

enum class CodecChoice { AUTO, AOM, SVT };
enum class Tune { TUNE_DEFAULT = 0, TUNE_PSNR, TUNE_SSIM };
struct SpeedRange {
  int min;
  int max;
};
enum class SpeedPreset : int { Good, MemoryHungry, RealTime };

struct Speed {

  SpeedPreset preset_ = SpeedPreset::Good;
  int default_speed = 5;
  SpeedRange speed_range = {5, 6};

  Speed(CodecChoice codec, SpeedPreset preset) : preset_(preset) {
    switch (codec) {
    case CodecChoice::AOM:
      switch (preset) {
      case SpeedPreset::MemoryHungry:
        default_speed = 4;
        speed_range = {0, 4};
        break;
      case SpeedPreset::Good:
        default_speed = 6;
        speed_range = {5, 7};
        break;
      case SpeedPreset::RealTime:
        default_speed = 9;
        speed_range = {8, 10};
        break;
      default:
        default_speed = 6;
        speed_range = {5, 7};
        break;
      }
      break;
    case CodecChoice::SVT:
      // Example ranges for SVT (adjust as needed)
      switch (preset) {
      case SpeedPreset::MemoryHungry:
        default_speed = 2;
        speed_range = {0, 2};
        break;
      case SpeedPreset::Good:
        default_speed = 5;
        speed_range = {3, 5};
        break;
      case SpeedPreset::RealTime:
        default_speed = 7;
        speed_range = {6, 8};
        break;
      default:
        default_speed = 5;
        speed_range = {3, 5};
        break;
      }
      break;
    case CodecChoice::AUTO:
      default_speed = 5;
      speed_range = {3, 5};
      break;
    default:
      default_speed = 5;
      speed_range = {3, 5};
      break;
    }
  }

  int getDefault() const { return default_speed; }
  void setSpeedRange(const SpeedRange &range) { speed_range = range; }
  SpeedRange getRange() const { return speed_range; }
  bool isValid(int speed) {
    return speed >= speed_range.min && speed <= speed_range.max;
  }
  SpeedPreset getPreset() const { return preset_; }
  void set(int ns) {
    if (isValid(ns)) {
      default_speed = ns;
    } else {
      printf("Speed is not valid! going back to default!\n");
    }
  }
};

struct EncodeConfig {
private:
  // Validation wrappers for fields
  int validatedQuality(int q) const { return std::clamp(q, 0, MAX_QUALITY); }
  int validatedQuantizer(int q) const {
    return q == -1 ? -1 : std::clamp(q, 0, MAX_QUANTIZER);
  }
  int validatedTileLog2(int t) const { return std::clamp(t, 0, MAX_TILE_LOG2); }
  int validatedSharpness(int s) const {
    return std::clamp(s, 0, MAX_SHARPNESS);
  }

public:
  int quality = 30;
  int qualityAlpha = -1;

  int sharpness = 0; // 0-7 filter strength
  avifPixelFormat pixelFormat = AVIF_PIXEL_FORMAT_YUV420;
  CodecChoice codecChoice;
  SpeedPreset preset;

  int minQuantizer = -1; // 0-63, -1 for default
  int maxQuantizer = -1;
  int tileRowsLog2 = 0;
  int tileColsLog2 = 0;

  Tune tune = Tune::TUNE_DEFAULT;

  // Setters with validation
  void setQuality(int q) { quality = validatedQuality(q); }
  void setMinQuantizer(int q) { minQuantizer = validatedQuantizer(q); }
  void setMaxQuantizer(int q) { maxQuantizer = validatedQuantizer(q); }
  void setTileRowsLog2(int t) { tileRowsLog2 = validatedTileLog2(t); }
  void setTileColsLog2(int t) { tileColsLog2 = validatedTileLog2(t); }
  void setSharpness(int s) { sharpness = validatedSharpness(s); }

  // Getters
  int getQuality() const { return quality; }
  int getMinQuantizer() const { return minQuantizer; }
  int getMaxQuantizer() const { return maxQuantizer; }
  int getTileRowsLog2() const { return tileRowsLog2; }
  int getTileColsLog2() const { return tileColsLog2; }
  int getSharpness() const { return sharpness; }
  int getSpeed() const { return speed.getDefault(); }

  void setPreset(SpeedPreset newPreset) {
    preset = newPreset;
    speed = Speed(codecChoice, preset);
  }
  SpeedPreset getPreset() const { return preset; }

  void updateSpeed(int new_speed, SpeedPreset newPreset) {

    setPreset(newPreset);
    printf("Updating speed: preset=%d, new_speed=%d, codec=%d\n",
           static_cast<int>(newPreset), new_speed,
           static_cast<int>(codecChoice));
    speed.set(new_speed);
    printf("updated new speed: %d\n", speed.getDefault());
  }
  CodecChoice getCodecChoice() const { return codecChoice; }
  void setCodecChoice(CodecChoice choice) {
    codecChoice = choice;
    speed = Speed(codecChoice, preset);
  }
  void printConfig() const {
    printf("EncodeConfig:\n");
    printf("  quality: %d\n", quality);
    printf("  qualityAlpha: %d\n", qualityAlpha);
    printf("  sharpness: %d\n", sharpness);
    printf("  pixelFormat: %d\n", pixelFormat);
    printf("  codecChoice: %d\n", static_cast<int>(codecChoice));
    printf("  preset: %d\n", static_cast<int>(preset));
    printf("  minQuantizer: %d\n", minQuantizer);
    printf("  maxQuantizer: %d\n", maxQuantizer);
    printf("  tileRowsLog2: %d\n", tileRowsLog2);
    printf("  tileColsLog2: %d\n", tileColsLog2);
    printf("  tune: %d\n", static_cast<int>(tune));
    printf("  speed: %d\n", speed.getDefault());
    printf("  speedRange: [%d, %d]\n", speed.getRange().min,
           speed.getRange().max);
  }

private:
  Speed speed = Speed(CodecChoice::AOM, SpeedPreset::Good);
};

Result convert_image(const std::vector<uint8_t> &input_data, int width,
                     int height, const EncodeConfig &config);

struct AvifEncoderDeleter {
  void operator()(avifEncoder *enc) { avifEncoderDestroy(enc); }
};
using UniqueAvifEncoder = std::unique_ptr<avifEncoder, AvifEncoderDeleter>;

struct AvifImageDeleter {
  void operator()(avifImage *img) { avifImageDestroy(img); }
};
using UniqueAvifImage = std::unique_ptr<avifImage, AvifImageDeleter>;

// ========================
// Helper Functions
// ========================

[[nodiscard]] Result SetAvifOption(avifEncoder *encoder, const char *key,
                                   const char *value,
                                   const std::string &humanReadableOption,
                                   const char *callerFunction = __func__) {
  avifResult result = avifEncoderSetCodecSpecificOption(encoder, key, value);
  if (result != AVIF_RESULT_OK) {
    return Error{avifToConverterError(result),
                 "Failed to set [" + std::string(key) + "=" + value + "] (" +
                     humanReadableOption + "): " + avifResultToString(result),
                 callerFunction};
  }
  return avifResult::AVIF_RESULT_OK;
}

#endif