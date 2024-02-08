#include "vec3.h"

Vec3 *add(Vec3 *a, Vec3 *b)
{
    a->x += b->x;
    a->y += b->y;
    a->z += b->z;
    return a;
}

Vec3 *sub(Vec3 *a, Vec3 *b)
{
    a->x -= b->x;
    a->y -= b->y;
    a->z -= b->z;
    return a;
}

Vec3 *mul(Vec3 *a, Vec3 *b)
{
    a->x *= b->x;
    a->y *= b->y;
    a->z *= b->z;
    return a;
}

Vec3 *div(Vec3 *a, Vec3 *b)
{
    a->x /= b->x;
    a->y /= b->y;
    a->z /= b->z;
    return a;
}

Vec3 *scale(Vec3 *a, float s)
{
    a->x *= s;
    a->y *= s;
    a->z *= s;
    return a;
}

float length(Vec3 *a)
{
    return sqrt(a->x * a->x + a->y * a->y + a->z * a->z);
}

Vec3 *normalize(Vec3 *a)
{
    float l = length(a);
    a->x /= l;
    a->y /= l;
    a->z /= l;
    return a;
}

float dot(Vec3 *a, Vec3 *b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

Vec3 *cross(Vec3 *a, Vec3 *b)
{
    Vec3 temp = *a;
    a->x = temp.y * b->z - temp.z * b->y;
    a->y = temp.z * b->x - temp.x * b->z;
    a->z = temp.x * b->y - temp.y * b->x;
    return a;
}