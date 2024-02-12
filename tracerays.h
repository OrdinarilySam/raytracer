#ifndef TRACERAYS_H
#define TRACERAYS_H

#include <float.h>
#include <math.h>
#include <stdbool.h>

#include "shaderay.h"
#include "typedefs.h"
#include "vec3.h"

typedef struct {
  Vec3 origin;
  Vec3 direction;
} Ray;

void traceRays(Scene *scene);
void traceRay(Ray *ray, Scene *scene);
float rayIntersection(Ray *ray, Ellipsoid *ellipsoid);

#endif  // TRACERAYS_H