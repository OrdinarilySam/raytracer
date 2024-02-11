#include "vec3.h"

Vec3 pointAdd(Vec3 a, Vec3 b) {
  Vec3 out;
  out.x = a.x + b.x;
  out.y = a.y + b.y;
  out.z = a.z + b.z;
  return out;
}

Vec3 pointSub(Vec3 a, Vec3 b) {
  Vec3 out;
  out.x = a.x - b.x;
  out.y = a.y - b.y;
  out.z = a.z - b.z;
  return out;
}

Vec3 pointMult(Vec3 a, Vec3 b) {
  Vec3 out;
  out.x = a.x * b.x;
  out.y = a.y * b.y;
  out.z = a.z * b.z;
  return out;
}

Vec3 pointDiv(Vec3 a, Vec3 b) {
  Vec3 out;
  out.x = a.x / b.x;
  out.y = a.y / b.y;
  out.z = a.z / b.z;
  return out;
}

Vec3 scale(Vec3 a, float s) {
  Vec3 out;
  out.x = a.x * s;
  out.y = a.y * s;
  out.z = a.z * s;
  return out;
}

float length(Vec3 *a) { return sqrt(a->x * a->x + a->y * a->y + a->z * a->z); }

void normalize(Vec3 *a) {
  float l = length(a);
  a->x /= l;
  a->y /= l;
  a->z /= l;
}

float dot(Vec3 *a, Vec3 *b) { return a->x * b->x + a->y * b->y + a->z * b->z; }

Vec3 cross(Vec3 a, Vec3 b) {
  Vec3 out;
  out.x = a.y * b.z - a.z * b.y;
  out.y = a.z * b.x - a.x * b.z;
  out.z = a.x * b.y - a.y * b.x;
  return out;
}

Vec3 newVec3(float a, float b, float c) {
  // Vec3 out = {a, b, c};
  // return out;
  return (Vec3){a, b, c};
}