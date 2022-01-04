#ifndef __SCALING_H__
#define __SCALING_H__

#include <iostream>
#include "Matrix4.h"

using namespace std;

class Scaling
{
public:
    int scalingId;
    double sx, sy, sz;
    Matrix4 matrix;
    bool initializedMatrix = false;

    Scaling();
    Scaling(int scalingId, double sx, double sy, double sz);
    Matrix4 getMatrix();
    friend ostream &operator<<(ostream &os, const Scaling &s);
};

#endif