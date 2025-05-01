#ifndef IMAGECONVERTER_H
#define IMAGECONVERTER_H
#include "imagebuffer.h"
#include <avif/avif.h>
#include <string>



struct EncodeConfig {
  int quality = 30;
  int speed = 6;
  avifPixelFormat pixelFormat = AVIF_PIXEL_FORMAT_YUV420;
  enum CodecChoice {
    AUTO,
    AOM,
    SVT
  } codecChoice = AOM;
};

std::shared_ptr<ImageBuffer> convert_image(const std::string &input_data,
                                           int width, int height,
                                           const EncodeConfig &config);


#endif