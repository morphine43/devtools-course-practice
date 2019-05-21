// Copyright 2019 Aksenov Nikita

#include "include/line_plane_intersection.h"

LinePlaneIntersection::LinePlaneIntersection() :
    NotIntersectPlaneLine(true) {}

Dot LinePlaneIntersection::CreateVector(Dot A, Dot B) {
    Dot CreateVector;

    CreateVector.x = B.x - A.x;
    CreateVector.y = B.y - A.y;
    CreateVector.z = B.z - A.z;

    return CreateVector;
}

Dot LinePlaneIntersection::VectorProduct(Dot A, Dot B) {
    Dot VectorProduct;

    VectorProduct.x = A.y * B.z - B.y * A.z;
    VectorProduct.y = A.z * B.x - B.z * A.x;
    VectorProduct.z = A.x * B.y - B.x * A.y;

    return VectorProduct;
}

double LinePlaneIntersection::ScalarProduct(Dot A, Dot B) {
    double ScalarProduct;

    ScalarProduct = A.x * B.x + A.y * B.y + A.z * B.z;

    return ScalarProduct;
}

void LinePlaneIntersection::Normalize(Dot A) {
    double mvn; // модуль вектора нормали

    mvn = sqrt(A.x * A.x + A.y * A.y + A.z * A.z);
    A.x = A.x / mvn;
    A.y = A.y / mvn;
    A.z = A.z / mvn;
}

bool LinePlaneIntersection::IncorrectLine(Dot A, Dot B) {
    if (A.x == B.x && A.y == B.y && A.z == B.z)
        return true;
    return false;
}

bool LinePlaneIntersection::IncorrectPlane(Dot A, Dot B, Dot C) {
    if (A.x == B.x && A.y == B.y && A.z == B.z)
        return true;
    if (A.x == C.x && A.y == C.y && A.z == C.z)
        return true;
    if (C.x == B.x && C.y == B.y && C.z == B.z)
        return true;
    return false;
}

Dot LinePlaneIntersection::PlaneIntersectLine(Dot A, Dot B, Dot C, Dot X, Dot Y) {
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
