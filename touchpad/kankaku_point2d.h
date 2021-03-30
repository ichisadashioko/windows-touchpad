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

int kankaku_point2d_initialize_list_with_a_single_element(Point2D point, Point2DList* list);
int kankaku_point2d_append_element(Point2D point, Point2DList* list);
#endif  // __POINT2D_H__
