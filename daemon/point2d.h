#ifndef __POINT2D_H__
#define __POINT2D_H__
#include <Windows.h>

struct Point2D
{
  ULONG X;
  ULONG Y;
};

typedef struct Point2D Point2D;

struct Point2DList
{
  Point2D* Entries;
  unsigned int Size;
};

typedef struct Point2DList Point2DList;

int mInitializePoint2DList(Point2D point, Point2DList* list);
int mAppendPoint2DToList(Point2D point, Point2DList* list);
void test_mAppendPoint2DToList();
#endif  // __POINT2D_H__
