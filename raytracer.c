/**
 * Author: Sam Martin (MART6353)
 */

#include "raytracer.h"

int materialIndex = -1;
int ellipsoidIndex = -1;
int lightIndex = -1;
MaterialType *materials;
EllipsoidType *ellipsoids;
LightType *lights;
Vec3 bkgcolor = {-1, -1, -1};

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: ./<program> <inputfile>\n");
    return 1;
  }

  // count number of ellipsoids and materials
  int ellipsoidCount = 0;
  int materialCount = 0;
  int lightCount = 0;

  char buffer[100];
  FILE *fscan = fopen(argv[1], "r");
  if (fscan == NULL) {
    printf("Couldn't open file %s\n", argv[1]);
    return 1;
  }
  while (fscanf(fscan, "%s", buffer) != EOF) {
    if (strcmp(buffer, "sphere") == 0 || strcmp(buffer, "ellipse") == 0) {
      ellipsoidCount++;
    } else if (strcmp(buffer, "mtlcolor") == 0) {
      materialCount++;
    }
  }
  fclose(fscan);

  // allocate material array
  materials = malloc(1 + materialCount * sizeof(MaterialType));
  if (materials == NULL) {
    printf("Failed to initialize a material array. Exiting...\n");
    return 1;
  }

  // allocate sphere array
  ellipsoids = malloc(ellipsoidCount * sizeof(EllipsoidType));
  if (ellipsoids == NULL) {
    printf("Failed to initialize an object array. Exiting...\n");
    free(materials);
    return 1;
  }

  lights = malloc(lightCount * sizeof(LightType));
  if (lights == NULL) {
    printf("Failed to initialize a light array. Exiting...\n");
    free(materials);
    free(ellipsoids);
    return 1;
  }

  // variable declaration
  Vec3 eye;
  Vec3 viewdir = {0, 0, 0};
  Vec3 updir = {0, 0, 0};
  float hfov = -1;
  int imgWidth, imgHeight = -1;

  short parallelViewEnabled, foundEye = 0;
  float frustumWidth, frustumHeight;

  /* ========================= INPUT HANDLING ========================= */

  FILE *fptr = fopen(argv[1], "r");
  if (fptr == NULL) {
    printf("Couldn't open file %s\n", argv[1]);
    return 1;
  }

  while (fscanf(fptr, "%s", buffer) != EOF) {
    if (strcmp(buffer, "eye") == 0) {
      // set the eye pos
      fscanf(fptr, "%f %f %f", &eye.x, &eye.y, &eye.z);
      foundEye = 1;
    } else if (strcmp(buffer, "parallel") == 0) {
      // create a parallel view image
      parallelViewEnabled = 1;
      fscanf(fptr, "%f", &frustumWidth);
    } else if (strcmp(buffer, "viewdir") == 0) {
      // set the viewdir vector
      fscanf(fptr, "%f", &viewdir.x);
      fscanf(fptr, "%f", &viewdir.y);
      fscanf(fptr, "%f", &viewdir.z);
    } else if (strcmp(buffer, "updir") == 0) {
      // seet the updir vector
      fscanf(fptr, "%f", &updir.x);
      fscanf(fptr, "%f", &updir.y);
      fscanf(fptr, "%f", &updir.z);
    } else if (strcmp(buffer, "hfov") == 0) {
      // set the horizontal fov
      fscanf(fptr, "%f", &hfov);
    } else if (strcmp(buffer, "imsize") == 0) {
      // set the image size
      fscanf(fptr, "%d", &imgWidth);
      fscanf(fptr, "%d", &imgHeight);
    } else if (strcmp(buffer, "bkgcolor") == 0) {
      // set the background color
      fscanf(fptr, "%f", &bkgcolor.r);
      fscanf(fptr, "%f", &bkgcolor.g);
      fscanf(fptr, "%f", &bkgcolor.b);
      // add the background color as a global variable
    } else if (strcmp(buffer, "mtlcolor") == 0) {
      MaterialType newMaterial;

      Vec3 tempColor;

      fscanf(fptr, "%f", &tempColor.r);
      fscanf(fptr, "%f", &tempColor.g);
      fscanf(fptr, "%f", &tempColor.b);
      if (((tempColor.r < 0) || (tempColor.r > 1)) ||
          ((tempColor.g < 0) || (tempColor.g > 1)) ||
          ((tempColor.b < 0) || (tempColor.b > 1))) {
        printf("Material color must be between 0 and 1\n");
        return cleanExit(1);
      }

      newMaterial.diffuseColor = tempColor;

      fscanf(fptr, "%f", &tempColor.r);
      fscanf(fptr, "%f", &tempColor.g);
      fscanf(fptr, "%f", &tempColor.b);
      if (((tempColor.r < 0) || (tempColor.r > 1)) ||
          ((tempColor.g < 0) || (tempColor.g > 1)) ||
          ((tempColor.b < 0) || (tempColor.b > 1))) {
        printf("Material color must be between 0 and 1\n");
        return cleanExit(1);
      }

      newMaterial.spectralColor = tempColor;

      fscanf(fptr, "%f", &newMaterial.ka);
      fscanf(fptr, "%f", &newMaterial.kd);
      fscanf(fptr, "%f", &newMaterial.ks);

      if (((newMaterial.ka < 0) || (newMaterial.ka > 1)) ||
          ((newMaterial.kd < 0) || (newMaterial.kd > 1)) ||
          ((newMaterial.ks < 0) || (newMaterial.ks > 1))) {
        printf("Color term weights must be between 0 and 1\n");
        return cleanExit(1);
      }

      fscanf(fptr, "%f", &newMaterial.n);

      if (newMaterial.n < 0) {
        printf("n must be positive\n");
        return cleanExit(1);
      }

      printf("Material %d: %f %f %f %f %f %f %f %f %f %f\n", materialIndex,
             newMaterial.diffuseColor.r, newMaterial.diffuseColor.g,
             newMaterial.diffuseColor.b, newMaterial.spectralColor.r,
             newMaterial.spectralColor.g, newMaterial.spectralColor.b,
             newMaterial.ka, newMaterial.kd, newMaterial.ks, newMaterial.n);

      // add the material to the array
      materialIndex++;
      materials[materialIndex] = newMaterial;
    } else if (strcmp(buffer, "sphere") == 0) {
      // create the new sphere
      EllipsoidType newSphere;
      fscanf(fptr, "%f", &newSphere.center.x);
      fscanf(fptr, "%f", &newSphere.center.y);
      fscanf(fptr, "%f", &newSphere.center.z);

      float radius;
      fscanf(fptr, "%f", &radius);
      newSphere.radius.x = radius;
      newSphere.radius.y = radius;
      newSphere.radius.z = radius;

      // ensure the sphere has size
      if (radius <= 0) {
        printf("Sphere radius must be greater than 0\n");
        return cleanExit(1);
      }

      // make sure a material was assigned
      if (materialIndex < 0) {
        printf("Must have at least one mtlcolor before a sphere.\n");
        return cleanExit(1);
      }
      newSphere.m = materialIndex;

      // add the sphere to the array
      ellipsoidIndex++;
      ellipsoids[ellipsoidIndex] = newSphere;
    } else if (strcmp(buffer, "ellipse") == 0) {
      // create the new ellipsoid
      EllipsoidType newEllipsoid;
      fscanf(fptr, "%f", &newEllipsoid.center.x);
      fscanf(fptr, "%f", &newEllipsoid.center.y);
      fscanf(fptr, "%f", &newEllipsoid.center.z);

      fscanf(fptr, "%f", &newEllipsoid.radius.x);
      fscanf(fptr, "%f", &newEllipsoid.radius.y);
      fscanf(fptr, "%f", &newEllipsoid.radius.z);

      // ensure the ellipsoid has size
      if (newEllipsoid.radius.x <= 0) {
        printf("Ellipsoid radii must be greater than 0\n");
        return cleanExit(1);
      }
      if (newEllipsoid.radius.y <= 0) {
        printf("Ellipsoid radii must be greater than 0\n");
        return cleanExit(1);
      }
      if (newEllipsoid.radius.z <= 0) {
        printf("Ellipsoid radii must be greater than 0\n");
        return cleanExit(1);
      }

      // make sure a material was assigned
      if (materialIndex == 0) {
        printf("Must have at least one mtlcolor before an ellipsoid.\n");
        return cleanExit(1);
      }
      newEllipsoid.m = materialIndex;

      // add the sphere to the array
      ellipsoidIndex++;
      ellipsoids[ellipsoidIndex] = newEllipsoid;
    } else if (strcmp(buffer, "light") == 0) {
      // create the new light
      Vec3 lightPos;
      fscanf(fptr, "%f", &lightPos.x);
      fscanf(fptr, "%f", &lightPos.y);
      fscanf(fptr, "%f", &lightPos.z);

      short lightType;
      fscanf(fptr, "%hd", &lightType);

      float lightIntensity;
      fscanf(fptr, "%f", &lightIntensity);

      LightType newLight;
      newLight.pos = lightPos;
      newLight.intensity = lightIntensity;
      newLight.type = lightType;

      if (lightType < 0 || lightType > 1) {
        printf("Light type must be 0 or 1\n");
        return cleanExit(1);
      }

      if (lightIntensity < 0 || lightIntensity > 1) {
        printf("Light intensity must be between 0 and 1\n");
        return cleanExit(1);
      }

      lightIndex++;
      lights[lightIndex] = newLight;
    }
  }

  fclose(fptr);

  /* ========================= ERROR CHECKING ========================= */

  // check for missing eye parameter
  if (!foundEye) {
    printf("Missing eye parameter\n");
    return cleanExit(1);
  }

  // check that viewdir has length
  if (length(&viewdir) <= 0.0) {
    printf("viewdir must have length\n");
    return cleanExit(1);
  }

  // check that updir has length
  if (length(&updir) <= 0.0) {
    printf("updir must have length\n");
    return cleanExit(1);
  }
  // normalize updir
  normalize(&updir);

  // check that 0 < hfov < 360
  if (((hfov <= 0) || (hfov >= 180)) && !parallelViewEnabled) {
    printf("hfov must be between 0 and 180 (exclusive)\n");
    return cleanExit(1);
  }

  // check for positive frustum width
  if (parallelViewEnabled && (frustumWidth <= 0)) {
    printf("frustum width must be greater than 0\n");
    return cleanExit(1);
  }

  // check that imsize is greater than 1 pixel
  if ((1 >= imgHeight) || (1 >= imgWidth)) {
    printf("Image width and height must be more than 1 pixel\n");
    return cleanExit(1);
  }

  // check that background colors are between 0 and 1
  if (((bkgcolor.r < 0) || (bkgcolor.r > 1)) ||
      ((bkgcolor.g < 0) || (bkgcolor.g > 1)) ||
      ((bkgcolor.b < 0) || (bkgcolor.b > 1))) {
    printf("Colors must be between 0 and 1 (inclusive)\n");
    return cleanExit(1);
  }

  /* ========================= CALCULATE VARIABLES ========================= */

  // create a normalized viewingdir vector
  Vec3 w = viewdir;
  normalize(&w);

  // u is the cross product of viewdir and updir
  Vec3 u = cross(w, updir);

  // normalize u
  if (length(&u) <= 0.001) {
    printf("the viewing and up directions are too close to parallel\n");
    return cleanExit(1);
  }
  normalize(&u);

  // v is the cross product of u and viewdir
  Vec3 v = cross(u, w);

  float viewWidth, viewHeight;

  Vec3 ul, ur, ll, lr;
  Vec3 hChange, vChange;

  if (!parallelViewEnabled) {
    /* ========================= PERSPECTIVE PROJECTION
     * ========================= */

    // calculate viewing window size
    viewWidth = 2 * tanf((hfov * (M_PI / 180.0)) / 2.0);
    viewHeight = viewWidth / ((float)imgWidth / (float)imgHeight);

    float halfViewWidth = viewWidth / 2;
    float halfViewHeight = viewHeight / 2;

    Vec3 scaledU = scale(u, halfViewWidth);
    Vec3 scaledV = scale(v, halfViewHeight);

    Vec3 addEye = pointAdd(eye, w);

    // calculate corners of viewing window
    ul = pointAdd(pointSub(addEye, scaledU), scaledV);
    ur = pointAdd(pointAdd(addEye, scaledU), scaledV);
    ll = pointSub(pointSub(addEye, scaledU), scaledV);
    lr = pointSub(pointAdd(addEye, scaledU), scaledV);

    // calculate the horizontal and vertical offset per pixel
    hChange = scale(pointSub(ur, ul), 1.0 / (imgWidth - 1));
    vChange = scale(pointSub(ll, ul), 1.0 / (imgHeight - 1));

  } else {
    /* ========================= PARALLEL PROJECTION =========================
     */

    frustumHeight = frustumWidth / ((float)imgWidth / (float)imgHeight);
    float halfFrustumWidth = frustumWidth / 2;
    float halfFrustumHeight = frustumHeight / 2;

    Vec3 scaledU = scale(u, halfFrustumWidth);
    Vec3 scaledV = scale(v, halfFrustumHeight);

    // calculate corners of viewing window
    ul = pointAdd(pointSub(eye, scaledU), scaledV);
    ur = pointAdd(pointAdd(eye, scaledU), scaledV);
    ll = pointSub(pointSub(eye, scaledU), scaledV);
    lr = pointSub(pointAdd(eye, scaledU), scaledV);

    // calculate the horizontal and vertical offset per pixel
    hChange = scale(pointSub(ur, ul), 1.0 / (imgWidth - 1));
    vChange = scale(pointSub(ll, ul), 1.0 / (imgHeight - 1));
  }

  /* ========================= DEFINING PIXEL COLORS =========================
   */

  // find where the dot is
  char *dotPosition = strrchr(argv[1], '.');

  // create a new file name and name it accordingly
  char newFileName[strlen(argv[1]) + strlen(".ppm") + 1];

  if (dotPosition != NULL || dotPosition == argv[1]) {
    strncpy(newFileName, argv[1], dotPosition - argv[1]);
    strcpy(newFileName + (dotPosition - argv[1]), ".ppm");
  } else {
    strcpy(newFileName, argv[1]);
    strcat(newFileName, ".ppm");
  }

  // open the new file for writing
  FILE *outputFile = fopen(newFileName, "w");
  if (fptr == NULL) {
    printf("Couldn't create file %s\n", argv[1]);
    return cleanExit(1);
  }
  // create the header for the ppm
  fprintf(outputFile, "P3 %d %d %d\n", imgWidth, imgHeight, 255);

  for (int i = 0; i < imgHeight; i++) {
    for (int j = 0; j < imgWidth; j++) {
      // calculate the point on the viewing window the ray will shoot
      Vec3 pointThrough =
          pointAdd(pointAdd(ul, scale(vChange, i)), scale(hChange, j));

      RayType curRay;

      if (parallelViewEnabled == 1) {
        // calculate the parallel view
        curRay.pos = pointThrough;
        curRay.dir = w;

      } else {
        // calculate the perspective view
        curRay.pos = eye;

        Vec3 perspectiveView = pointSub(pointThrough, eye);
        normalize(&perspectiveView);

        curRay.dir = perspectiveView;
      }

      Vec3 fColor = traceRay(curRay);
      fprintf(outputFile, "%d %d %d\n", (int)(fColor.r * 255),
              (int)(fColor.g * 255), (int)(fColor.b * 255));
    }
  }

  fclose(outputFile);
  return cleanExit(0);
}

Vec3 traceRay(RayType ray) {
  float minimumDistance = MAXFLOAT;
  EllipsoidType currentEllipsoid;

  Vec3 pointHit;

  for (int i = 0; i <= ellipsoidIndex; i++) {
    // calculate components of distance solution
    float A = (powf(ray.dir.x / ellipsoids[i].radius.x, 2.0) +
               powf(ray.dir.y / ellipsoids[i].radius.y, 2.0) +
               powf(ray.dir.z / ellipsoids[i].radius.z, 2.0));

    float B = 2.0 * ((((ray.pos.x - ellipsoids[i].center.x) * ray.dir.x) /
                      powf(ellipsoids[i].radius.x, 2.0)) +
                     (((ray.pos.y - ellipsoids[i].center.y) * ray.dir.y) /
                      powf(ellipsoids[i].radius.y, 2.0)) +
                     (((ray.pos.z - ellipsoids[i].center.z) * ray.dir.z) /
                      powf(ellipsoids[i].radius.z, 2.0)));

    float C =
        (powf((ray.pos.x - ellipsoids[i].center.x) / ellipsoids[i].radius.x,
              2.0) +
         powf((ray.pos.y - ellipsoids[i].center.y) / ellipsoids[i].radius.y,
              2.0) +
         powf((ray.pos.z - ellipsoids[i].center.z) / ellipsoids[i].radius.z,
              2.0) -
         1);

    // if discriminant is less than 0, ray does not hit object
    float discriminant = powf(B, 2.0) - (4 * A * C);
    if (discriminant < 0) {
      continue;
    }

    float plusT = (-B + sqrtf(discriminant)) / (2 * A);
    float minusT = (-B - sqrtf(discriminant)) / (2 * A);

    if ((plusT > 0) && (plusT < minimumDistance)) {
      minimumDistance = plusT;
      currentEllipsoid = ellipsoids[i];
      pointHit = pointAdd(ray.pos, scale(ray.dir, plusT));
    }

    if ((minusT > 0) && (minusT < minimumDistance)) {
      minimumDistance = minusT;
      currentEllipsoid = ellipsoids[i];
      pointHit = pointAdd(ray.pos, scale(ray.dir, minusT));
    }
  }

  // check for unchanged value
  if (minimumDistance == MAXFLOAT) {
    return bkgcolor;
  }
  return shadeRay(currentEllipsoid, pointHit, ray);
}

Vec3 shadeRay(EllipsoidType closestEllipsoid, Vec3 pointHit, RayType ray) {
  MaterialType surfaceMaterial = materials[closestEllipsoid.m];
  Vec3 color = scale(surfaceMaterial.diffuseColor, surfaceMaterial.ka);

  for (int i = 0; i <= lightIndex; i++) {
    Vec3 lightDir;
    if (lights[i].type == 1) {
      lightDir = pointSub(lights[i].pos, pointHit);
    } else {
      lightDir = scale(lights[i].dir, -1);
    }

    normalize(&lightDir);

    Vec3 N = pointDiv(pointSub(pointHit, closestEllipsoid.center),
                      closestEllipsoid.radius);

    float NdotL = dot(&N, &lightDir);
    if (NdotL <= 0) {
      continue;
    }

    Vec3 V = scale(ray.dir, -1);

    Vec3 H = scale(pointAdd(lightDir, V), 0.5);
    normalize(&H);

    float NdotH = dot(&N, &H);
    if (NdotH < 0) {
      NdotH = 0;
    }

    Vec3 diffuse =
        scale(surfaceMaterial.diffuseColor, NdotL * surfaceMaterial.kd);
    Vec3 specular = scale(surfaceMaterial.spectralColor,
                          powf(NdotH, surfaceMaterial.n) * surfaceMaterial.ks);

    color = pointAdd(color,
                     scale(pointAdd(diffuse, specular), lights[i].intensity));
  }

  if (color.r > 1) {
    color.r = 1;
  }
  if (color.g > 1) {
    color.g = 1;
  }
  if (color.b > 1) {
    color.b = 1;
  }
  return color;
}

int cleanExit(int value) {
  free(ellipsoids);
  free(materials);
  free(lights);
  return value;
}
