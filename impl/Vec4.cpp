
#include "Vec4.h"
#include <iomanip>

using namespace std;

Vec4::Vec4()
{
    this->x = 0.0;
    this->y = 0.0;
    this->z = 0.0;
    this->t = 0.0;
    this->colorId = -1;
}

Vec4::Vec4(double x, double y, double z, double t, int colorId)
{
    this->x = x;
    this->y = y;
    this->z = z;
    this->t = t;
    this->colorId = colorId;
}
Vec4::Vec4(const Vec4 &other)
{
    this->x = other.x;
    this->y = other.y;
    this->z = other.z;
    this->t = other.t;
    this->colorId = other.colorId;
}

Vec4 Vec4::convertFromVec3(const Vec3 &other)
{
    Vec4 res;
    res.x = other.x;
    res.y = other.y;
    res.z = other.z;
    res.t = 1;
    res.colorId = other.colorId;
    return res;
}

void Vec4::applyPerspectiveDivision(){
    x = x/t;
    y = y/t;
    z = z/t;
    t = 1;
}

double Vec4::getElementAt(int index)
{
    switch (index)
    {
    case 0:
        return this->x;

    case 1:
        return this->y;

    case 2:
        return this->z;

    case 3:
        return this->t;

    default:
        return this->t;
    }
}

ostream& operator<<(ostream& os, const Vec4& v) {
    
    os << fixed << setprecision(6) << "[" << v.x << ", " << v.y << ", " << v.z << ", " << v.t << "]";

    return os;
}