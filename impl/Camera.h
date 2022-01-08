#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "Vec3.h"
#include "Matrix4.h"
#include <string>

using namespace std;

class Camera
{

public:
    int cameraId;
    int projectionType; // 1 for perspective, 0 for orthographic
    Vec3 pos;
    Vec3 gaze;
    Vec3 u;
    Vec3 v;
    Vec3 w;
    double left, right, bottom, top;
    double near;
    double far;
    int horRes;
    int verRes;
    string outputFileName;

    Matrix4 matrix;
    bool initializedMatrix = false;

    Camera();

    Camera(int cameraId,
           int projectionType,
           Vec3 pos, Vec3 gaze,
           Vec3 u, Vec3 v, Vec3 w,
           double left, double right, double bottom, double top,
           double near, double far,
           int horRes, int verRes,
           string outputFileName);

    Camera(const Camera &other);

    Matrix4 getMatrix();
    Matrix4 computeCameraMatrix();
    Matrix4 computeCVVMatrix();

    friend std::ostream &operator<<(std::ostream &os, const Camera &c);
};

#endif