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
    }
    if (strcmp(buffer, "ellipse") == 0) {
      scene->numEllipsoids++;
    }
    if (strcmp(buffer, "mtlcolor") == 0) {
      scene->numMaterials++;
    }
    if (strcmp(buffer, "light") == 0) {
      scene->numLights++;
    }
  }
  fclose(file);

  scene->ellipsoids =
      (Ellipsoid*)malloc(scene->numEllipsoids * sizeof(Ellipsoid));
  scene->materials = (Material*)malloc(scene->numMaterials * sizeof(Material));
  scene->lights = (Light*)malloc(scene->numLights * sizeof(Light));

  file = fopen(filename, "r");
  if (file == NULL) {
    free(scene);
    printf("Error: file %s not found\n", filename);
    exit(1);
  }

  int ellipsoidIndex = 0;
  int materialIndex = 0;
  int lightIndex = 0;

  bool foundEye = false;

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
        fscanf(file, "%f %f %f", &scene->bkgcolor.x, &scene->bkgcolor.y,
               &scene->bkgcolor.z);
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
        fscanf(file, "%f %f %f", &scene->ellipsoids[ellipsoidIndex].center.x,
               &scene->ellipsoids[ellipsoidIndex].center.y,
               &scene->ellipsoids[ellipsoidIndex].center.z, &r);

        if (r <= 0) {
          printf("Error: radius must be greater than 0\n");
          freeAll(scene);
          fclose(file);
          exit(1);
        }

        scene->ellipsoids[ellipsoidIndex].radii = (Vec3){r, r, r};
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

        fscanf(file, "%f %f %f %f %f %f %f %f %f",
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

        scene->ellipsoids[ellipsoidIndex].material = materialIndex - 1;
        ellipsoidIndex++;
        break;

      case MTL_COLOR:
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

        materialIndex++;
        break;

      case LIGHT:
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

        lightIndex++;
        break;

      case UNKNOWN:
        break;
    }
  }

  fclose(file);

  if (foundEye == false) {
    printf("Error: eye not found\n");
    freeAll(scene);
    exit(1);
  }
  validateScene(scene);
}

void validateScene(Scene* scene) {
  if (scene->numMaterials == 0) {
    printf("Error: no materials defined\n");
    freeAll(scene);
    exit(1);
  }
  if (scene->numEllipsoids == 0) {
    printf("Error: no ellipsoids defined\n");
    freeAll(scene);
    exit(1);
  }
  if (scene->numLights == 0) {
    printf("Error: no lights defined\n");
    freeAll(scene);
    exit(1);
  }
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

  normalize(&scene->updir);
  normalize(&scene->viewdir);
}

void freeAll(Scene* scene) {
  free(scene->ellipsoids);
  free(scene->materials);
  free(scene->lights);
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
}

Keyword getKeyword(char* keyword) {
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
  return UNKNOWN;
}