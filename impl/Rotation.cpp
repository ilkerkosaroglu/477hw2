#include "Rotation.h"
#include "Helpers.h"
#include "Vec3.h"
#include <iostream>
#include <iomanip>

#include <math.h> 

using namespace std;

Rotation::Rotation() {}

Rotation::Rotation(int rotationId, double angle, double x, double y, double z)
{
    this->rotationId = rotationId;
    this->angle = angle;
    this->ux = x;
    this->uy = y;
    this->uz = z;
}

Matrix4 Rotation::getMatrix(){
    if(initializedMatrix){
        return matrix;
    }

    matrix = getIdentityMatrix();

    //use Alternative Method in slides

    //find orthonormal basis uvw
    Vec3 u(ux,uy,uz,-1);
    u = normalizeVec3(u);
    Vec3 testVector(1,1,1,-1);
    Vec3 v = normalizeVec3(crossProductVec3(u,testVector));
    Vec3 w = crossProductVec3(u,v);

    //create the change of basis matrix M and Rotation(x) matrix Rx. MInv = M^-1
    double rad = angle*M_PI/180.0;
    double MVal[4][4] = {{u.x,u.y,u.z,0},{v.x,v.y,v.z,0},{w.x,w.y,w.z,0},{0,0,0,1}};
    double MInvVal[4][4] = {{u.x,v.x,w.x,0},{u.y,v.y,w.y,0},{u.z,v.z,w.z,0},{0,0,0,1}};
    double RxVal[4][4] = {{1,0,0,0},{0,cos(rad),-sin(rad),0},{0,sin(rad),cos(rad),0},{0,0,0,1}};
    Matrix4 M(MVal);
    Matrix4 MInv(MInvVal);
    Matrix4 Rx(RxVal);

    matrix = multiplyMatrixWithMatrix(M,matrix);
    matrix = multiplyMatrixWithMatrix(Rx,matrix);
    matrix = multiplyMatrixWithMatrix(MInv,matrix);

    initializedMatrix = true;
    return matrix;
}

ostream &operator<<(ostream &os, const Rotation &r)
{
    os << fixed << setprecision(3) << "Rotation " << r.rotationId << " => [angle=" << r.angle << ", " << r.ux << ", " << r.uy << ", " << r.uz << "]";

    return os;
}