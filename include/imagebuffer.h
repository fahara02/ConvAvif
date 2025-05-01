// imagebuffer.h
#include <emscripten.h>
#include <emscripten/bind.h>
#include <memory>

class ImageBuffer {
public:
  ImageBuffer(uint8_t *data, size_t size) : data_(data), size_(size) {}
  ~ImageBuffer() { free(data_); }

  emscripten::val getData() const {
    return emscripten::val(emscripten::typed_memory_view(size_, data_));
  }

  size_t getSize() const { return size_; }

private:
  uint8_t *data_;
  size_t size_;
};

EMSCRIPTEN_BINDINGS(ImageBuffer) {
  emscripten::class_<ImageBuffer>("ImageBuffer")
      .smart_ptr<std::shared_ptr<ImageBuffer>>("shared_ptr<ImageBuffer>")
      .function("getData", &ImageBuffer::getData)
      .function("getSize", &ImageBuffer::getSize);
}