#ifndef SHADERAY_H
#define SHADERAY_H

#include <math.h>
#include <stdbool.h>
#include <time.h>

#include "typedefs.h"
#include "vec3.h"

float raySphereIntersection(Ray *ray, Ellipsoid *ellipsoid);
float rayTriangleIntersection(Scene *scene, Ray *ray, Triangle *face);
// todo implement and utilize this function
Vec3 calculateBarycentric(Ray *ray, Triangle *face);

void shadeRay(Scene *scene, Ray *ray, Ellipsoid *ellipsoid, float t);
// todo implement these functions
void shadeTriangle(Scene *scene, Ray *ray, Triangle *face, float t);
void shadeSphere(Scene *scene, Ray *ray, Ellipsoid *ellipsoid, float t);

Vec3 ellipsoidNormal(Ellipsoid *ellipsoid, Vec3 point);
float shadowCheck(Scene *scene, Ray *ray, Ellipsoid *ellipsoid);

float getAttenuation(Light *light, float distance);
float getDepthCueing(DepthCue *depthcue, float distance);

Vec3 generateOrthogonal(Vec3 *v);
Vec3 rotateAroundAxis(Vec3 *v, Vec3 *axis, float angle);
void jitterShadowRay(Ray *shadowRay, Vec3 *orthogonal, Vec3 *ligthDir,
                     Vec3 *pointHit, float maxScalingFactor);

#endif  // SHADERAY_H