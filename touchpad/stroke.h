#ifndef __STROKE_H__
#define __STROKE_H__
#include "kankaku_point2d.h"

struct StrokeList
{
  Point2DList* Entries;
  unsigned int Size;
};

typedef struct StrokeList StrokeList;

int mCreateNewStroke(Point2D point, StrokeList* strokes);
#endif  // __STROKE_H__
