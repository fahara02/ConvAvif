#ifndef IMAGE_GURU_H
#define IMAGE_GURU_H
#include <algorithm>
#include <array>
#include <vector>

enum class ImageType { JPEG, PNG, GIF, BMP, TIFF, WEBP, AVIF, UNKNOWN };

class ImageGuru {
public:
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

  static bool IsValid(const std::vector<uint8_t> &data) {
    return GetImageType(data) != ImageType::UNKNOWN;
  }

  static bool IsSpecificType(const std::vector<uint8_t> &data, ImageType type) {
    return GetImageType(data) == type;
  }
};
#endif