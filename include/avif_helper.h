#ifndef AVIF_HELPERS_H
#define AVIF_HELPERS_H

#include "result.h"
#include <avif/avif.h>
#include <memory>

struct AvifEncoderDeleter {
  void operator()(avifEncoder *enc) { avifEncoderDestroy(enc); }
};
using UniqueAvifEncoder = std::unique_ptr<avifEncoder, AvifEncoderDeleter>;

struct AvifImageDeleter {
  void operator()(avifImage *img) { avifImageDestroy(img); }
};
using UniqueAvifImage = std::unique_ptr<avifImage, AvifImageDeleter>;

// ========================
// Helper Functions
// ========================

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