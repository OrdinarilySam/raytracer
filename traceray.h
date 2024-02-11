#ifndef TRACERAY_H
#define TRACERAY_H

#include "typedefs.h"
#include "vec3.h"

typedef struct {
  Vec3 origin;
  Vec3 direction;
} Ray;

void traceray(Scene *scene, int i, int j);

#endif  // TRACERAY_H