// Copyright 2019 Aksenov Nikita

#ifndef MODULES_LINE_PLANE_INTERSECTION_INCLUDE_LINE_PLANE_INTERSECTION_H_
#define MODULES_LINE_PLANE_INTERSECTION_INCLUDE_LINE_PLANE_INTERSECTION_H_

#include <cmath>

struct Dot {
    double x, y, z;
};

class LinePlaneIntersection {
 public:
    LinePlaneIntersection();

    Dot PlaneIntersectLine(Dot X, Dot Y, Dot Z, Dot A, Dot B);
    bool GetNotIntersectPlaneLine();

    friend bool operator== (const Dot A, const Dot B);

 private:
    bool NotIntersectPlaneLine{ true };

    Dot CreateVector(Dot A, Dot B);
    Dot VectorProduct(Dot A, Dot B);
    double ScalarProduct(Dot A, Dot B);
    bool IncorrectLine(const Dot A, const Dot B);
    bool IncorrectPlane(const Dot A, const Dot B, const Dot C);
    void Normalize(Dot A);
};

#endif  // MODULES_LINE_PLANE_INTERSECTION_INCLUDE_LINE_PLANE_INTERSECTION_H_
