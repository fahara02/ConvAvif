/**
 * @file imageconverter.h
 * @brief Header for image conversion functionality to AVIF format.
 * @author ConvAvif Developer
 * @date 2025
 * 
 * Provides functions to convert various image formats to AVIF using
 * the AVIF library with configurable encoding parameters.
 */
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

/**
 * @brief Converts an image to AVIF format with specified parameters
 * 
 * @param type The type of input image (e.g., PNG, JPEG)
 * @param input_data Vector containing the raw image data
 * @param width Desired width of the output image (will resize if different from source)
 * @param height Desired height of the output image (will resize if different from source)
 * @param config Encoding configuration parameters
 * 
 * @return Result object containing either the encoded image data on success or an Error on failure
 * 
 * @note Image dimensions are limited to 1-8192px
 */
Result convert_image(ImageType type, const std::vector<uint8_t> &input_data,
                     int width, int height, const EncodeConfig &config);

/**
 * @brief JavaScript binding wrapper for image conversion
 * 
 * Takes JavaScript Uint8Array input and returns JavaScript object with result data.
 * 
 * @param jsData JavaScript array/TypedArray containing image data
 * @param width Desired width of the output image
 * @param height Desired height of the output image
 * @param config Encoding configuration parameters
 * 
 * @return JavaScript object containing either the encoded image buffer or error information
 */
emscripten::val convertImageDirect(emscripten::val jsData, int width,
                                   int height, const EncodeConfig &config);

#endif