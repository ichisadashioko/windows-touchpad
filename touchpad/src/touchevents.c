#include <stdio.h>

#include "touchevents.h"
#include "termcolor.h"
#include "utils.h"

int mInterpretRawTouchInput(TOUCH_DATA_LIST* prevTouchesList, TOUCH_DATA curTouch, unsigned int* eventType)
{
  // check arguments
  if (eventType == NULL)
  {
    printf(FG_RED);
    printf("You must pass a unsigned int pointer eventType. It's NULL right now!\n");
    printf(RESET_COLOR);
    exit(-1);
    return -1;
  }

  if (prevTouchesList == NULL)
  {
    printf(FG_RED);
    printf("You must pass a valid pointer for prevTouchesList. It's NULL right now!\n");
    printf(RESET_COLOR);
    exit(-1);
    return -1;
  }

  if ((prevTouchesList->Entries == NULL) || (prevTouchesList->Size == 0))
  {
    prevTouchesList->Entries = (TOUCH_DATA*)mMalloc(sizeof(TOUCH_DATA), __FILE__, __LINE__);
    prevTouchesList->Size    = 1;

    prevTouchesList->Entries[0] = curTouch;

    if (curTouch.OnSurface)
    {
      (*eventType) = EVENT_TYPE_TOUCH_DOWN;
    }
    else
    {
      (*eventType) = EVENT_TYPE_TOUCH_UP;
    }

    return 0;
  }
  else
  {
    for (unsigned int touchIdx = 0; touchIdx < prevTouchesList->Size; touchIdx++)
    {
      TOUCH_DATA prevTouch = prevTouchesList->Entries[touchIdx];
      if (prevTouch.TouchID == curTouch.TouchID)
      {
        if (prevTouch.OnSurface && curTouch.OnSurface)
        {
          if ((prevTouch.X == curTouch.X) && (prevTouch.Y == curTouch.Y))
          {
            (*eventType) = EVENT_TYPE_TOUCH_MOVE_UNCHANGED;
          }
          else
          {
            (*eventType) = EVENT_TYPE_TOUCH_MOVE;
          }
        }
        else if ((prevTouch.OnSurface != 0) && (curTouch.OnSurface == 0))
        {
          (*eventType) = EVENT_TYPE_TOUCH_UP;
        }
        else if ((prevTouch.OnSurface == 0) && (curTouch.OnSurface != 0))
        {
          (*eventType) = EVENT_TYPE_TOUCH_DOWN;
        }
        else
        {
          // (prevTouch.OnSurface == 0) && (curTouch.OnSurface == 0)
          // this might never be the case unless the touchpad or its driver is broken
          (*eventType) = EVENT_TYPE_TOUCH_UP;
        }

        // update touch data
        prevTouchesList->Entries[touchIdx] = curTouch;

        return 0;
      }
    }
  }

  // this touch id has not been recorded before

  // If you touchpad only supports maximum of 5 touches, then there will be only 5 unique touch IDs.
  unsigned int newTouchesListSize = prevTouchesList->Size + 1;
  TOUCH_DATA* tmpTouchesList      = (TOUCH_DATA*)mMalloc(sizeof(TOUCH_DATA) * newTouchesListSize, __FILE__, __LINE__);

  memcpy(tmpTouchesList, prevTouchesList->Entries, sizeof(TOUCH_DATA) * prevTouchesList->Size);

  tmpTouchesList[newTouchesListSize - 1] = curTouch;
  free(prevTouchesList->Entries);
  prevTouchesList->Entries = tmpTouchesList;
  tmpTouchesList           = NULL;
  prevTouchesList->Size    = newTouchesListSize;

  if (curTouch.OnSurface)
  {
    (*eventType) = EVENT_TYPE_TOUCH_MOVE;
  }
  else
  {
    // edge case
    // how can a touch be lifted while it was not on the touchpad surface.
    // probably because of faulty hardware or driver
    (*eventType) = EVENT_TYPE_TOUCH_UP;
  }

  return 0;
}
