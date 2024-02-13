#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <stdbool.h>
#include <stdio.h>

#include "vec3.h"

typedef struct {
  Vec3 center;
  Vec3 radii;
  int material;
} Ellipsoid;

typedef struct {
  Vec3 diffuseColor;
  Vec3 specularColor;
  float ka, kd, ks, n;
} Material;

typedef struct {
  union {
    Vec3 position;
    Vec3 direction;
  };
  float intensity;
  Vec3 attenuation;
  bool type;
} Light;

typedef struct {
  int width;
  int height;
} Image;

typedef struct {
  Vec3 origin;
  Vec3 direction;
} Ray;

typedef struct {
  Vec3 color;
  float minA, maxA;
  float minDist, maxDist;
  bool enabled;
} DepthCue;

typedef struct {
  Vec3 eye;
  Vec3 viewdir;
  Vec3 updir;
  Vec3 bkgcolor;
  Image imgsize;
  Ellipsoid* ellipsoids;
  Material* materials;
  Light* lights;
  FILE* output;
  DepthCue depthcue;
  union {
    float hfov;
    float frustum;
  };
  int numEllipsoids, numMaterials, numLights;
  bool parallel;
} Scene;

#endif  // TYPEDEFS_H