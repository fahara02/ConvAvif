/**
 * @file avif_helper.h
 * @brief Helper utilities for AVIF library integration
 * @author ConvAvif Developer
 * @date 2025
 * 
 * This file provides utility classes and functions to simplify working with
 * the AVIF library, including RAII wrappers for AVIF objects and helper
 * functions for common operations.
 */
#ifndef AVIF_HELPERS_H
#define AVIF_HELPERS_H

#include "result.h"
#include <avif/avif.h>
#include <memory>

/**
 * @brief Custom deleter for avifEncoder objects
 * 
 * This functor is used with std::unique_ptr to properly clean up
 * avifEncoder resources when the unique_ptr goes out of scope.
 */
struct AvifEncoderDeleter {
  /**
   * @brief Deletes an avifEncoder object
   * @param enc Pointer to the avifEncoder to destroy
   */
  void operator()(avifEncoder *enc) { avifEncoderDestroy(enc); }
};

/**
 * @brief Unique pointer type for AVIF encoders with automatic cleanup
 * 
 * This type ensures that avifEncoder objects are properly destroyed
 * when they are no longer needed.
 */
using UniqueAvifEncoder = std::unique_ptr<avifEncoder, AvifEncoderDeleter>;

/**
 * @brief Custom deleter for avifImage objects
 * 
 * This functor is used with std::unique_ptr to properly clean up
 * avifImage resources when the unique_ptr goes out of scope.
 */
struct AvifImageDeleter {
  /**
   * @brief Deletes an avifImage object
   * @param img Pointer to the avifImage to destroy
   */
  void operator()(avifImage *img) { avifImageDestroy(img); }
};

/**
 * @brief Unique pointer type for AVIF images with automatic cleanup
 * 
 * This type ensures that avifImage objects are properly destroyed
 * when they are no longer needed.
 */
using UniqueAvifImage = std::unique_ptr<avifImage, AvifImageDeleter>;

// ========================
// Helper Functions
// ========================

/**
 * @brief Set a codec-specific option on an AVIF encoder
 * 
 * This function wraps the avifEncoderSetCodecSpecificOption function with
 * error handling and detailed error messages.
 * 
 * @param encoder The AVIF encoder to set the option on
 * @param key The option key (specific to the codec)
 * @param value The option value to set
 * @param humanReadableOption Human-readable description of the option (for error messages)
 * @param callerFunction Name of the calling function (for error context)
 * 
 * @return Result containing either OK status or an Error with details
 */
[[nodiscard]] inline Result
SetAvifOption(avifEncoder *encoder, const char *key, const char *value,
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

#endif // AVIF_HELPERS_H