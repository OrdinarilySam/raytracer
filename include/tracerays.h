#ifndef TRACERAYS_H
#define TRACERAYS_H

#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

#include "shaderay.h"
#include "typedefs.h"
#include "vec3.h"
#include "definitions.h"

typedef struct {
  Vec3 ul;
  Vec3 vChange;
  Vec3 hChange;
  Vec3 w;
} ThreadData;

typedef struct {
  Scene *scene;
  ThreadData *threadData;
  int start;
  int end;
} ThreadArgs;

void traceRays(Scene *scene);
Vec3 traceRay(Scene *scene, Ray *ray);

void* traceRaysThread(void* arg);

float raySphereIntersection(Ray *ray, Ellipsoid *ellipsoid);
float rayTriangleIntersection(Scene *scene, Ray *ray, Triangle *face);

void printProgressBar(int current, int total);

#endif  // TRACERAYS_H