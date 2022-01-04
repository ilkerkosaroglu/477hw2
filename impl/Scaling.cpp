#include "Scaling.h"
#include "Helpers.h"
#include <iostream>
#include <iomanip>

using namespace std;

Scaling::Scaling() {}

Scaling::Scaling(int scalingId, double sx, double sy, double sz)
{
    this->scalingId = scalingId;
    this->sx = sx;
    this->sy = sy;
    this->sz = sz;
}

Matrix4 Scaling::getMatrix(){
    if(initializedMatrix){
        return matrix;
    }

    matrix = getIdentityMatrix();
    matrix.val[0][0]=sx;
    matrix.val[1][1]=sy;
    matrix.val[2][2]=sz;

    initializedMatrix = true;
    return matrix;
}

ostream &operator<<(ostream &os, const Scaling &s)
{
    os << fixed << setprecision(3) << "Scaling " << s.scalingId << " => [" << s.sx << ", " << s.sy << ", " << s.sz << "]";

    return os;
}
