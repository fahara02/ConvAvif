/**
 * @file encoder_config.h
 * @brief Configuration parameters and settings for AVIF encoding
 * @author ConvAvif Developer
 * @date 2025
 * 
 * This file defines the configuration structures and enums that control
 * the AVIF encoding process, including codec selection, quality settings,
 * and various encoder-specific parameters.
 */
#ifndef ENCODE_CONFIG_H
#define ENCODE_CONFIG_H

#include "constants.h"
#include <algorithm>
#include <avif/avif.h>
#include <string>

/**
 * @brief Codecs available for AVIF encoding
 */
enum class CodecChoice { AUTO, AOM, SVT };

/**
 * @brief Tuning options for the encoder
 */
enum class Tune { TUNE_DEFAULT = 0, TUNE_PSNR, TUNE_SSIM };

/**
 * @brief Speed presets for encoding with different performance/quality tradeoffs
 */
enum class SpeedPreset : int { Good, MemoryHungry, RealTime };

/**
 * @brief Helper template function to clamp a field value between minimum and maximum values
 * 
 * @tparam T The type of the field (typically int)
 * @param field Reference to the field to be clamped
 * @param minVal Minimum allowed value
 * @param maxVal Maximum allowed value
 */
template <typename T> void setField(T &field, T minVal, T maxVal) {
  field = std::clamp(field, minVal, maxVal);
}

/**
 * @brief Structure to define the range of valid speed values
 */
struct SpeedRange {
  int min; ///< Minimum valid speed value
  int max; ///< Maximum valid speed value
};

/**
 * @brief Speed settings manager for AVIF encoding
 * 
 * This class manages speed settings based on the selected codec and preset.
 * Different codecs have different optimal speed ranges for various quality/speed tradeoffs.
 */
struct Speed {

  SpeedPreset preset_ = SpeedPreset::Good; ///< Current speed preset
  int default_speed = 5; ///< Default speed value
  SpeedRange speed_range = {5, 6}; ///< Valid speed range

  /**
   * @brief Constructor that sets appropriate speed ranges based on codec and preset
   * 
   * @param codec The selected codec (AOM, SVT, AUTO)
   * @param preset The desired speed preset (Good, MemoryHungry, RealTime)
   */
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

  /**
   * @brief Get the default speed value
   * @return Current default speed value
   */
  int getDefault() const { return default_speed; }
  
  /**
   * @brief Set a new speed range
   * @param range New speed range to use
   */
  void setSpeedRange(const SpeedRange &range) { speed_range = range; }
  
  /**
   * @brief Get the current speed range
   * @return Current speed range
   */
  SpeedRange getRange() const { return speed_range; }
  
  /**
   * @brief Check if a speed value is within the valid range
   * @param speed Speed value to check
   * @return true if speed is valid, false otherwise
   */
  bool isValid(int speed) {
    return speed >= speed_range.min && speed <= speed_range.max;
  }
  
  /**
   * @brief Get the current speed preset
   * @return Current SpeedPreset value
   */
  SpeedPreset getPreset() const { return preset_; }
  
  /**
   * @brief Set a new speed value if it's valid
   * @param ns New speed value to set
   */
  void set(int ns) {
    if (isValid(ns)) {
      default_speed = ns;
    } else {
      printf("Speed is not valid! going back to default!\n");
    }
  }
};

/**
 * @brief Main configuration structure for AVIF encoding
 * 
 * This structure contains all the parameters that control the AVIF encoding process,
 * including quality settings, codec choice, and various encoder-specific parameters.
 */
struct EncodeConfig {
private:
  /**
   * @brief Validate and clamp quality value to valid range
   * @param q Quality value to validate
   * @return Clamped quality value
   */
  int validatedQuality(int q) const { return std::clamp(q, 0, MAX_QUALITY); }
  
  /**
   * @brief Validate and clamp quantizer value to valid range
   * @param q Quantizer value to validate
   * @return Clamped quantizer value or -1 if default value is specified
   */
  int validatedQuantizer(int q) const {
    return q == -1 ? -1 : std::clamp(q, 0, MAX_QUANTIZER);
  }
  
  /**
   * @brief Validate and clamp tile log2 value to valid range
   * @param t Tile log2 value to validate
   * @return Clamped tile log2 value
   */
  int validatedTileLog2(int t) const { return std::clamp(t, 0, MAX_TILE_LOG2); }
  
  /**
   * @brief Validate and clamp sharpness value to valid range
   * @param s Sharpness value to validate
   * @return Clamped sharpness value
   */
  int validatedSharpness(int s) const {
    return std::clamp(s, 0, MAX_SHARPNESS);
  }

public:
  int quality = 30;       ///< Quality value (0-100, higher is better quality)
  int qualityAlpha = -1;  ///< Alpha channel quality (-1 means use quality value)

  int sharpness = 0;      ///< Sharpness filter strength (0-7)
  avifPixelFormat pixelFormat = AVIF_PIXEL_FORMAT_YUV420;  ///< YUV pixel format
  CodecChoice codecChoice;  ///< Selected codec (AOM, SVT, AUTO)
  SpeedPreset preset;       ///< Speed preset

  int minQuantizer = -1;  ///< Minimum quantizer (0-63, -1 for default)
  int maxQuantizer = -1;  ///< Maximum quantizer (0-63, -1 for default)
  int tileRowsLog2 = 0;   ///< Log2 of tile rows (0 = 1 tile row, 1 = 2 rows, etc)
  int tileColsLog2 = 0;   ///< Log2 of tile columns

  Tune tune = Tune::TUNE_DEFAULT;  ///< Encoder tuning option

  /**
   * @brief Set the quality value with validation
   * @param q New quality value
   */
  void setQuality(int q) { quality = validatedQuality(q); }
  
  /**
   * @brief Set the minimum quantizer value with validation
   * @param q New minimum quantizer value
   */
  void setMinQuantizer(int q) { minQuantizer = validatedQuantizer(q); }
  
  /**
   * @brief Set the maximum quantizer value with validation
   * @param q New maximum quantizer value
   */
  void setMaxQuantizer(int q) { maxQuantizer = validatedQuantizer(q); }
  
  /**
   * @brief Set the tile rows log2 value with validation
   * @param t New tile rows log2 value
   */
  void setTileRowsLog2(int t) { tileRowsLog2 = validatedTileLog2(t); }
  
  /**
   * @brief Set the tile columns log2 value with validation
   * @param t New tile columns log2 value
   */
  void setTileColsLog2(int t) { tileColsLog2 = validatedTileLog2(t); }
  
  /**
   * @brief Set the sharpness value with validation
   * @param s New sharpness value
   */
  void setSharpness(int s) { sharpness = validatedSharpness(s); }

  /**
   * @brief Get the current quality value
   * @return Current quality value
   */
  int getQuality() const { return quality; }
  
  /**
   * @brief Get the current minimum quantizer value
   * @return Current minimum quantizer value
   */
  int getMinQuantizer() const { return minQuantizer; }
  
  /**
   * @brief Get the current maximum quantizer value
   * @return Current maximum quantizer value
   */
  int getMaxQuantizer() const { return maxQuantizer; }
  
  /**
   * @brief Get the current tile rows log2 value
   * @return Current tile rows log2 value
   */
  int getTileRowsLog2() const { return tileRowsLog2; }
  
  /**
   * @brief Get the current tile columns log2 value
   * @return Current tile columns log2 value
   */
  int getTileColsLog2() const { return tileColsLog2; }
  
  /**
   * @brief Get the current sharpness value
   * @return Current sharpness value
   */
  int getSharpness() const { return sharpness; }
  
  /**
   * @brief Get the current speed value
   * @return Current speed value
   */
  int getSpeed() const { return speed.getDefault(); }

  /**
   * @brief Set a new speed preset and update speed settings
   * @param newPreset New speed preset
   */
  void setPreset(SpeedPreset newPreset) {
    preset = newPreset;
    speed = Speed(codecChoice, preset);
  }
  
  /**
   * @brief Get the current speed preset
   * @return Current speed preset
   */
  SpeedPreset getPreset() const { return preset; }

  /**
   * @brief Update speed settings with new values
   * 
   * @param new_speed New speed value
   * @param newPreset New speed preset
   */
  void updateSpeed(int new_speed, SpeedPreset newPreset) {

    setPreset(newPreset);
    printf("Updating speed: preset=%d, new_speed=%d, codec=%d\n",
           static_cast<int>(newPreset), new_speed,
           static_cast<int>(codecChoice));
    speed.set(new_speed);
    printf("updated new speed: %d\n", speed.getDefault());
  }
  
  /**
   * @brief Get the current codec choice
   * @return Current codec choice
   */
  CodecChoice getCodecChoice() const { return codecChoice; }
  
  /**
   * @brief Set a new codec choice and update speed settings
   * @param choice New codec choice
   */
  void setCodecChoice(CodecChoice choice) {
    codecChoice = choice;
    speed = Speed(codecChoice, preset);
  }
  
  /**
   * @brief Print the current configuration to stdout
   * 
   * Useful for debugging and logging purposes
   */
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
  Speed speed = Speed(CodecChoice::AOM, SpeedPreset::Good); ///< Speed settings
};

#endif // ENCODE_CONFIG_H