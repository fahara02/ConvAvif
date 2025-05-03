#ifndef IMAGE_BUFFER_H
#define IMAGE_BUFFER_H
#include <emscripten.h>
#include <emscripten/bind.h>
#include <memory>
#include <vector> // Add this

class ImageBuffer {
public:
  ImageBuffer(std::vector<uint8_t> data) : data_(std::move(data)) {}

  emscripten::val getData() const {
    // Return a view into the vector's data
    return emscripten::val(
        emscripten::typed_memory_view(data_.size(), data_.data()));
  }

  size_t getSize() const { return data_.size(); }

  // Direct access to the underlying data vector
  const std::vector<uint8_t> &getRawData() const { return data_; }

private:
  std::vector<uint8_t> data_;
};

#endif