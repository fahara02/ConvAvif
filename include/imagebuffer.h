#include <emscripten.h>
#include <emscripten/bind.h>
#include <memory>
#include <vector> // Add this

class ImageBuffer {
public:

  ImageBuffer(std::vector<uint8_t> data) : data_(std::move(data)) {}

  emscripten::val getData() const {
    // Return a view into the vector's data
    return emscripten::val(emscripten::typed_memory_view(data_.size(), data_.data()));
  }

  size_t getSize() const { return data_.size(); }

private:
  std::vector<uint8_t> data_; 
};

EMSCRIPTEN_BINDINGS(ImageBuffer) {
  emscripten::class_<ImageBuffer>("ImageBuffer")
      .smart_ptr<std::shared_ptr<ImageBuffer>>("shared_ptr<ImageBuffer>")
      .function("getData", &ImageBuffer::getData)
      .function("getSize", &ImageBuffer::getSize);
}