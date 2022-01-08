#include "Camera.h"
#include "Helpers.h"
#include <string>
#include <iostream>
#include <iomanip>

using namespace std;

Camera::Camera() {}

Camera::Camera(int cameraId,
               int projectionType,
               Vec3 pos, Vec3 gaze,
               Vec3 u, Vec3 v, Vec3 w,
               double left, double right, double bottom, double top,
               double near, double far,
               int horRes, int verRes,
               string outputFileName)
{

    this->cameraId = cameraId;
    this->projectionType = projectionType;
    this->pos = pos;
    this->gaze = gaze;
    this->u = u;
    this->v = v;
    this->w = w;
    this->left = left;
    this->right = right;
    this->bottom = bottom;
    this->top = top;
    this->near = near;
    this->far = far;
    this->horRes = horRes;
    this->verRes = verRes;
    this->outputFileName = outputFileName;
}

Camera::Camera(const Camera &other)
{
    this->cameraId = other.cameraId;
    this->projectionType = other.projectionType;
    this->pos = other.pos;
    this->gaze = other.gaze;
    this->u = other.u;
    this->v = other.v;
    this->w = other.w;
    this->left = other.left;
    this->right = other.right;
    this->bottom = other.bottom;
    this->top = other.top;
    this->near = other.near;
    this->far = other.far;
    this->horRes = other.horRes;
    this->verRes = other.verRes;
    this->outputFileName = other.outputFileName;
}

Matrix4 Camera::computeCameraMatrix(){
    Matrix4 translation = getIdentityMatrix();
    translation.val[0][3]=pos.x;
    translation.val[1][3]=pos.y;
    translation.val[2][3]=pos.z;

    double basis[4][4] = {{u.x,u.y,u.z,0},{v.x,v.y,v.z,0},{w.x,w.y,w.z,0},{0,0,0,1}};
    Matrix4 basisMatrix(basis);

    return multiplyMatrixWithMatrix(basis, translation);
}

Matrix4 Camera::computeCVVMatrix(){
    Matrix4 M = getIdentityMatrix();

    double oVal[4][4] = {{2/(right-left),0,0,0},{0,2/(top-bottom),0,0},{0,0,-2/(far-near),0},{0,0,0,1}};
    oVal[0][3] = -(right+left)/(right-left);
    oVal[1][3] = -(top+bottom)/(top-bottom);
    oVal[2][3] = -(far+near)/(far-near);
    Matrix4 ortho(oVal);

    //if perspective
    if(projectionType == 0){
        double pVal[4][4] = {{near,0,0,0},{0,near,0,0},{0,0,far+near,far*near},{0,0,-1,0}};
        Matrix4 pers(pVal);
        M = multiplyMatrixWithMatrix(pers,M);
    }

    M = multiplyMatrixWithMatrix(ortho, M);
    return M;
}

Matrix4 Camera::getMatrix(){
    if(initializedMatrix){
        return matrix;
    }

    Matrix4 camTransform = computeCameraMatrix();
    Matrix4 cvvTransform = computeCVVMatrix();

    matrix = multiplyMatrixWithMatrix(camTransform, cvvTransform);

    initializedMatrix = true;
    return matrix;
}

ostream &operator<<(ostream &os, const Camera &c)
{
    const char *camType = c.projectionType ? "perspective" : "orthographic";

    os << fixed << setprecision(6) << "Camera " << c.cameraId << " (" << camType << ") => pos: " << c.pos << " gaze: " << c.gaze << endl
       << "\tu: " << c.u << " v: " << c.v << " w: " << c.w << endl
       << fixed << setprecision(3) << "\tleft: " << c.left << " right: " << c.right << " bottom: " << c.bottom << " top: " << c.top << endl
       << "\tnear: " << c.near << " far: " << c.far << " resolutions: " << c.horRes << "x" << c.verRes << " fileName: " << c.outputFileName;

    return os;
}