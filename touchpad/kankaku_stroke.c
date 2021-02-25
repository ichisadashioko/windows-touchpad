#include <Windows.h>

#include <stdio.h>

#include "termcolor.h"
#include "utils.h"
#include "kankaku_stroke.h"

/*
Add `point` to the collection `strokes`.

If `isCreatingNewStroke` is set to a non-zero value, the function will add the `point` to the last stroke in `strokes`. If `strokes.Size` equals `0` or `strokes.Entries` equals `NULL`, the function will fail.

If `dieOnFailure` is set to a non-zero value, the function will call `exit(-1)` on failure. Otherwise, the function returns non-zero value to indicate failure.
 */
int kankaku_stroke_add_point(Point2D point, StrokeList* strokes, int isCreatingNewStroke, int dieOnFailure)
{
  int retval = 0;

#pragma region input checking
  if (strokes == NULL)
  {
    printf(FG_RED);
    printf("strokes argument is NULL!\n");
    printf(RESET_COLOR);
    retval = -1;

    if (dieOnFailure == 0)
    {
      return retval;
    }
    else
    {
      exit(-1);
    }
  }
  else
#pragma endregion end input checking
  {
    int isCollectionEmpty = (strokes->Entries == NULL) || (strokes->Size == 0);

    if (isCreatingNewStroke)
    {
      if (isCollectionEmpty)
      {
        strokes->Size    = 1;
        strokes->Entries = (Point2DList*)kankaku_utils_malloc_or_die(sizeof(Point2DList), __FILE__, __LINE__);

        retval = kankaku_point2d_append_element(point, &(strokes->Entries[0]));
      }
      else
      {
        retval = kankaku_point2d_append_element(point, &(strokes->Entries[strokes->Size - 1]));
      }
    }
    else
    {
      if (isCollectionEmpty)
      {
        retval = -1;
        fprintf(stderr, "Cannot add point an empty collection! %s:%d\n", __FILE__, __LINE__);

        if (dieOnFailure)
        {
          exit(-1);
        }
      }
      else
      {
        retval = kankaku_point2d_append_element(point, &(strokes->Entries[strokes->Size - 1]));
      }
    }
  }

  return retval;
}

int kankaku_stroke_create_and_append_a_new_stroke(Point2D point, StrokeList* strokes)
{
  if (strokes == NULL)
  {
    printf(FG_RED);
    printf("strokes argument is NULL!\n");
    printf(RESET_COLOR);

#ifdef DIE_ON_FAIL
#if DIE_ON_FAIL == 1
    exit(-1);
#else
    return -1;
#endif  // #if DIE_ON_FAIL == 1
#endif  // #ifdef DIE_ON_FAIL
  }

  if ((strokes->Entries == NULL) || (strokes->Size == 0))
  {
    strokes->Size    = 1;
    strokes->Entries = (Point2DList*)kankaku_utils_malloc_or_die(sizeof(Point2DList), __FILE__, __LINE__);

    // TODO check for memory access violation
    strokes->Entries[0].Entries = NULL;
    strokes->Entries[0].Size    = 0;
    return kankaku_point2d_append_element(point, &(strokes->Entries[0]));
  }
  else
  {
    unsigned int newStrokeArraySize = strokes->Size + 1;
    Point2DList* newStrokeArray     = (Point2DList*)kankaku_utils_malloc_or_die(sizeof(Point2DList) * newStrokeArraySize, __FILE__, __LINE__);

    for (unsigned int strokeIdx = 0; strokeIdx < strokes->Size; strokeIdx++)
    {
      newStrokeArray[strokeIdx] = strokes->Entries[strokeIdx];
    }

    // memcpy(newStrokeArray, strokes->Entries, sizeof(Point2DList) * strokes->Size);
    free(strokes->Entries);

    strokes->Entries                    = newStrokeArray;
    strokes->Size                       = newStrokeArraySize;
    strokes->Entries[strokes->Size - 1] = (Point2DList){.Entries = NULL, .Size = 0};
    return kankaku_point2d_append_element(point, &(strokes->Entries[strokes->Size - 1]));
  }

  return 0;
}
