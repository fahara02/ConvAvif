/**
 * @file imagebuffer.h
 * @brief Image buffer management for AVIF conversion results
 * @author ConvAvif Developer
 * @date 2025
 * 
 * This file defines the ImageBuffer class which encapsulates the
 * raw binary data of a converted image and provides methods to
 * access it from both C++ and JavaScript contexts.
 */
#ifndef IMAGE_BUFFER_H
#define IMAGE_BUFFER_H
#include <emscripten.h>
#include <emscripten/bind.h>
#include <memory>
#include <vector> // Add this

/**
 * @brief Container class for image binary data
 * 
 * This class encapsulates a vector of bytes representing image data
 * and provides methods to access this data from both C++ and JavaScript.
 */
class ImageBuffer {
public:
  /**
   * @brief Constructor that takes ownership of image data
   * 
   * @param data Vector containing the binary image data (typically AVIF encoded)
   */
  ImageBuffer(std::vector<uint8_t> data) : data_(std::move(data)) {}

  /**
   * @brief Get the image data as a JavaScript typed array
   * 
   * @return JavaScript typed array (Uint8Array) view of the image data
   * 
   * @note This creates a view into the existing memory without copying
   */
  emscripten::val getData() const {
    // Return a view into the vector's data
    return emscripten::val(
        emscripten::typed_memory_view(data_.size(), data_.data()));
  }

  /**
   * @brief Get the size of the image data in bytes
   * 
   * @return Size of the image data in bytes
   */
  size_t getSize() const { return data_.size(); }

  /**
   * @brief Get direct access to the underlying data vector
   * 
   * @return Const reference to the underlying byte vector
   */
  const std::vector<uint8_t> &getRawData() const { return data_; }

private:
  std::vector<uint8_t> data_;  ///< The binary image data
};

#endif