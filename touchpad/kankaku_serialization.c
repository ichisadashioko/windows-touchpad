#pragma once
#include "kankaku_serialization.h"

#include <stdint.h>

#include "kankaku_utils.h"

int kankaku_serialize_device_dimensions(kankaku_device_dimensions deviceDimensions, uint8_t** serializedData, size_t* serializedDataByteCount)
{
  int retval = 0;

  size_t pSerializedDataByteCount = 4 * sizeof(uint8_t);
  uint8_t* pSerializedData        = kankaku_utils_malloc_or_die(pSerializedDataByteCount, __FILE__, __LINE__);

  pSerializedData[0] = deviceDimensions.width & 0xff;
  pSerializedData[1] = deviceDimensions.width >> 8;
  pSerializedData[2] = deviceDimensions.height & 0xff;
  pSerializedData[3] = deviceDimensions.height >> 8;

  (*serializedData)          = pSerializedData;
  (*serializedDataByteCount) = pSerializedDataByteCount;

  return retval;
}
