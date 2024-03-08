#ifndef READSCENE_H
#define READSCENE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "typedefs.h"
#include "vec3.h"

typedef enum {
  IMSIZE,
  EYE,
  VIEWDIR,
  UPDIR,
  HFOV,
  BKG_COLOR,
  ELLIPSE,
  MTL_COLOR,
  LIGHT,
  PARALLEL,
  SPHERE,
  ATT_LIGHT,
  DEPTH_CUEING,
  SOFT_SHADOWS,
  VERTEX,
  FACE,
  VERTEX_NORMAL,
  VERTEX_TEXTURE,
  TEXTURE,
  UNKNOWN
} Keyword;

Keyword getKeyword(char* keyword);
void readScene(Scene* scene, char* filename);
void initializeScene(Scene* scene);
void validateScene(Scene* scene);
void freeAll(Scene* scene);
FILE* createOutputFile(char* filename, int imgWidth, int imgHeight);
void printScene(Scene* scene);

#endif  // READSCENE_H