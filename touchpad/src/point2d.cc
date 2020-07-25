#include "point2d.h"

int mInitializePoint2DList(Point2D point, Point2DList* list) {
  int retval = 0;
  if (list == NULL) {
    retval = -1;
    std::cout << FG_RED << "list argument is NULL!" << RESET_COLOR << std::endl;
    throw;
    exit(-1);
  } else {
    list->Size    = 1;
    list->Entries = (Point2D*)malloc(sizeof(Point2D));

    if (list->Entries == NULL) {
      retval = -1;
      std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
      throw;
      exit(-1);
    } else {
      list->Entries[0].X = point.X;
      list->Entries[0].Y = point.Y;
    }
  }

  return retval;
}

int mAppendPoint2DToList(Point2D point, Point2DList* list) {
  int retval = 0;
  if (list == NULL) {
    retval = -1;
    std::cout << FG_RED << "list argument is NULL!" << RESET_COLOR << std::endl;
    throw;
    exit(-1);
  } else {
    if ((list->Entries == NULL) || (list->Size == 0)) {
      retval = mInitializePoint2DList(point, list);
    } else {
      unsigned int newArraySize = list->Size + 1;
      Point2D* newArray         = (Point2D*)malloc(sizeof(Point2D) * newArraySize);

      if (newArray == NULL) {
        retval = -1;
        std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
        throw;
        exit(-1);
      } else {
        for (unsigned int pIdx = 0; pIdx < list->Size; pIdx++) {
          newArray[pIdx].X = list->Entries[pIdx].X;
          newArray[pIdx].Y = list->Entries[pIdx].Y;
        }

        free(list->Entries);
        list->Entries                   = newArray;
        list->Size                      = newArraySize;
        list->Entries[list->Size - 1].X = point.X;
        list->Entries[list->Size - 1].Y = point.Y;
      }
    }
  }

  return retval;
}

void test_mAppendPoint2DToList() {
  std::cout << "test_mAppendPoint2DToList" << std::endl;
  int cFuncRetval;

  // #1

  // setup test
  Point2D p1        = {2, 5};
  Point2D p2        = {6, 3};
  Point2DList pList = {NULL, 0};

  // execute
  cFuncRetval = mAppendPoint2DToList(p1, &pList);

  // assert
  if (cFuncRetval != 0) {
    std::cout << FG_RED << "- FAILED" << RESET_COLOR << " mAppendPoint2DToList does not return 0!" << std::endl;
    std::cout << FG_RED << "Abort at " << __FILE__ << ":" << __LINE__ << "!" << RESET_COLOR << std::endl;
    return;
  }

  if ((&pList) == NULL) {
    std::cout << FG_RED << "- FAILED" << RESET_COLOR << " pList became NULL!" << std::endl;
    std::cout << FG_RED << "Abort at " << __FILE__ << ":" << __LINE__ << "!" << RESET_COLOR << std::endl;
    return;
  }

  if (pList.Size != 1) {
    std::cout << FG_RED << "- FAILED" << RESET_COLOR << " pList.Size  (" << pList.Size << ") does not equal 1!" << std::endl;
    std::cout << FG_RED << "Abort at " << __FILE__ << ":" << __LINE__ << "!" << RESET_COLOR << std::endl;
    return;
  }

  if (pList.Entries == NULL) {
    std::cout << FG_RED << "- FAILED" << RESET_COLOR << " pList.Entries is NULL!" << std::endl;
    std::cout << FG_RED << "Abort at " << __FILE__ << ":" << __LINE__ << "!" << RESET_COLOR << std::endl;
    return;
  }

  if (pList.Entries[0].X != p1.X) {
    std::cout << FG_RED << "- FAILED" << RESET_COLOR << " pList.Entries[0].X (" << pList.Entries[0].X << ") does not equal p1.X (" << p1.X << ")!" << std::endl;
    std::cout << FG_RED << "Abort at " << __FILE__ << ":" << __LINE__ << "!" << RESET_COLOR << std::endl;
    return;
  }

  if (pList.Entries[0].Y != p1.Y) {
    std::cout << FG_RED << "- FAILED" << RESET_COLOR << " pList.Entries[0].Y (" << pList.Entries[0].Y << ") does not equal p1.Y (" << p1.Y << ")!" << std::endl;
    std::cout << FG_RED << "Abort at " << __FILE__ << ":" << __LINE__ << "!" << RESET_COLOR << std::endl;
    return;
  }

  // #2
  cFuncRetval = mAppendPoint2DToList(p2, &pList);
  if (cFuncRetval != 0) {
    std::cout << FG_RED << "- FAILED" << RESET_COLOR << " mAppendPoint2DToList does not return 0!" << std::endl;
    std::cout << FG_RED << "Abort at " << __FILE__ << ":" << __LINE__ << "!" << RESET_COLOR << std::endl;
    return;
  }

  if ((&pList) == NULL) {
    std::cout << FG_RED << "- FAILED" << RESET_COLOR << " pList became NULL!" << std::endl;
    std::cout << FG_RED << "Abort at " << __FILE__ << ":" << __LINE__ << "!" << RESET_COLOR << std::endl;
    return;
  }

  if (pList.Size != 2) {
    std::cout << FG_RED << "- FAILED" << RESET_COLOR << " pList.Size  (" << pList.Size << ") does not equal 2!" << std::endl;
    std::cout << FG_RED << "Abort at " << __FILE__ << ":" << __LINE__ << "!" << RESET_COLOR << std::endl;
    return;
  }

  if (pList.Entries == NULL) {
    std::cout << FG_RED << "- FAILED" << RESET_COLOR << " pList.Entries is NULL!" << std::endl;
    std::cout << FG_RED << "Abort at " << __FILE__ << ":" << __LINE__ << "!" << RESET_COLOR << std::endl;
    return;
  }

  if (pList.Entries[0].X != p1.X) {
    std::cout << FG_RED << "- FAILED" << RESET_COLOR << " pList.Entries[0].X (" << pList.Entries[0].X << ") does not equal p1.X (" << p1.X << ")!" << std::endl;
    std::cout << FG_RED << "Abort at " << __FILE__ << ":" << __LINE__ << "!" << RESET_COLOR << std::endl;
    return;
  }

  if (pList.Entries[0].Y != p1.Y) {
    std::cout << FG_RED << "- FAILED" << RESET_COLOR << " pList.Entries[0].Y (" << pList.Entries[0].Y << ") does not equal p1.Y (" << p1.Y << ")!" << std::endl;
    std::cout << FG_RED << "Abort at " << __FILE__ << ":" << __LINE__ << "!" << RESET_COLOR << std::endl;
    return;
  }

  if (pList.Entries[1].X != p2.X) {
    std::cout << FG_RED << "- FAILED" << RESET_COLOR << " pList.Entries[1].X (" << pList.Entries[1].X << ") does not equal p2.X (" << p2.X << ")!" << std::endl;
    std::cout << FG_RED << "Abort at " << __FILE__ << ":" << __LINE__ << "!" << RESET_COLOR << std::endl;
    return;
  }

  if (pList.Entries[1].Y != p2.Y) {
    std::cout << FG_RED << "- FAILED" << RESET_COLOR << " pList.Entries[1].Y (" << pList.Entries[1].Y << ") does not equal p2.Y (" << p2.Y << ")!" << std::endl;
    std::cout << FG_RED << "Abort at " << __FILE__ << ":" << __LINE__ << "!" << RESET_COLOR << std::endl;
    return;
  }

  std::cout << FG_GREEN << "- OK" << RESET_COLOR << " All tests are passed." << std::endl;
}
