#include "vector3.h"
#include "math.h"



// Default constructor
using namespace std;
Vector3::Vector3() : x(0.0f), y(0.0f), z(0.0f) {}

// Parameterized constructor
Vector3::Vector3(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {}


        
Vector3::Vector3(unsigned short _x) : x((double)(_x >> 11) * 8), y((double)((_x >> 5) & 0b0000000000111111) * 4), z((double)(_x & 0b0000000000011111) * 8) {}

// Addition
Vector3 Vector3::add(const Vector3& v) const {
    return Vector3(x + v.x, y + v.y, z + v.z);
}

// Subtraction
Vector3 Vector3::sub(const Vector3& v) const {
    return Vector3(x - v.x, y - v.y, z - v.z);
}

// Multiplication by a scalar
Vector3 Vector3::mul(double scalar) const {
    return Vector3(x * scalar, y * scalar, z * scalar);
}
double Vector3::dot(const Vector3& v) const {
    return x * v.x+  y * v.y    +z * v.z;
}
// Normalization
Vector3 Vector3::normalize() const {
    double invLen = q_rsqrt(x*x+y*y+z*z);
    return Vector3(x * invLen, y * invLen, z * invLen);
}
unsigned int Vector3::toColor() const {

  //  00000000 00000000 00000000 11111000 [blue]
  //  00000000 00000000 11111100 00000000 [green]
  //  00000000 11111000 00000000 00000000 [red]

    return ((((unsigned int)x)&255)<<16)   |((((unsigned int)y)&255)<<8)|     ((unsigned int)z)&255;
}
unsigned short Vector3::toShortColor() const {
    // take a compressed color and convert it to a non compress vector3 color 
    unsigned int rgb =(((unsigned int)x)<<16)|(((unsigned int)y)<<8)|(unsigned int)z;
    return (unsigned short)(((rgb&0xF80000)>>8)|((rgb&0x00FC00)>>5)|((rgb&0x0000F8)>>3));
}


double Vector3::squareLength() const {
    return x * x + y * y + z * z;
}

// v1*(1-t)+v2*t
Vector3 gradient(Vector3 v1 ,Vector3 v2,double t){
    return v1.mul(1-t).add(v2.mul(t));
}
