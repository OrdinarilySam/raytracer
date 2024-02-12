#ifndef SHADERAY_H
#define SHADERAY_H

#include <math.h>
#include <stdbool.h>

#include "typedefs.h"
#include "vec3.h"

float rayIntersection(Ray *ray, Ellipsoid *ellipsoid);

void shadeRay(Scene *scene, Ray *ray, Ellipsoid *ellipsoid, float t);

Vec3 ellipsoidNormal(Ellipsoid *ellipsoid, Vec3 point, float t);
float shadowCheck(Scene *scene, Ray *ray, Ellipsoid *ellipsoid);

#endif  // SHADERAY_H