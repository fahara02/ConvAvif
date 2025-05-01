#ifndef IMAGECONVERTER_H
#define IMAGECONVERTER_H
#include "imagebuffer.h"
#include <algorithm>
#include <avif/avif.h>
#include <string>

static constexpr int RGB_DEPTH = 8;
static constexpr int MAX_QUALITY = 99;
static constexpr int MAX_QUANTIZER = 63;
static constexpr int MAX_TILE_LOG2 = 6;
static constexpr int MAX_SHARPNESS = 7; // Sharpness range (0-7)

template <typename T> void setField(T &field, T minVal, T maxVal) {
  field = std::clamp(field, minVal, maxVal);
}

struct EncodeConfig {
private:
  // Validation wrappers for fields
  int validatedQuality(int q) const { return std::clamp(q, 0, MAX_QUALITY); }
  int validatedQuantizer(int q) const {
    return q == -1 ? -1 : std::clamp(q, 0, MAX_QUANTIZER);
  }
  int validatedTileLog2(int t) const { return std::clamp(t, 0, MAX_TILE_LOG2); }
  int validatedSharpness(int s) const { return std::clamp(s, 0, MAX_SHARPNESS); }

public:
  int quality = 30;
  int qualityAlpha = -1;
  int speed = 6;
  int sharpness = 0; // 0-7 filter strength
  avifPixelFormat pixelFormat = AVIF_PIXEL_FORMAT_YUV420;
  enum CodecChoice { AUTO, AOM, SVT } codecChoice = AOM;
  int minQuantizer = -1; // 0-63, -1 for default
  int maxQuantizer = -1;
  int tileRowsLog2 = 0;
  int tileColsLog2 = 0;
  enum Tune { TUNE_DEFAULT = 0, TUNE_PSNR, TUNE_SSIM };
  Tune tune = TUNE_DEFAULT;

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
};

std::shared_ptr<ImageBuffer> convert_image(const std::string &input_data,
                                           int width, int height,
                                           const EncodeConfig &config);

#endif