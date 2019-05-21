// Copyright 2019 Aksenov Nikita

#include <cmath>

#ifndef MODULES_LINE_PLANE_INTERSECTION_INCLUDE_LINE_PLANE_INTERSECTION_H_
#define MODULES_LINE_PLANE_INTERSECTION_INCLUDE_LINE_PLANE_INTERSECTION_H_

struct Dot {
    double x, y, z;
};

class LinePlaneIntersection {
 public:
    LinePlaneIntersection();

    Dot PlaneIntersectLine(Dot X, Dot Y, Dot Z, Dot A, Dot B);
    bool GetNotIntersectPlaneLine();

 private:
    bool NotIntersectPlaneLine;

    Dot CreateVector(Dot A, Dot B);
    Dot VectorProduct(Dot A, Dot B);
    double ScalarProduct(Dot A, Dot B);
    bool IncorrectLine(Dot A, Dot B);
    bool IncorrectPlane(Dot A, Dot B, Dot C);
    void Normalize(Dot A);
};

#endif  // MODULES_LINE_PLANE_INTERSECTION_INCLUDE_LINE_PLANE_INTERSECTION_H_
