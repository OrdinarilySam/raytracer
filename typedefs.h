#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include "vec3.h"

typedef struct {
  Vec3 center;
  Vec3 radii;
} Ellipsoid;

typedef struct {
  Vec3 diffuseColor;
  Vec3 specularColor;
  float ka, kd, ks, n;
} Material;

typedef struct {
  Vec3 position;
  bool type;
  float intensity;
} Light;

typedef struct {
  int width;
  int height;
} Image;

typedef struct {
  Vec3 position;
  Vec3 direction;

} Ray;

typedef struct {
  Vec3 eye;
  Vec3 viewdir;
  Vec3 updir;
  Vec3 bkgcolor;
  Image imgsize;
  union {
    float hfov;
    float frustum;
  };
  Ellipsoid* ellipsoids;
  Material* materials;
  Light* lights;
  int numEllipsoids, numMaterials, numLights;
  FILE* output;
} Scene;

#endif  // TYPEDEFS_H