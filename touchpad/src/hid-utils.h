#include <Windows.h>

struct GET_NUM_DEVICES_RETURN {
  UINT NumDevices;
  UINT ErrorCode;
};

void printTimestamp();
void printLastError();
GET_NUM_DEVICES_RETURN GetNumberOfDevices();