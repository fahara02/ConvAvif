#ifndef RESULT_H
#define RESULT_H

#include "error.h"
#include "imagebuffer.h"
#include <memory>
#include <variant>

using ImgaePtr = std::shared_ptr<ImageBuffer>;
using Result = std::variant<ImgaePtr, avifResult, Error>;

struct jsResult {
  Result result;
  bool hasError() const { return std::holds_alternative<Error>(result); }
  bool hasImage() const { return std::holds_alternative<ImgaePtr>(result); }
  Error getError() const { return std::get<Error>(result); }
  ImgaePtr getImage() const { return std::get<ImgaePtr>(result); }
};

#endif // RESULT_H