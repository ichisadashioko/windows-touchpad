#ifndef POINT2D_H
#define POINT2D_H
#include <Windows.h>
#include <iostream>
#include "termcolor.h"

struct Point2D {
  ULONG X;
  ULONG Y;
};

struct Point2DList {
  Point2D* Entries;
  unsigned int Size;
};

int mInitializePoint2DList(Point2D point, Point2DList* list);
int mAppendPoint2DToList(Point2D point, Point2DList* list);
void test_mAppendPoint2DToList();
#endif  // POINT2D_H
