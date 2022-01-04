#ifndef __TRANSLATION_H__
#define __TRANSLATION_H__

#include <iostream>
#include "Matrix4.h"

using namespace std;

class Translation
{
public:
    int translationId;
    double tx, ty, tz;
    Matrix4 matrix;
    bool initializedMatrix = false;

    Translation();
    Translation(int translationId, double tx, double ty, double tz);
    Matrix4 getMatrix();
    friend ostream &operator<<(ostream &os, const Translation &t);
};

#endif