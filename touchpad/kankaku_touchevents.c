#include "kankaku_touchevents.h"

#include <stdio.h>

#include "termcolor.h"
#include "kankaku_utils.h"

/*
Given an existing collection of TOUCH_DATA and a new TOUCH_DATA element, we need to interpret the raw touch input data to touch events (start - move - end).

After interpreting the event, we need to update the collection for tracking and interpreting more touch input data later.
*/
int kankaku_touchevents_interpret_event_type(TOUCH_DATA_LIST* prevTouchesList, TOUCH_DATA curTouch, unsigned int* eventType)
{
  int retval = 0;

#pragma region input checking
  if (eventType == NULL)
  {
    fprintf(stderr, "%sYou must pass a unsigned int pointer eventType. It's NULL right now!%s\n", FG_RED, RESET_COLOR);
    retval = -1;
    // TODO add dieOnFailure flag
    exit(-1);
  }
  else if (prevTouchesList == NULL)
  {
    fprintf(stderr, "%sYou must pass a valid pointer for prevTouchesList. It's NULL right now!%s\n", FG_RED, RESET_COLOR);
    retval = -1;
    // TODO add dieOnFailure flag
    exit(-1);
  }
  else
#pragma endregion end input checking
  {
    if ((prevTouchesList->Entries == NULL) || (prevTouchesList->Size == 0))
    {
      prevTouchesList->Entries = (TOUCH_DATA*)kankaku_utils_malloc_or_die(sizeof(TOUCH_DATA), __FILE__, __LINE__);
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
        }
      }
    }

    return retval;
  }

  // this touch id has not been recorded before

  // If you touchpad only supports maximum of 5 touches, then there will be only 5 unique touch IDs.
  unsigned int newTouchesListSize = prevTouchesList->Size + 1;
  TOUCH_DATA* tmpTouchesList      = (TOUCH_DATA*)kankaku_utils_malloc_or_die(sizeof(TOUCH_DATA) * newTouchesListSize, __FILE__, __LINE__);

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
