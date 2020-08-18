#include "point2d.h"
#include "utils.h"

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

void test_mAppendPoint2DToList()
{
  printf("test_mAppendPoint2DToList");
  int cFuncRetval;

  // #1

  // setup test

  Point2D p1        = (Point2D){.X = 2, .Y = 5};
  Point2D p2        = (Point2D){.X = 6, .Y = 3};
  Point2DList pList = (Point2DList){.Entries = NULL, .Size = 0};

  // execute
  cFuncRetval = mAppendPoint2DToList(p1, &pList);

  // assert
  if (cFuncRetval != 0)
  {
    printf(FG_RED);
    printf("- FAILED");
    printf(RESET_COLOR);
    printf(" mAppendPoint2DToList does not return 0!\n");

    printf(FG_RED);
    printf("Abort at %s:%d\n", __FILE__, __LINE__);
    printf(RESET_COLOR);
    return;
  }

  if ((&pList) == NULL)
  {
    printf(FG_RED);
    printf("- FAILED");
    printf(RESET_COLOR);
    printf(" pList became NULL!\n");

    printf(FG_RED);
    printf("Abort at %s:%d\n", __FILE__, __LINE__);
    printf(RESET_COLOR);
    return;
  }

  if (pList.Size != 1)
  {
    printf(FG_RED);
    printf("- FAILED");
    printf(RESET_COLOR);
    printf(" pList.Size  (%d) does not equal 1!\n", pList.Size);

    printf(FG_RED);
    printf("Abort at %s:%d\n", __FILE__, __LINE__);
    printf(RESET_COLOR);
    return;
  }

  if (pList.Entries == NULL)
  {
    printf(FG_RED);
    printf("- FAILED");
    printf(RESET_COLOR);
    printf(" pList.Entries is NULL!\n");

    printf(FG_RED);
    printf("Abort at %s:%d\n", __FILE__, __LINE__);
    printf(RESET_COLOR);
    return;
  }

  if (pList.Entries[0].X != p1.X)
  {
    printf(FG_RED);
    printf("- FAILED");
    printf(RESET_COLOR);
    printf(" pList.Entries[0].X (%d) does not equal p1.X (%d)!\n", pList.Entries[0].X, p1.X);

    printf(FG_RED);
    printf("Abort at %s:%d\n", __FILE__, __LINE__);
    printf(RESET_COLOR);
    return;
  }

  if (pList.Entries[0].Y != p1.Y)
  {
    printf(FG_RED);
    printf("- FAILED");
    printf(RESET_COLOR);
    printf(" pList.Entries[0].Y (%d) does not equal p1.Y (%d)!\n", pList.Entries[0].Y, p1.Y);

    printf(FG_RED);
    printf("Abort at %s:%d\n", __FILE__, __LINE__);
    printf(RESET_COLOR);
    return;
  }

  // #2
  cFuncRetval = mAppendPoint2DToList(p2, &pList);
  if (cFuncRetval != 0)
  {
    printf(FG_RED);
    printf("- FAILED");
    printf(RESET_COLOR);
    printf(" mAppendPoint2DToList does not return 0!\n");

    printf(FG_RED);
    printf("Abort at %s:%d\n", __FILE__, __LINE__);
    printf(RESET_COLOR);
    return;
  }

  if ((&pList) == NULL)
  {
    printf(FG_RED);
    printf("- FAILED");
    printf(RESET_COLOR);
    printf(" pList became NULL!\n");

    printf(FG_RED);
    printf("Abort at %s:%d\n", __FILE__, __LINE__);
    printf(RESET_COLOR);
    return;
  }

  if (pList.Size != 2)
  {
    printf(FG_RED);
    printf("- FAILED");
    printf(RESET_COLOR);
    printf(" pList.Size  (%d) does not equal 2!\n", pList.Size);

    printf(FG_RED);
    printf("Abort at %s:%d\n", __FILE__, __LINE__);
    printf(RESET_COLOR);
    return;
  }

  if (pList.Entries == NULL)
  {
    printf(FG_RED);
    printf("- FAILED");
    printf(RESET_COLOR);
    printf(" pList.Entries is NULL!\n");

    printf(FG_RED);
    printf("Abort at %s:%d\n", __FILE__, __LINE__);
    printf(RESET_COLOR);
    return;
  }

  if (pList.Entries[0].X != p1.X)
  {
    printf(FG_RED);
    printf("- FAILED");
    printf(RESET_COLOR);
    printf(" pList.Entries[0].X (%d) does not equal p1.X (%d)!\n", pList.Entries[0].X, p1.X);

    printf(FG_RED);
    printf("Abort at %s:%d\n", __FILE__, __LINE__);
    printf(RESET_COLOR);
    return;
  }

  if (pList.Entries[0].Y != p1.Y)
  {
    printf(FG_RED);
    printf("- FAILED");
    printf(RESET_COLOR);
    printf(" pList.Entries[0].Y (%d) does not equal p1.Y (%d)!\n", pList.Entries[0].Y, p1.Y);

    printf(FG_RED);
    printf("Abort at %s:%d\n", __FILE__, __LINE__);
    printf(RESET_COLOR);
    return;
  }

  if (pList.Entries[1].X != p2.X)
  {
    printf(FG_RED);
    printf("- FAILED");
    printf(RESET_COLOR);
    printf(" pList.Entries[1].X (%d) does not equal p2.X (%d)!\n", pList.Entries[1].X, p2.X);

    printf(FG_RED);
    printf("Abort at %s:%d\n", __FILE__, __LINE__);
    printf(RESET_COLOR);
    return;
  }

  if (pList.Entries[1].Y != p2.Y)
  {
    printf(FG_RED);
    printf("- FAILED");
    printf(RESET_COLOR);
    printf(" pList.Entries[1].Y (%d) does not equal p2.Y (%d)!\n", pList.Entries[1].Y, p2.Y);

    printf(FG_RED);
    printf("Abort at %s:%d\n", __FILE__, __LINE__);
    printf(RESET_COLOR);
    return;
  }

  printf(FG_GREEN);
  printf("- OK");
  printf(RESET_COLOR);
  printf(" All tests are passed.\n");
}
