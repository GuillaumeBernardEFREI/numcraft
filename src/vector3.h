
#ifndef VECTOR3_H
#define VECTOR3_H


struct Vector3 {
    double x, y, z;

    // Constructors
    Vector3();
    Vector3(double x, double y, double z);
    Vector3(unsigned short _x);
    // Member functions
    Vector3 add(const Vector3& v) const;
    Vector3 sub(const Vector3& v) const;
    Vector3 mul(double scalar) const;
    double dot(const Vector3& v) const;
    Vector3 normalize() const;
    double squareLength() const;
    unsigned int toColor() const;
    unsigned short toShortColor() const;
    
};
Vector3 gradient(Vector3 v1, Vector3 v2, double t);
#endif // VECTOR3_H