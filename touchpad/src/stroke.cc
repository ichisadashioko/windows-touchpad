#include <Windows.h>
#include <iostream>

#include "termcolor.h"
#include "utils.h"
#include "stroke.h"

int mCreateNewStroke(Point2D point, StrokeList* strokes)
{
  if (strokes == NULL)
  {
    std::cout << FG_RED << "strokes argument is NULL!" << RESET_COLOR << std::endl;
    throw;
    exit(-1);
    return -1;
  }

  if ((strokes->Entries == NULL) || (strokes->Size == 0))
  {
    strokes->Size    = 1;
    strokes->Entries = (Point2DList*)mMalloc(sizeof(Point2DList), __FILE__, __LINE__);

    // TODO check for memory access violation
    strokes->Entries[0].Entries = NULL;
    strokes->Entries[0].Size    = 0;
    return mAppendPoint2DToList(point, &(strokes->Entries[0]));
  }
  else
  {
    unsigned int newStrokeArraySize = strokes->Size + 1;
    Point2DList* newStrokeArray     = (Point2DList*)mMalloc(sizeof(Point2DList) * newStrokeArraySize, __FILE__, __LINE__);

    for (unsigned int strokeIdx = 0; strokeIdx < strokes->Size; strokeIdx++)
    {
      newStrokeArray[strokeIdx] = strokes->Entries[strokeIdx];
    }

    // memcpy(newStrokeArray, strokes->Entries, sizeof(Point2DList) * strokes->Size);
    free(strokes->Entries);

    strokes->Entries                    = newStrokeArray;
    strokes->Size                       = newStrokeArraySize;
    strokes->Entries[strokes->Size - 1] = {NULL, 0};
    return mAppendPoint2DToList(point, &(strokes->Entries[strokes->Size - 1]));
  }

  return 0;
}
