#include <Windows.h>

#include <stdio.h>

#include "point2d.h"

#include "utils.h"
#include "termcolor.h"

int mInitializePoint2DList(Point2D point, Point2DList* list)
{
  int retval = 0;
  if (list == NULL)
  {
    retval = -1;
    printf(FG_RED);
    printf("list argument is NULL!\n");
    printf(RESET_COLOR);
    exit(-1);
  }
  else
  {
    list->Size    = 1;
    list->Entries = (Point2D*)mMalloc(sizeof(Point2D), __FILE__, __LINE__);

    list->Entries[0].X = point.X;
    list->Entries[0].Y = point.Y;
  }

  return retval;
}

int mAppendPoint2DToList(Point2D point, Point2DList* list)
{
  int retval = 0;
  if (list == NULL)
  {
    retval = -1;
    printf(FG_RED);
    printf("list argument is NULL!\n");
    printf(RESET_COLOR);
    exit(-1);
  }
  else
  {
    if ((list->Entries == NULL) || (list->Size == 0))
    {
      retval = mInitializePoint2DList(point, list);
    }
    else
    {
      unsigned int newArraySize = list->Size + 1;
      Point2D* newArray         = (Point2D*)mMalloc(sizeof(Point2D) * newArraySize, __FILE__, __LINE__);

      for (unsigned int pIdx = 0; pIdx < list->Size; pIdx++)
      {
        newArray[pIdx].X = list->Entries[pIdx].X;
        newArray[pIdx].Y = list->Entries[pIdx].Y;
      }

      newArray[list->Size].X = point.X;
      newArray[list->Size].Y = point.Y;

      free(list->Entries);

      list->Entries = newArray;
      list->Size    = newArraySize;
    }
  }

  return retval;
}
