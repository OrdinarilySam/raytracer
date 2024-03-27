#include "readscene.h"

void readScene(Scene* scene, char* filename) {
  FILE* file = fopen(filename, "r");
  if (file == NULL) {
    free(scene);
    printf("Error: file %s not found\n", filename);
    exit(1);
  }

  initializeScene(scene);

  char buffer[100];
  while (fscanf(file, "%s", buffer) != EOF) {
    if (strcmp(buffer, "sphere") == 0) {
      scene->numEllipsoids++;
    } else if (strcmp(buffer, "ellipse") == 0) {
      scene->numEllipsoids++;
    } else if (strcmp(buffer, "mtlcolor") == 0) {
      scene->numMaterials++;
    } else if (strcmp(buffer, "light") == 0) {
      scene->numLights++;
    } else if (strcmp(buffer, "attlight") == 0) {
      scene->numLights++;
    } else if (strcmp(buffer, "f") == 0) {
      scene->numFaces++;
    } else if (strcmp(buffer, "v") == 0) {
      scene->numVertices++;
    } else if (strcmp(buffer, "vn") == 0) {
      scene->numNormals++;
    } else if (strcmp(buffer, "texture") == 0) {
      scene->numTextures++;
    } else if (strcmp(buffer, "vt") == 0) {
      scene->numVertexTextures++;
    }
  }
  fclose(file);

  scene->ellipsoids =
      (Ellipsoid*)malloc(scene->numEllipsoids * sizeof(Ellipsoid));
  scene->materials = (Material*)malloc(scene->numMaterials * sizeof(Material));
  scene->lights = (Light*)malloc(scene->numLights * sizeof(Light));
  scene->faces = (Triangle*)malloc(scene->numFaces * sizeof(Triangle));
  scene->vertices = (Vec3*)malloc(scene->numVertices * sizeof(Vec3));
  scene->normals = (Vec3*)malloc(scene->numNormals * sizeof(Vec3));
  scene->vertexTextures =
      (Texel*)malloc(scene->numVertexTextures * sizeof(Texel));
  scene->textures = (Texture*)malloc(scene->numFaces * sizeof(Texture));

  if (scene->ellipsoids == NULL || scene->materials == NULL ||
      scene->lights == NULL || scene->faces == NULL ||
      scene->vertices == NULL || scene->normals == NULL ||
      scene->textures == NULL || scene->vertexTextures == NULL) {
    freeAll(scene);
    printf("Error: malloc failed\n");
    exit(1);
  }

  for (int i = 0; i < scene->numTextures; i++) {
    scene->textures[i].pixels = NULL;
  }

  file = fopen(filename, "r");
  if (file == NULL) {
    freeAll(scene);
    printf("Error: file %s not found\n", filename);
    exit(1);
  }

  printf("Reading scene file %s\n", filename);

  int ellipsoidIndex = 0;
  int materialIndex = 0;
  int lightIndex = 0;
  int vertexIndex = 0;
  int faceIndex = 0;
  int normalIndex = 0;
  int vertexTextureIndex = 0;
  int textureIndex = 0;

  int maxNormalIndex = -1;
  int maxTextureIndex = -1;
  int maxVertexIndex = -1;

  bool foundEye = false;
  bool onTexture = false;

  while (fscanf(file, "%s", buffer) != EOF) {
    switch (getKeyword(buffer)) {
      case IMSIZE:
        fscanf(file, "%d %d", &scene->imgsize.width, &scene->imgsize.height);
        break;

      case EYE:
        foundEye = true;
        fscanf(file, "%f %f %f", &scene->eye.x, &scene->eye.y, &scene->eye.z);
        break;

      case VIEWDIR:
        fscanf(file, "%f %f %f", &scene->viewdir.x, &scene->viewdir.y,
               &scene->viewdir.z);
        break;

      case UPDIR:
        fscanf(file, "%f %f %f", &scene->updir.x, &scene->updir.y,
               &scene->updir.z);
        break;

      case HFOV:
        scene->parallel = false;
        fscanf(file, "%f", &scene->hfov);
        break;

      case BKG_COLOR:
        fscanf(file, "%f %f %f %f", &scene->bkgcolor.x, &scene->bkgcolor.y,
               &scene->bkgcolor.z, &scene->backgroundEta);
        break;

      case PARALLEL:
        scene->parallel = true;
        fscanf(file, "%f", &scene->frustum);
        break;

      case SPHERE:
        if (materialIndex == 0) {
          printf("Error: must define material before sphere\n");
          freeAll(scene);
          fclose(file);
          exit(1);
        }

        float r;
        fscanf(file, "%f %f %f %f", &scene->ellipsoids[ellipsoidIndex].center.x,
               &scene->ellipsoids[ellipsoidIndex].center.y,
               &scene->ellipsoids[ellipsoidIndex].center.z, &r);

        if (r <= 0) {
          printf("Error: radius must be greater than 0\n");
          freeAll(scene);
          fclose(file);
          exit(1);
        }

        scene->ellipsoids[ellipsoidIndex].radii = (Vec3){r, r, r};
        if (onTexture) {
          scene->ellipsoids[ellipsoidIndex].texture = textureIndex - 1;
          scene->ellipsoids[ellipsoidIndex].usingTexture = true;
        } else {
          scene->ellipsoids[ellipsoidIndex].usingTexture = false;
        }
        scene->ellipsoids[ellipsoidIndex].material = materialIndex - 1;
        ellipsoidIndex++;
        break;

      case ELLIPSE:
        if (materialIndex == 0) {
          printf("Error: must define material before ellipsoid\n");
          freeAll(scene);
          fclose(file);
          exit(1);
        }

        fscanf(file, "%f %f %f %f %f %f",
               &scene->ellipsoids[ellipsoidIndex].center.x,
               &scene->ellipsoids[ellipsoidIndex].center.y,
               &scene->ellipsoids[ellipsoidIndex].center.z,
               &scene->ellipsoids[ellipsoidIndex].radii.x,
               &scene->ellipsoids[ellipsoidIndex].radii.y,
               &scene->ellipsoids[ellipsoidIndex].radii.z);

        if (scene->ellipsoids[ellipsoidIndex].radii.x <= 0 ||
            scene->ellipsoids[ellipsoidIndex].radii.y <= 0 ||
            scene->ellipsoids[ellipsoidIndex].radii.z <= 0) {
          printf("Error: radii must be greater than 0\n");
          freeAll(scene);
          fclose(file);
          exit(1);
        }

        if (onTexture) {
          scene->ellipsoids[ellipsoidIndex].texture = textureIndex - 1;
          scene->ellipsoids[ellipsoidIndex].usingTexture = true;
        } else {
          scene->ellipsoids[ellipsoidIndex].usingTexture = false;
        }
        scene->ellipsoids[ellipsoidIndex].material = materialIndex - 1;
        ellipsoidIndex++;
        break;

      case MTL_COLOR: {
        fscanf(file, "%f %f %f",
               &scene->materials[materialIndex].diffuseColor.x,
               &scene->materials[materialIndex].diffuseColor.y,
               &scene->materials[materialIndex].diffuseColor.z);
        fscanf(file, "%f %f %f",
               &scene->materials[materialIndex].specularColor.x,
               &scene->materials[materialIndex].specularColor.y,
               &scene->materials[materialIndex].specularColor.z);
        fscanf(file, "%f %f %f %f", &scene->materials[materialIndex].ka,
               &scene->materials[materialIndex].kd,
               &scene->materials[materialIndex].ks,
               &scene->materials[materialIndex].n);
        fscanf(file, "%f %f", &scene->materials[materialIndex].alpha,
               &scene->materials[materialIndex].eta);

        if (scene->materials[materialIndex].ka < 0 ||
            scene->materials[materialIndex].ka > 1 ||
            scene->materials[materialIndex].kd < 0 ||
            scene->materials[materialIndex].kd > 1 ||
            scene->materials[materialIndex].ks < 0 ||
            scene->materials[materialIndex].ks > 1) {
          printf("Error: ka, kd, ks must be between 0 and 1\n");
          freeAll(scene);
          fclose(file);
          exit(1);
        }
        if (scene->materials[materialIndex].n < 0) {
          printf("Error: n must be greater than 0\n");
          freeAll(scene);
          fclose(file);
          exit(1);
        }
        if (scene->materials[materialIndex].eta < 1) {
          printf("Error: eta must be greater than or equal to 1\n");
          freeAll(scene);
          fclose(file);
          exit(1);
        }

        if (scene->materials[materialIndex].alpha < 0 ||
            scene->materials[materialIndex].alpha > 1) {
          printf("Error: alpha must be between 0 and 1\n");
          freeAll(scene);
          fclose(file);
          exit(1);
        }

        materialIndex++;
        break;
      }

      case LIGHT: {
        int temp;
        fscanf(file, "%f %f %f", &scene->lights[lightIndex].position.x,
               &scene->lights[lightIndex].position.y,
               &scene->lights[lightIndex].position.z);
        fscanf(file, "%d %f", &temp, &scene->lights[lightIndex].intensity);
        scene->lights[lightIndex].type = (bool)temp;

        if (scene->lights[lightIndex].intensity < 0 ||
            scene->lights[lightIndex].intensity > 1) {
          printf("Error: light intensity must be between 0 and 1\n");
          freeAll(scene);
          fclose(file);
          exit(1);
        }

        scene->lights[lightIndex].attenuation = (Vec3){0, 0, 0};

        lightIndex++;
        break;
      }

      case ATT_LIGHT: {
        int temp;
        fscanf(file, "%f %f %f", &scene->lights[lightIndex].position.x,
               &scene->lights[lightIndex].position.y,
               &scene->lights[lightIndex].position.z);
        fscanf(file, "%d %f", &temp, &scene->lights[lightIndex].intensity);
        scene->lights[lightIndex].type = (bool)temp;

        if (scene->lights[lightIndex].intensity < 0 ||
            scene->lights[lightIndex].intensity > 1) {
          printf("Error: light intensity must be between 0 and 1\n");
          freeAll(scene);
          fclose(file);
          exit(1);
        }

        fscanf(file, "%f %f %f", &scene->lights[lightIndex].attenuation.x,
               &scene->lights[lightIndex].attenuation.y,
               &scene->lights[lightIndex].attenuation.z);

        if (scene->lights[lightIndex].attenuation.x < 0 ||
            scene->lights[lightIndex].attenuation.y < 0 ||
            scene->lights[lightIndex].attenuation.z < 0) {
          printf("Error: attenuation components must be greater than 0\n");
          freeAll(scene);
          fclose(file);
          exit(1);
        }
        break;
      }

      case DEPTH_CUEING:
        fscanf(file, "%f %f %f %f %f %f %f", &scene->depthcue.color.x,
               &scene->depthcue.color.y, &scene->depthcue.color.z,
               &scene->depthcue.minA, &scene->depthcue.maxA,
               &scene->depthcue.minDist, &scene->depthcue.maxDist);
        scene->depthcue.enabled = true;
        break;

      case SOFT_SHADOWS:
        scene->softShadows = true;
        break;

      case VERTEX: {
        float x, y, z;
        fscanf(file, "%f %f %f", &x, &y, &z);
        Vec3 vertex = (Vec3){x, y, z};
        scene->vertices[vertexIndex] = vertex;
        vertexIndex++;
        break;
      }

      case FACE: {
        if (!onTexture && materialIndex == 0) {
          printf("Error: must define material before face\n");
          freeAll(scene);
          fclose(file);
          exit(1);
        }
        int v1, v2, v3, t1, t2, t3, n1, n2, n3 = 0;
        char line[MAXCHAR];
        Triangle face;
        fgets(line, MAXCHAR, file);

        if (sscanf(line, "%d/%d/%d %d/%d/%d %d/%d/%d", &v1, &t1, &n1, &v2, &t2,
                   &n2, &v3, &t3, &n3) == 9) {
          face.type = VERTICES_NORMALS_TEXTURES;
          maxNormalIndex = maxNormalIndex < n1 ? n1 : maxNormalIndex;
          maxNormalIndex = maxNormalIndex < n2 ? n2 : maxNormalIndex;
          maxNormalIndex = maxNormalIndex < n3 ? n3 : maxNormalIndex;
          maxTextureIndex = maxTextureIndex < t1 ? t1 : maxTextureIndex;
          maxTextureIndex = maxTextureIndex < t2 ? t2 : maxTextureIndex;
          maxTextureIndex = maxTextureIndex < t3 ? t3 : maxTextureIndex;
        }

        else if (sscanf(line, "%d//%d %d//%d %d//%d", &v1, &n1, &v2, &n2, &v3,
                        &n3) == 6) {
          face.type = VERTICES_NORMALS;
          maxNormalIndex = maxNormalIndex < n1 ? n1 : maxNormalIndex;
          maxNormalIndex = maxNormalIndex < n2 ? n2 : maxNormalIndex;
          maxNormalIndex = maxNormalIndex < n3 ? n3 : maxNormalIndex;
        }

        else if (sscanf(line, "%d/%d %d/%d %d/%d", &v1, &t1, &v2, &t2, &v3,
                        &t3) == 6) {
          face.type = VERTICES_TEXTURES;
          maxTextureIndex = maxTextureIndex < t1 ? t1 : maxTextureIndex;
          maxTextureIndex = maxTextureIndex < t2 ? t2 : maxTextureIndex;
          maxTextureIndex = maxTextureIndex < t3 ? t3 : maxTextureIndex;
        }

        else if (sscanf(line, "%d %d %d", &v1, &v2, &v3) == 3) {
          face.type = VERTICES_ONLY;
        }

        else {
          printf("Error: invalid face format\n");
          freeAll(scene);
          fclose(file);
          exit(1);
        }

        if ((face.type == VERTICES_NORMALS_TEXTURES ||
             face.type == VERTICES_TEXTURES) &&
            !onTexture) {
          printf("Error: must define texture before face\n");
          freeAll(scene);
          fclose(file);
          exit(1);
        }

        if (onTexture) {
          face.texture = textureIndex - 1;
        } 
        face.material = materialIndex - 1;

        face.vertices = (Indices){v1 - 1, v2 - 1, v3 - 1};
        face.normals = (Indices){n1 - 1, n2 - 1, n3 - 1};
        face.textures = (Indices){t1 - 1, t2 - 1, t3 - 1};

        maxVertexIndex = maxVertexIndex < v1 ? v1 : maxVertexIndex;
        maxVertexIndex = maxVertexIndex < v2 ? v2 : maxVertexIndex;
        maxVertexIndex = maxVertexIndex < v3 ? v3 : maxVertexIndex;

        if (v1 < 1 || v2 < 1 || v3 < 1) {
          printf("Error: vertex index out of bounds\n");
          freeAll(scene);
          fclose(file);
          exit(1);
        }

        if (face.type == VERTICES_NORMALS ||
            face.type == VERTICES_NORMALS_TEXTURES) {
          if (n1 < 1 || n2 < 1 || n3 < 1) {
            printf("Error: normal index out of bounds\n");
            freeAll(scene);
            fclose(file);
            exit(1);
          }
        }

        if (face.type == VERTICES_TEXTURES ||
            face.type == VERTICES_NORMALS_TEXTURES) {
          if (t1 < 1 || t2 < 1 || t3 < 1) {
            printf("Error: texture index out of bounds\n");
            freeAll(scene);
            fclose(file);
            exit(1);
          }
        }

        scene->faces[faceIndex] = face;
        faceIndex++;
        break;
      }

      case VERTEX_NORMAL: {
        float x, y, z;
        fscanf(file, "%f %f %f", &x, &y, &z);
        Vec3 normal = (Vec3){x, y, z};
        scene->normals[normalIndex] = normal;
        normalIndex++;
        break;
      }

      case TEXTURE: {
        onTexture = true;
        char textureName[MAXCHAR];
        fscanf(file, "%s", textureName);
        Texture texture;
        parsePPM(&texture, textureName, filename);
        scene->textures[textureIndex] = texture;
        textureIndex++;
        break;
      }

      case VERTEX_TEXTURE: {
        Texel texel;
        fscanf(file, "%f %f", &texel.u, &texel.v);
        scene->vertexTextures[vertexTextureIndex] = texel;
        vertexTextureIndex++;
        break;
      }

      case UNKNOWN:
        break;
    }
  }

  fclose(file);
  // printScene(scene);

  if (maxNormalIndex > scene->numNormals) {
    printf("maxNormalIndex: %d\n", maxNormalIndex);
    printf("scene->numNormals: %d\n", scene->numNormals);
    printf("Error: normal index out of bounds\n");
    freeAll(scene);
    exit(1);
  }

  if (maxTextureIndex > scene->numVertexTextures) {
    printf("maxTextureIndex: %d\n", maxTextureIndex);
    printf("scene->numTextures: %d\n", scene->numTextures);
    printf("Error: texture index out of bounds\n");
    freeAll(scene);
    exit(1);
  }

  if (maxVertexIndex > scene->numVertices) {
    printf("maxVertexIndex: %d\n", maxVertexIndex);
    printf("scene->numVertices: %d\n", scene->numVertices);
    printf("Error: vertex index out of bounds\n");
    freeAll(scene);
    exit(1);
  }

  if (foundEye == false) {
    printf("Error: eye not found\n");
    freeAll(scene);
    exit(1);
  }
  validateScene(scene);
  // FILE* fp =
      // createOutputFile(filename, scene->imgsize.width, scene->imgsize.height);
  // if (fp == NULL) {
    // freeAll(scene);
    // exit(1);
  // }

  initializePixels(scene);
  // scene->output = fp;

  // ? Debugging
  // printScene(scene);
}

void initializePixels(Scene* scene) {
  Vec3** pixels = (Vec3**)malloc(scene->imgsize.height * sizeof(Vec3*));
  if (pixels == NULL) {
    printf("Error: malloc failed\n");
    freeAll(scene);
    exit(1);
  }
  for (int i = 0; i < scene->imgsize.height; i++) {
    pixels[i] = (Vec3*)malloc(scene->imgsize.width * sizeof(Vec3));
    if (pixels[i] == NULL) {
      printf("Error: malloc failed\n");
      freeAll(scene);
      exit(1);
    }
  }
  scene->pixels = pixels;

}

FILE* createOutputFile(char* filename, int imgWidth, int imgHeight) {
  char* dotPosition = strrchr(filename, '.');
  char newFileName[strlen(filename) + strlen(".ppm") + 1];

  if (dotPosition != NULL) {
    strncpy(newFileName, filename, dotPosition - filename);
    strcpy(newFileName + (dotPosition - filename), ".ppm\0");
  } else {
    strcpy(newFileName, filename);
    strcat(newFileName, ".ppm\0");
  }

  printf("Creating file %s\n", newFileName);
  FILE* outputFile = fopen(newFileName, "w");
  if (outputFile == NULL) {
    printf("Couldn't create file %s\n", newFileName);
    return NULL;
  }
  return outputFile;
}

void validateScene(Scene* scene) {
  if (scene->numMaterials == 0) {
    printf("Error: no materials defined\n");
    freeAll(scene);
    exit(1);
  }
  // if (scene->numEllipsoids == 0) {
  //   printf("Error: no ellipsoids defined\n");
  //   freeAll(scene);
  //   exit(1);
  // }
  if (scene->imgsize.width <= 1 || scene->imgsize.height <= 1) {
    printf("Error: image size must be greater than 1\n");
    freeAll(scene);
    exit(1);
  }
  if (scene->parallel == false && (scene->hfov <= 0 || scene->hfov >= 180)) {
    printf("Error: hfov must be between 0 and 180\n");
    freeAll(scene);
    exit(1);
  }
  if (scene->parallel == true && scene->frustum <= 0) {
    printf("Error: frustum must be greater than 0\n");
    freeAll(scene);
    exit(1);
  }
  if (length(&scene->viewdir) == 0) {
    printf("Error: viewdir must have length\n");
    freeAll(scene);
    exit(1);
  }
  if (length(&scene->updir) == 0) {
    printf("Error: updir must have length\n");
    freeAll(scene);
    exit(1);
  }
  if ((dot(&scene->viewdir, &scene->updir) /
       (length(&scene->viewdir) * length(&scene->updir))) > 0.001) {
    printf("Error: viewdir and updir too close to parallel\n");
    freeAll(scene);
    exit(1);
  }
  if (scene->bkgcolor.x < 0 || scene->bkgcolor.x > 1 || scene->bkgcolor.y < 0 ||
      scene->bkgcolor.y > 1 || scene->bkgcolor.z < 0 || scene->bkgcolor.z > 1) {
    printf("Error: bkgcolor components must be between 0 and 1\n");
    freeAll(scene);
    exit(1);
  }
  if (scene->backgroundEta < 1) {
    printf("Error: backgroundEta must be greater than or equal to 1\n");
    freeAll(scene);
    exit(1);
  }
  if (scene->depthcue.enabled) {
    if (scene->depthcue.minA < 0 || scene->depthcue.minA > 1 ||
        scene->depthcue.maxA < 0 || scene->depthcue.maxA > 1) {
      printf("Error: minA and maxA must be between 0 and 1\n");
      freeAll(scene);
      exit(1);
    }
    if (scene->depthcue.minDist < 0 || scene->depthcue.maxDist < 0) {
      printf("Error: minDist and maxDist must be greater than 0\n");
      freeAll(scene);
      exit(1);
    }
    if (scene->depthcue.minDist >= scene->depthcue.maxDist) {
      printf("Error: minDist must be less than maxDist\n");
      freeAll(scene);
      exit(1);
    }
    if (scene->depthcue.color.x < 0 || scene->depthcue.color.x > 1 ||
        scene->depthcue.color.y < 0 || scene->depthcue.color.y > 1 ||
        scene->depthcue.color.z < 0 || scene->depthcue.color.z > 1) {
      printf("Error: depthcue color components must be between 0 and 1\n");
      freeAll(scene);
      exit(1);
    }
  }

  normalize(&scene->updir);
  normalize(&scene->viewdir);
}

void freeAll(Scene* scene) {
  free(scene->ellipsoids);
  free(scene->materials);
  free(scene->lights);
  free(scene->faces);
  free(scene->vertices);
  free(scene->normals);
  for (int i = 0; i < scene->numTextures; i++) {
    if (scene->textures[i].pixels == NULL) {
      continue;
    }
    for (int j = 0; j < scene->textures[i].height; j++) {
      free(scene->textures[i].pixels[j]);
    }
    free(scene->textures[i].pixels);
  }
  free(scene->textures);
  if (scene->pixels != NULL) {
    for (int i = 0; i < scene->imgsize.height; i++) {
      if (scene->pixels[i] != NULL)
      free(scene->pixels[i]);
    }
    free(scene->pixels);
  }
  free(scene);
}

void initializeScene(Scene* scene) {
  scene->numEllipsoids = 0;
  scene->numMaterials = 0;
  scene->numLights = 0;
  scene->viewdir = (Vec3){0, 0, 0};
  scene->updir = (Vec3){0, 0, 0};
  scene->eye = (Vec3){0, 0, 0};
  scene->bkgcolor = (Vec3){-1, -1, -1};
  scene->parallel = false;
  scene->depthcue.enabled = false;
  scene->pixels = NULL;
}

Keyword getKeyword(char* keyword) {
  if (strcmp(keyword, "v") == 0) {
    return VERTEX;
  }
  if (strcmp(keyword, "f") == 0) {
    return FACE;
  }
  if (strcmp(keyword, "vn") == 0) {
    return VERTEX_NORMAL;
  }
  if (strcmp(keyword, "sphere") == 0) {
    return SPHERE;
  }
  if (strcmp(keyword, "ellipse") == 0) {
    return ELLIPSE;
  }
  if (strcmp(keyword, "mtlcolor") == 0) {
    return MTL_COLOR;
  }
  if (strcmp(keyword, "light") == 0) {
    return LIGHT;
  }
  if (strcmp(keyword, "imsize") == 0) {
    return IMSIZE;
  }
  if (strcmp(keyword, "eye") == 0) {
    return EYE;
  }
  if (strcmp(keyword, "viewdir") == 0) {
    return VIEWDIR;
  }
  if (strcmp(keyword, "updir") == 0) {
    return UPDIR;
  }
  if (strcmp(keyword, "hfov") == 0) {
    return HFOV;
  }
  if (strcmp(keyword, "bkgcolor") == 0) {
    return BKG_COLOR;
  }
  if (strcmp(keyword, "parallel") == 0) {
    return PARALLEL;
  }
  if (strcmp(keyword, "attlight") == 0) {
    return ATT_LIGHT;
  }
  if (strcmp(keyword, "depthcueing") == 0) {
    return DEPTH_CUEING;
  }
  if (strcmp(keyword, "softshadows") == 0) {
    return SOFT_SHADOWS;
  }
  if (strcmp(keyword, "texture") == 0) {
    return TEXTURE;
  }
  if (strcmp(keyword, "vt") == 0) {
    return VERTEX_TEXTURE;
  }
  return UNKNOWN;
}

void printScene(Scene* scene) {
  printf("------------------- Scene -------------------\n");
  printf("Image size: %d %d\n", scene->imgsize.width, scene->imgsize.height);
  printf("Eye: %f %f %f\n", scene->eye.x, scene->eye.y, scene->eye.z);
  printf("Viewdir: %f %f %f\n", scene->viewdir.x, scene->viewdir.y,
         scene->viewdir.z);
  printf("Updir: %f %f %f\n", scene->updir.x, scene->updir.y, scene->updir.z);
  if (scene->parallel) {
    printf("Frustum: %f\n", scene->frustum);
  } else {
    printf("Hfov: %f\n", scene->hfov);
  }
  printf("Background color: %f %f %f\n", scene->bkgcolor.x, scene->bkgcolor.y,
         scene->bkgcolor.z);
  printf("-------------------- Ellipsoids -------------------\n");
  printf("Number of ellipsoids: %d\n", scene->numEllipsoids);
  for (int i = 0; i < scene->numEllipsoids; i++) {
    printf("Ellipsoid %d\n", i);
    printf("Center: %f %f %f\n", scene->ellipsoids[i].center.x,
           scene->ellipsoids[i].center.y, scene->ellipsoids[i].center.z);
    printf("Radii: %f %f %f\n", scene->ellipsoids[i].radii.x,
           scene->ellipsoids[i].radii.y, scene->ellipsoids[i].radii.z);
    printf("Material: %d\n", scene->ellipsoids[i].material);
  }

  printf("-------------------- Materials -------------------\n");
  printf("Number of materials: %d\n", scene->numMaterials);
  for (int i = 0; i < scene->numMaterials; i++) {
    printf("Material %d\n", i);
    printf("Diffuse color: %f %f %f\n", scene->materials[i].diffuseColor.x,
           scene->materials[i].diffuseColor.y,
           scene->materials[i].diffuseColor.z);
    printf("Specular color: %f %f %f\n", scene->materials[i].specularColor.x,
           scene->materials[i].specularColor.y,
           scene->materials[i].specularColor.z);
    printf("ka: %f\n", scene->materials[i].ka);
    printf("kd: %f\n", scene->materials[i].kd);
    printf("ks: %f\n", scene->materials[i].ks);
    printf("n: %f\n", scene->materials[i].n);
  }

  printf("-------------------- Lights -------------------\n");
  printf("Number of lights: %d\n", scene->numLights);
  for (int i = 0; i < scene->numLights; i++) {
    printf("Light %d\n", i);
    printf("Position: %f %f %f\n", scene->lights[i].position.x,
           scene->lights[i].position.y, scene->lights[i].position.z);
    printf("Type: %d\n", scene->lights[i].type);
    printf("Intensity: %f\n\n", scene->lights[i].intensity);
  }
}

void parsePPM(Texture* texture, char* filename, char* inputPath) {
  char newname[MAXCHAR];
  {
    char path[MAXCHAR];
    char* lastSlash = strrchr(inputPath, '/');
    if (lastSlash == NULL) {
      sprintf(newname, "texture/%s", filename);
    } else {
      strncpy(path, inputPath, lastSlash - inputPath + 1);
      newname[lastSlash - inputPath + 1] = '\0';
      sprintf(newname, "%stexture/%s", path, filename);
    }
  }

  FILE* fp = fopen(newname, "r");
  if (fp == NULL) {
    printf("Error: file %s not found\n", newname);
    exit(1);
  }

  int width, height, maxColor;
  // remove P3
  {
    char buffer[MAXCHAR];
    fscanf(fp, "%s", buffer);
    if (strcmp(buffer, "P3") != 0) {
      printf("Error: invalid PPM file\n");
      fclose(fp);
      exit(1);
    }
  }
  fscanf(fp, "%d %d", &width, &height);
  fscanf(fp, "%d", &maxColor);

  if (width <= 0 || height <= 0 || maxColor != 255) {
    printf("Error: invalid PPM file\n");
    fclose(fp);
    exit(1);
  };

  texture->width = width;
  texture->height = height;
  texture->pixels = (Vec3**)malloc(height * sizeof(Vec3*));
  if (texture->pixels == NULL) {
    printf("Error: malloc failed\n");
    fclose(fp);
    exit(1);
  }
  for (int i = 0; i < height; i++) {
    texture->pixels[i] = (Vec3*)malloc(width * sizeof(Vec3));
    if (texture->pixels[i] == NULL) {
      printf("Error: malloc failed\n");
      fclose(fp);
      exit(1);
    }
  }

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      int r, g, b;
      fscanf(fp, "%d %d %d", &r, &g, &b);
      texture->pixels[i][j] =
          (Vec3){(float)r / maxColor, (float)g / maxColor, (float)b / maxColor};
    }
  }

  char buffer[MAXCHAR];
}