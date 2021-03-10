#ifndef __KANKAKU_SERIALIZATION_H__
#define __KANKAKU_SERIALIZATION_H__

#include <stdint.h>

typedef struct
{
  uint16_t width;
  uint16_t height;
} kankaku_device_dimensions;

typedef struct
{
  uint8_t id;
  uint8_t onSurface;
  uint16_t x;
  uint16_t y;
} kankaku_contact_info;

int kankaku_serialize_device_dimensions(kankaku_device_dimensions deviceDimensions, uint8_t** serializedData, size_t* serializedDataBytesCount);

int kankaku_serialize_contact_info(kankaku_contact_info contactInfo, uint8_t** serializedData, size_t* serializedDataBytesCount);

#endif  // __KANKAKU_SERIALIZATION_H__
