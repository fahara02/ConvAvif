#include "error.h"

Error::Error(ConverterError c, const std::string &msg, const std::string &trace)
    : code(c), message(msg), stackTrace(trace) {}

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

emscripten::val toJsError(const Error &e) {
  emscripten::val jresult = emscripten::val::object();
  emscripten::val errorObj = emscripten::val::object();
  errorObj.set("code", static_cast<int>(e.code));
  errorObj.set("message", e.message);
  errorObj.set("stackTrace", e.stackTrace);
  jresult.set("error", errorObj);
  jresult.set("success", false);
  return jresult;
}