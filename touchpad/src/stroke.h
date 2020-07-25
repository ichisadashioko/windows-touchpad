#ifndef STROKE_H
#define STROKE_H
#include "point2d.h"

struct StrokeList {
  Point2DList* Entries;
  unsigned int Size;
};

int mCreateNewStroke(Point2D point, StrokeList* strokes);
#endif  // STROKE_H
