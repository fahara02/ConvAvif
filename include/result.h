/**
 * @file result.h
 * @brief Result handling utilities for the AVIF conversion process
 * @author ConvAvif Developer
 * @date 2025
 * 
 * This file defines the Result type and related structures used to handle
 * the outcome of image conversion operations, which can be either a
 * successfully converted image or an error.
 */
#ifndef RESULT_H
#define RESULT_H

#include "error.h"
#include "imagebuffer.h"
#include <memory>
#include <variant>

/**
 * @brief Shared pointer to an ImageBuffer, representing a successfully converted image
 * 
 * This type alias is used as the success case in the Result variant.
 */
using ImgaePtr = std::shared_ptr<ImageBuffer>;

/**
 * @brief Result type that can hold either an image buffer or an error
 * 
 * This variant type is used as the return value for conversion operations,
 * allowing functions to return either a successful result (image data) or
 * an error condition.
 */
using Result = std::variant<ImgaePtr, avifResult, Error>;

/**
 * @brief JavaScript-friendly result structure
 * 
 * This structure wraps the C++ Result variant and provides helper methods
 * for JavaScript bindings to easily check result status and extract data.
 */
struct jsResult {
  Result result;  ///< The wrapped Result variant

  /**
   * @brief Check if the result contains an error
   * @return true if the result is an error, false otherwise
   */
  bool hasError() const { return std::holds_alternative<Error>(result); }
  
  /**
   * @brief Check if the result contains an image
   * @return true if the result is an image, false otherwise
   */
  bool hasImage() const { return std::holds_alternative<ImgaePtr>(result); }
  
  /**
   * @brief Get the error from the result
   * @return The Error object
   * @pre The result must contain an error (check with hasError() first)
   */
  Error getError() const { return std::get<Error>(result); }
  
  /**
   * @brief Get the image from the result
   * @return The image buffer pointer
   * @pre The result must contain an image (check with hasImage() first)
   */
  ImgaePtr getImage() const { return std::get<ImgaePtr>(result); }
};

#endif // RESULT_H