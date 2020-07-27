#ifndef _STROKE_H_
#define _STROKE_H_
#include "point2d.h"

struct StrokeList {
  Point2DList* Entries;
  unsigned int Size;
};

int mCreateNewStroke(Point2D point, StrokeList* strokes);
#endif  // _STROKE_H_
