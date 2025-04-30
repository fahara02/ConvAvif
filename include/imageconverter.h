#ifndef IMAGECONVERTER_H
#define IMAGECONVERTER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  unsigned char *data;
  int size;
} Buffer;

Buffer convert_image(const char *input, int input_size, int width, int height);
void free_buffer(unsigned char *buffer);

#ifdef __cplusplus
}
#endif

#endif