#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <stdbool.h>
#include <stdio.h>

#include "vec3.h"

typedef struct {
  Vec3 center;
  Vec3 radii;
  int material;
  int texture;
  bool usingTexture;
} Ellipsoid;

typedef struct {
  union {
    struct {
      int v1, v2, v3;
    };
    struct {
      int n1, n2, n3;
    };
    struct {
      int t1, t2, t3;
    };
    struct {
      int x1, x2, x3;
    };
    struct {
      int i, j, k;
    };
  };
} Indices;

/*
Types:
0 - vertices only
1 - vertices and normals
2 - vertices and textures
3 - vertices, normals, and textures
*/
typedef enum {
  VERTICES_ONLY,
  VERTICES_NORMALS,
  VERTICES_TEXTURES,
  VERTICES_NORMALS_TEXTURES
} TriangleType;

typedef struct {
  float u, v;
} Texel;

typedef struct {
  Indices vertices;
  Indices normals;
  Indices textures;
  TriangleType type;
  int material;
  int texture;
} Triangle;

typedef struct {
  Vec3 diffuseColor;
  Vec3 specularColor;
  float ka, kd, ks, n, alpha, eta;
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
  int depth;
} Ray;

typedef struct {
  int height, width;
  Vec3** pixels;
} Texture;

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
  Triangle* faces;
  Vec3* vertices;
  Vec3* normals;
  Texel* vertexTextures;
  Texture* textures;
  // FILE* output;
  Vec3** pixels;
  DepthCue depthcue;
  union {
    float hfov;
    float frustum;
  };
  int numEllipsoids, numMaterials, numLights, numFaces, numVertices, numNormals, numTextures, numVertexTextures;
  bool parallel;
  bool softShadows;
} Scene;

#endif  // TYPEDEFS_H