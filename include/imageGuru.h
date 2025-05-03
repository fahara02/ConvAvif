/**
 * @file imageGuru.h
 * @brief Image format detection and type management
 * @author ConvAvif Developer
 * @date 2025
 * 
 * This file provides utilities for detecting image formats from binary data
 * and checking format support status for the AVIF conversion process.
 */
#ifndef IMAGE_GURU_H
#define IMAGE_GURU_H
#include <algorithm>
#include <array>
#include <string>
#include <vector>

/**
 * @brief Enumeration of supported image types
 * 
 * Lists all the image formats that can be detected by the ImageGuru,
 * regardless of whether they can be converted to AVIF.
 */
enum class ImageType { JPEG, PNG, GIF, BMP, TIFF, WEBP, AVIF, UNKNOWN };

/**
 * @brief Utility class for image format detection and support checking
 * 
 * This class provides static methods to detect image formats from binary data
 * and check whether specific formats are supported for conversion to AVIF.
 */
class ImageGuru {
public:
  /**
   * @brief Detect the image format from binary data
   * 
   * Examines the file header/magic bytes in the provided data to
   * determine the image format.
   * 
   * @param data Binary image data to analyze
   * @return Detected ImageType, or ImageType::UNKNOWN if format isn't recognized
   */
  static ImageType GetImageType(const std::vector<uint8_t> &data) {
    if (data.empty()) {
      return ImageType::UNKNOWN;
    }

    std::array<uint8_t, 12> buffer{};
    size_t bytes_to_copy = std::min(data.size(), buffer.size());
    std::copy_n(data.begin(), bytes_to_copy, buffer.begin());

    // Check for JPEG (FF D8)
    if (data.size() >= 2 && buffer[0] == 0xFF && buffer[1] == 0xD8) {
      return ImageType::JPEG;
    }

    // Check for BMP (BM)
    if (data.size() >= 2 && buffer[0] == 'B' && buffer[1] == 'M') {
      return ImageType::BMP;
    }

    // Check for GIF (GIF87a or GIF89a)
    if (data.size() >= 6 && buffer[0] == 'G' && buffer[1] == 'I' &&
        buffer[2] == 'F' && buffer[3] == '8' &&
        (buffer[4] == '7' || buffer[4] == '9') && buffer[5] == 'a') {
      return ImageType::GIF;
    }

    // Check for TIFF (II or MM header)
    if (data.size() >= 4) {
      bool isTiffLE = (buffer[0] == 'I' && buffer[1] == 'I' &&
                       buffer[2] == 0x2A && buffer[3] == 0x00);
      bool isTiffBE = (buffer[0] == 'M' && buffer[1] == 'M' &&
                       buffer[2] == 0x00 && buffer[3] == 0x2A);
      if (isTiffLE || isTiffBE)
        return ImageType::TIFF;
    }

    // Check for PNG
    if (data.size() >= 8) {
      const std::array<uint8_t, 8> pngSig = {0x89, 'P',  'N',  'G',
                                             0x0D, 0x0A, 0x1A, 0x0A};
      if (std::equal(pngSig.begin(), pngSig.end(), buffer.begin())) {
        return ImageType::PNG;
      }
    }

    // Check for WEBP (RIFF....WEBP)
    if (data.size() >= 12 && buffer[0] == 'R' && buffer[1] == 'I' &&
        buffer[2] == 'F' && buffer[3] == 'F' && buffer[8] == 'W' &&
        buffer[9] == 'E' && buffer[10] == 'B' && buffer[11] == 'P') {
      return ImageType::WEBP;
    }

    // Check for AVIF (ftypavif)
    if (data.size() >= 12 && buffer[4] == 'f' && buffer[5] == 't' &&
        buffer[6] == 'y' && buffer[7] == 'p' && buffer[8] == 'a' &&
        buffer[9] == 'v' && buffer[10] == 'i' && buffer[11] == 'f') {
      return ImageType::AVIF;
    }

    return ImageType::UNKNOWN;
  }

  /**
   * @brief Check if binary data appears to be a valid image
   * 
   * @param data Binary data to check
   * @return true if the data appears to be a recognized image format, false otherwise
   */
  static bool IsValid(const std::vector<uint8_t> &data) {
    return GetImageType(data) != ImageType::UNKNOWN;
  }

  /**
   * @brief Check if binary data matches a specific image format
   * 
   * @param data Binary data to check
   * @param type Specific ImageType to check for
   * @return true if the data matches the specified format, false otherwise
   */
  static bool IsSpecificType(const std::vector<uint8_t> &data, ImageType type) {
    return GetImageType(data) == type;
  }

  /**
   * @brief Structure for tracking image format support status
   */
  struct imageSupport {
    ImageType type;      ///< Type of image format
    bool supported;      ///< Whether conversion is supported
  };
  
  /**
   * @brief Check if a specific image type is supported for conversion
   * 
   * @param type The ImageType to check support for
   * @return true if the format is supported for conversion, false otherwise
   */
  static bool isSupported(const ImageType type) {

    auto it = std::find_if(
        supported_image.begin(), supported_image.end(),
        [type](const imageSupport &entry) { return entry.type == type; });

    return (it != supported_image.end()) ? it->supported : false;
  }

  /**
   * @brief Convert an ImageType enum to a string representation
   * 
   * @param type The ImageType to convert
   * @return String representation of the image type
   */
  static std::string typeToString(const ImageType type) {
    switch (type) {
    case ImageType::JPEG:
      return "JPEG";
    case ImageType::PNG:
      return "PNG";
    case ImageType::GIF:
      return "GIF";
    case ImageType::BMP:
      return "BMP";
    case ImageType::TIFF:
      return "TIFF";
    case ImageType::WEBP:
      return "WEBP";
    case ImageType::AVIF:
      return "AVIF";
    case ImageType::UNKNOWN:
      return "UNKNOWN";
    default:
      return "UNKNOWN";
    }
  }

private:
  /**
   * @brief Array defining which image formats are supported for conversion
   * 
   * This array maps each ImageType to a boolean indicating whether
   * that format can be converted to AVIF by the library.
   */
  static constexpr std::array<imageSupport, 8> supported_image = {
      {{ImageType::JPEG, true}, // JPEG supported
       {ImageType::PNG, true},  // PNG supported
       {ImageType::GIF, false}, // Others unsupported
       {ImageType::BMP, false},
       {ImageType::TIFF, false},
       {ImageType::WEBP, false},
       {ImageType::AVIF, true},
       {ImageType::UNKNOWN, false}}};
};
#endif