#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vec3.h"

typedef struct {
  Vec3 center;
  Vec3 radius;
  int m;
} EllipsoidType;

typedef struct {
  Vec3 pos;
  Vec3 dir;
} RayType;

typedef struct {
  Vec3 diffuseColor;
  Vec3 spectralColor;
  float ka, kd, ks;
  float n;
} MaterialType;

typedef struct {
  union {
    Vec3 pos;
    Vec3 dir;
  };
  short type;
  float intensity;

} LightType;

Vec3 traceRay(RayType ray);
Vec3 shadeRay(EllipsoidType closestEllipsoid, Vec3 pointHit, RayType ray);
int cleanExit(int value);