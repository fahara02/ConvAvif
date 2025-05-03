#ifndef IMAGECONVERTER_H
#define IMAGECONVERTER_H
#include "avif_helper.h"
#include "constants.h"
#include "encoder_config.h"
#include "error.h"
#include "imageGuru.h"
#include "imagebuffer.h"
#include "result.h"
#include <algorithm>
#include <avif/avif.h>
#include <string>

Result convert_image(const std::vector<uint8_t> &input_data, int width,
                     int height, const EncodeConfig &config);
emscripten::val convertImageDirect(emscripten::val jsData, int width,
                                   int height, const EncodeConfig &config);

#endif