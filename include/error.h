/**
 * @file error.h
 * @brief Error handling for AVIF conversion operations
 * @author ConvAvif Developer
 * @date 2025
 * 
 * This file defines error types, error codes, and helper functions for
 * handling and reporting errors during AVIF conversion operations.
 */
#ifndef ERROR_H
#define ERROR_H

#include <avif/avif.h>
#include <emscripten/val.h>
#include <string>

/**
 * @brief Error codes for the image converter
 * 
 * These error codes identify different failure modes in the conversion process.
 * Error codes 100-199 are specific to the converter, while codes 200+ are 
 * mapped from the AVIF library.
 */
enum class ConverterError {
  OK = 0,                      ///< No error, operation successful
  INVALID_DIMENSIONS = 100,    ///< Image dimensions are invalid (too small/large)
  UNSUPPORTED_IMAGETYPE,       ///< Image format is not supported
  IMAGE_LOAD_FAILED,           ///< Failed to load/decode the input image
  ENCODER_CREATION_FAILED,     ///< Failed to create the AVIF encoder
  CONVERSION_FAILED,           ///< Failed during the RGB to YUV conversion
  ENCODING_FAILED,             ///< Failed during the encoding process
  INVALID_ARGUMENT,            ///< Invalid argument provided to a function
  OUT_OF_MEMORY,               ///< Memory allocation failed
  INVALID_QUANTIZER_VALUES,    ///< Invalid quantizer values (e.g., min > max)
  UNKNOWN_ERROR,               ///< Unspecified error
  AVIF_ERROR_START = 200       ///< Start of error codes mapped from avifResult
};

/**
 * @brief Error class for reporting conversion errors
 * 
 * This class encapsulates an error code, a descriptive message, and optional
 * stack trace information for debugging.
 */
class Error {
public:
  ConverterError code;       ///< Error code identifying the error type
  std::string message;       ///< Human-readable error message
  std::string stackTrace;    ///< Function call stack or context information

  /**
   * @brief Constructor for Error objects
   * 
   * @param c Error code identifying the error type
   * @param msg Human-readable error message
   * @param trace Optional function name or call stack information
   */
  Error(ConverterError c, const std::string &msg,
        const std::string &trace = "");
};

/**
 * @brief Converts an AVIF library error code to a ConverterError
 * 
 * Maps the native AVIF library error codes to the application's error enum.
 * 
 * @param result AVIF library result code
 * @return Equivalent ConverterError code
 */
ConverterError avifToConverterError(avifResult result);

/**
 * @brief Get a human-readable error message for an error code
 * 
 * @param error Error code to get the message for
 * @return Standard error message for the given code
 */
std::string getErrorMessage(ConverterError error);

/**
 * @brief Convert a C++ Error object to a JavaScript error object
 * 
 * Creates a JavaScript error object with properties for code, message, and stackTrace
 * suitable for returning to JavaScript code.
 * 
 * @param e The C++ Error object to convert
 * @return JavaScript object representing the error
 */
emscripten::val toJsError(const Error &e);

#endif // ERROR_H