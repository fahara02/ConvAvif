#ifndef ERROR_H
#define ERROR_H

#include <avif/avif.h>
#include <string>

enum class ConverterError {
  OK = 0,
  INVALID_DIMENSIONS = 100,
  IMAGE_LOAD_FAILED,
  ENCODER_CREATION_FAILED,
  CONVERSION_FAILED,
  ENCODING_FAILED,
  INVALID_ARGUMENT,
  OUT_OF_MEMORY,
  INVALID_QUANTIZER_VALUES,
  UNKNOWN_ERROR,
  AVIF_ERROR_START = 200
};

class Error {
public:
  ConverterError code;
  std::string message;
  std::string stackTrace;

  Error(ConverterError c, const std::string &msg,
        const std::string &trace = "");
};

ConverterError avifToConverterError(avifResult result);
std::string getErrorMessage(ConverterError error);

#endif // ERROR_H