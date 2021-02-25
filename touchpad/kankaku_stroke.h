#ifndef __STROKE_H__
#define __STROKE_H__
#include "kankaku_point2d.h"

struct StrokeList
{
  Point2DList* Entries;
  unsigned int Size;
};

typedef struct StrokeList StrokeList;

int kankaku_stroke_create_and_append_a_new_stroke(Point2D point, StrokeList* strokes);

int kankaku_stroke_add_point(Point2D point, StrokeList* strokes, int isCreatingNewStroke, int dieOnFailure);

#endif  // __STROKE_H__
