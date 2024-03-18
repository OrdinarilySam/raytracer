#ifndef TRACERAYS_H
#define TRACERAYS_H

#include <float.h>
#include <math.h>
#include <stdbool.h>

#include "shaderay.h"
#include "typedefs.h"
#include "vec3.h"

void traceRays(Scene *scene);
Vec3 traceRay(Scene *scene, Ray *ray);

void printProgressBar(int current, int total);

#endif  // TRACERAYS_H