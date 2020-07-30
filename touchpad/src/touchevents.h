#ifndef _TOUCHEVENTS_H_
#define _TOUCHEVENTS_H_
#include <Windows.h>

static const unsigned int EVENT_TYPE_TOUCH_DOWN           = 0;
static const unsigned int EVENT_TYPE_TOUCH_MOVE           = 1;
static const unsigned int EVENT_TYPE_TOUCH_UP             = 2;
static const unsigned int EVENT_TYPE_TOUCH_MOVE_UNCHANGED = 3;

struct TOUCH_DATA
{
  ULONG TouchID;
  ULONG X;
  ULONG Y;
  int OnSurface;
};

struct TOUCH_DATA_LIST
{
  TOUCH_DATA* Entries;
  unsigned int Size;
};

int mInterpretRawTouchInput(TOUCH_DATA_LIST* prevTouchesList, TOUCH_DATA curTouch, unsigned int* eventType);

#endif  // _TOUCHEVENTS_H_
