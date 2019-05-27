// Copyright 2019 Aksenov Nikita

#include "include/line_plane_intersection.h"

LinePlaneIntersection::LinePlaneIntersection() {
}

Dot LinePlaneIntersection::CreateVector(Dot A, Dot B) {
    Dot CV;

    CV.x = B.x - A.x;
    CV.y = B.y - A.y;
    CV.z = B.z - A.z;

    return CV;
}

Dot LinePlaneIntersection::VectorProduct(Dot A, Dot B) {
    Dot VP;

    VP.x = A.y * B.z - B.y * A.z;
    VP.y = A.z * B.x - B.z * A.x;
    VP.z = A.x * B.y - B.x * A.y;

    return VP;
}

double LinePlaneIntersection::ScalarProduct(Dot A, Dot B) {
    double SP;

    SP = A.x * B.x + A.y * B.y + A.z * B.z;

    return SP;
}

void LinePlaneIntersection::Normalize(Dot A) {
    double mvn;  // module of the vector of the normal

    mvn = sqrt(A.x * A.x + A.y * A.y + A.z * A.z);
    A.x = A.x / mvn;
    A.y = A.y / mvn;
    A.z = A.z / mvn;
}

bool LinePlaneIntersection::IncorrectLine(Dot A, Dot B) {
    if (A == B)
        return true;
    return false;
}

bool LinePlaneIntersection::IncorrectPlane(Dot A, Dot B, Dot C) {
    if (A == B)
        return true;
    if (A == C)
        return true;
    if (C == B)
        return true;
    return false;
}

Dot LinePlaneIntersection::PlaneIntersectLine(Dot A, Dot B, Dot C,
    Dot X, Dot Y) {
    Dot N, V, W, PlaneIntersectLine = { 0, 0, 0 };
    double e, d;

    NotIntersectPlaneLine = true;

    if (IncorrectLine(X, Y))
        throw "Incorrect Line";

    if (IncorrectPlane(A, B, C))
        throw "Incorrect Plane";

    N = VectorProduct(CreateVector(A, B), CreateVector(A, C));
    Normalize(N);
    V = CreateVector(X, A);
    d = ScalarProduct(N, V);
    W = CreateVector(X, Y);
    e = ScalarProduct(N, W);

    if (e != 0) {
        PlaneIntersectLine.x = X.x + W.x * d / e;
        PlaneIntersectLine.y = X.y + W.y * d / e;
        PlaneIntersectLine.z = X.z + W.z * d / e;
        NotIntersectPlaneLine = false;
    }

    return PlaneIntersectLine;
}

bool LinePlaneIntersection::GetNotIntersectPlaneLine() {
    return NotIntersectPlaneLine;
}

bool Dot::operator== (const Dot &A) const {
    return this->x == A.x && this->y == A.y && this->z == A.z;
}
