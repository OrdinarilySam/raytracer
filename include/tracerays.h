#ifndef TRACERAYS_H
#define TRACERAYS_H

#include <float.h>
#include <math.h>
#include <stdbool.h>

#include "shaderay.h"
#include "typedefs.h"
#include "vec3.h"

void traceRays(Scene *scene);
void traceRay(Scene *scene, Ray *ray);

#endif  // TRACERAYS_H