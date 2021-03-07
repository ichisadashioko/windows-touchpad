#ifndef __KANKAKU_SERIALIZATION_H__
#define __KANKAKU_SERIALIZATION_H__

#include <stdint.h>

typedef struct
{
  uint16_t width;
  uint16_t height;
} kankaku_device_dimensions;

int kankaku_serialize_device_dimensions(kankaku_device_dimensions deviceDimensions, uint8_t** serializedData, size_t* serializedDataBytesCount);

#endif  // __KANKAKU_SERIALIZATION_H__
