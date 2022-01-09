#include "Color.h"
#include <iostream>
#include <iomanip>

using namespace std;

Color::Color() {}

Color::Color(double r, double g, double b)
{
    this->r = r;
    this->g = g;
    this->b = b;
}

Color::Color(const Color &other)
{
    this->r = other.r;
    this->g = other.g;
    this->b = other.b;
}

void Color::addColor(const Color& other){
    r+=other.r;
    g+=other.g;
    b+=other.b;
}

Color Color::operator-(const Color& o){
    return Color(r-o.r,g-o.g,b-o.b);
}

Color Color::operator*(double k){
    return Color(r*k,g*k,b*k);
}

Color Color::operator+(const Color& o){
    return Color(r+o.r,g+o.g,b+o.b);
}

ostream& operator<<(ostream& os, const Color& c)
{
    os << fixed << setprecision(0) << "rgb(" << c.r << ", " << c.g << ", " << c.b << ")";
    return os;
}
