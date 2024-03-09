#ifndef VEC3_H
#define VEC3_H

#include <math.h>
#include <stdlib.h>

typedef struct {
  union {
    struct {
      float x, y, z;
    };
    struct {
      float r, g, b;
    };
    struct {
      float u, v, w;
    };
    struct {
      int v1, v2, v3;
    };
  };
} Vec3;

float dot(Vec3 *a, Vec3 *b);
float length(Vec3 *a);

Vec3 pointAdd(Vec3 a, Vec3 b);
Vec3 pointSub(Vec3 a, Vec3 b);
Vec3 pointMult(Vec3, Vec3 b);
Vec3 pointDiv(Vec3 a, Vec3 b);

Vec3 scale(Vec3 a, float b);

void normalize(Vec3 *a);
Vec3 cross(Vec3 a, Vec3 b);

Vec3 pointPower(Vec3 a, float b);

#endif  // VEC3_H