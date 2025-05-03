#ifndef ENCODE_CONFIG_H
#define ENCODE_CONFIG_H

#include "constants.h"
#include <algorithm>
#include <avif/avif.h>
#include <string>

enum class ImageType { JPEG, PNG, GIF, BMP, TIFF, WEBP, AVIF, UNKNOWN };
enum class CodecChoice { AUTO, AOM, SVT };
enum class Tune { TUNE_DEFAULT = 0, TUNE_PSNR, TUNE_SSIM };
enum class SpeedPreset : int { Good, MemoryHungry, RealTime };

template <typename T> void setField(T &field, T minVal, T maxVal) {
  field = std::clamp(field, minVal, maxVal);
}

struct SpeedRange {
  int min;
  int max;
};

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

#endif // ENCODE_CONFIG_H