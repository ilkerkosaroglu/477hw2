#include "Translation.h"
#include "Helpers.h"
#include <iostream>
#include <iomanip>

using namespace std;

Translation::Translation()
{
    this->translationId = -1;
    this->tx = 0.0;
    this->ty = 0.0;
    this->tz = 0.0;
}

Translation::Translation(int translationId, double tx, double ty, double tz)
{
    this->translationId = translationId;
    this->tx = tx;
    this->ty = ty;
    this->tz = tz;
}

Matrix4 Translation::getMatrix(){
    if(initializedMatrix){
        return matrix;
    }

    matrix = getIdentityMatrix();
    matrix.val[0][3]=tx;
    matrix.val[1][3]=ty;
    matrix.val[2][3]=tz;

    initializedMatrix = true;
    return matrix;
}

ostream &operator<<(ostream &os, const Translation &t)
{
    os << fixed << setprecision(3) << "Translation " << t.translationId << " => [" << t.tx << ", " << t.ty << ", " << t.tz << "]";

    return os;
}