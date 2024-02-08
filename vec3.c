#include "vec3.h"

Vec3 pointAdd(Vec3 *a, Vec3 *b) {
  Vec3 temp = *a;
  temp.x += b->x;
  temp.y += b->y;
  temp.z += b->z;
  return temp;
}

Vec3 pointSub(Vec3 *a, Vec3 *b) {
  Vec3 temp = *a;
  temp.x -= b->x;
  temp.y -= b->y;
  temp.z -= b->z;
  return temp;
}

Vec3 pointMult(Vec3 *a, Vec3 *b) {
  Vec3 temp = *a;
  temp.x *= b->x;
  temp.y *= b->y;
  temp.z *= b->z;
  return temp;
}

Vec3 pointDiv(Vec3 *a, Vec3 *b) {
  Vec3 temp = *a;
  temp.x /= b->x;
  temp.y /= b->y;
  temp.z /= b->z;
  return temp;
}

Vec3 scale(Vec3 *a, float s) {
  Vec3 temp = *a;
  temp.x *= s;
  temp.y *= s;
  temp.z *= s;
  return temp;
}

float length(Vec3 *a) { return sqrt(a->x * a->x + a->y * a->y + a->z * a->z); }

void normalize(Vec3 *a) {
  float l = length(a);
  a->x /= l;
  a->y /= l;
  a->z /= l;
}

float dot(Vec3 *a, Vec3 *b) { return a->x * b->x + a->y * b->y + a->z * b->z; }

Vec3 cross(Vec3 *a, Vec3 *b) {
  Vec3 temp = *a;
  temp.x = a->y * b->z - a->z * b->y;
  temp.y = a->z * b->x - a->x * b->z;
  temp.z = a->x * b->y - a->y * b->x;
  return temp;
}