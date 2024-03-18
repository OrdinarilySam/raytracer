#ifndef MAIN_H
#define MAIN_H

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "readscene.h"
#include "tracerays.h"
#include "typedefs.h"
#include "vec3.h"

void printImage(Scene* scene, char* filename);

#endif  // MAIN_H