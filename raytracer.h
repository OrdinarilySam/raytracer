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

Vec3 traceRay(RayType ray);
Vec3 shadeRay(EllipsoidType closestEllipsoid);
int cleanExit(int value);