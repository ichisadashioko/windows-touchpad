#pragma once
#include "kankaku_serialization.h"

#include <stdint.h>

#include "kankaku_utils.h"

int kankaku_serialize_device_dimensions(kankaku_device_dimensions deviceDimensions, uint8_t** serializedData, size_t* serializedDataBytesCount)
{
  int retval = 0;

  size_t pSerializedDataBytesCount = 4 * sizeof(uint8_t);
  uint8_t* pSerializedData         = kankaku_utils_malloc_or_die(pSerializedDataBytesCount, __FILE__, __LINE__);

  pSerializedData[0] = deviceDimensions.width & 0xff;
  pSerializedData[1] = deviceDimensions.width >> 8;

  pSerializedData[2] = deviceDimensions.height & 0xff;
  pSerializedData[3] = deviceDimensions.height >> 8;

  (*serializedData)           = pSerializedData;
  (*serializedDataBytesCount) = pSerializedDataBytesCount;

  return retval;
}

int kankaku_serialize_contact_info(kankaku_contact_info contactInfo, uint8_t** serializedData, size_t* serializedDataBytesCount)
{
  int retval = 0;

  size_t pSerializedDataBytesCount = 6 * sizeof(uint8_t);
  uint8_t* pSerializedData         = kankaku_utils_malloc_or_die(pSerializedDataBytesCount, __FILE__, __LINE__);

  pSerializedData[0] = contactInfo.id;
  pSerializedData[1] = contactInfo.onSurface;

  pSerializedData[2] = contactInfo.x & 0xff;
  pSerializedData[3] = contactInfo.x >> 8;

  pSerializedData[4] = contactInfo.y & 0xff;
  pSerializedData[5] = contactInfo.y >> 8;

  (*serializedData)           = pSerializedData;
  (*serializedDataBytesCount) = pSerializedDataBytesCount;

  return retval;
}
