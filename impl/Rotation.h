#ifndef __ROTATION_H__
#define __ROTATION_H__

#include <iostream>
#include "Matrix4.h"

using namespace std;

class Rotation
{
public:
    int rotationId;
    double angle, ux, uy, uz;
    Matrix4 matrix;
    bool initializedMatrix = false;

    Rotation();
    Rotation(int rotationId, double angle, double x, double y, double z);
    Matrix4 getMatrix();
    friend ostream &operator<<(ostream &os, const Rotation &r);
};

#endif