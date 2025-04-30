#include "imageconverter.h"
#include <avif/avif.h>
#include <cstdlib>
#include <stb_image.h>
#include <stb_image_resize2.h>
#include <vector>


extern "C" {

Buffer convert_image(const char *input, int input_size, int width, int height) {
  Buffer result = {nullptr, 0};

  // Decode input image
  int w, h, channels;
  unsigned char *data =
      stbi_load_from_memory(reinterpret_cast<const stbi_uc *>(input),
                            input_size, &w, &h, &channels, STBI_rgb_alpha);

  if (!data)
    return result;

  // Resize image
  std::vector<unsigned char> resized_data(width * height * 4);
  stbir_resize_uint8_linear(data, w, h, 0, resized_data.data(), width, height, 0, STBIR_RGBA);
  stbi_image_free(data);

  // Encode to AVIF
  avifRGBImage rgb = {.width = static_cast<uint32_t>(width),
                      .height = static_cast<uint32_t>(height),
                      .depth = 8,
                      .format = AVIF_RGB_FORMAT_RGBA,
                      .pixels = resized_data.data(),
                      .rowBytes = static_cast<uint32_t>(width * 4)};

  avifEncoder *encoder = avifEncoderCreate();
  avifEncoderSetCodecSpecificOption(encoder, "end-usage", "q");
  avifEncoderSetCodecSpecificOption(encoder, "cq-level", "30");
  avifImage *image =
      avifImageCreate(width, height, 8, AVIF_PIXEL_FORMAT_YUV444);
  avifImageRGBToYUV(image, &rgb);

  avifRWData output = AVIF_DATA_EMPTY;
  avifResult encodeResult = avifEncoderWrite(encoder, image, &output);
  avifEncoderDestroy(encoder);
  avifImageDestroy(image);

  if (encodeResult != AVIF_RESULT_OK) {
    return result;
  }

  result.data = static_cast<unsigned char *>(malloc(output.size));
  if (result.data) {
    memcpy(result.data, output.data, output.size);
    result.size = static_cast<int>(output.size);
  }
  avifRWDataFree(&output);

  return result;
}

void free_buffer(unsigned char *buffer) { free(buffer); }

} // extern "C"