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
  SPHERE
} Keyword;

int readscene(Scene* scene, char* filename);

#endif  // READSCENE_H