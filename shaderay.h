#ifndef SHADERAY_H
#define SHADERAY_H

#include "typedefs.h"
#include "vec3.h"

void shadeRay(Scene *scene, Ray *ray, Ellipsoid *ellipsoid, float t);

#endif  // SHADERAY_H