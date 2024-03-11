#include "tracerays.h"

void traceRays(Scene* scene) {
  Vec3 w = scene->viewdir;
  normalize(&w);
  Vec3 u = cross(w, scene->updir);
  normalize(&u);

  Vec3 v = cross(u, w);
  normalize(&v);

  Vec3 hChange, vChange;
  Vec3 ul, ur, ll, lr;

  {
    float viewWidth, viewHeight;
    float halfWidth, halfHeight;

    Vec3 scaledU, scaledV;

    Vec3 eye;

    if (!scene->parallel) {
      viewWidth = 2 * tanf(scene->hfov * (M_PI / 180.0) / 2.0);
      eye = pointAdd(scene->eye, w);

    } else {
      viewWidth = scene->frustum;
      eye = scene->eye;
    }

    viewHeight = viewWidth /
                 ((float)scene->imgsize.width / (float)scene->imgsize.height);

    halfWidth = viewWidth / 2;
    halfHeight = viewHeight / 2;

    scaledU = scale(u, halfWidth);
    scaledV = scale(v, halfHeight);

    ul = pointAdd(pointSub(eye, scaledU), scaledV);
    ur = pointAdd(pointAdd(eye, scaledU), scaledV);
    ll = pointSub(pointSub(eye, scaledU), scaledV);
    lr = pointSub(pointAdd(eye, scaledU), scaledV);

    hChange = scale(pointSub(ur, ul), 1.0 / (scene->imgsize.width - 1));
    vChange = scale(pointSub(ll, ul), 1.0 / (scene->imgsize.height - 1));
  }

  for (int i = 0; i < scene->imgsize.height; i++) {
    printProgressBar(i, scene->imgsize.height);
    for (int j = 0; j < scene->imgsize.width; j++) {
      Vec3 pointThrough =
          pointAdd(pointAdd(ul, scale(vChange, i)), scale(hChange, j));
      Ray curRay;

      if (!scene->parallel) {
        curRay.origin = scene->eye;
        curRay.direction = pointSub(pointThrough, scene->eye);
        normalize(&curRay.direction);
      } else {
        curRay.origin = pointThrough;
        curRay.direction = w;
      }

      traceRay(scene, &curRay);
    }
  }
  printProgressBar(scene->imgsize.height, scene->imgsize.height);
  printf("\n");
}

void traceRay(Scene* scene, Ray* ray) {
  float minimumDistance = INFINITY;
  Ellipsoid* closestEllipsoid;
  Triangle* closestFace;
  bool hit = false;
  bool isSphere = true;

  for (int i = 0; i < scene->numEllipsoids; i++) {
    Ellipsoid* ellipsoid = &scene->ellipsoids[i];
    float t = raySphereIntersection(ray, ellipsoid);

    if (t > 0 && t < minimumDistance) {
      minimumDistance = t;
      closestEllipsoid = ellipsoid;
      hit = true;
    }
  }

  for (int i = 0; i < scene->numFaces; i++) {
    Triangle* face = &scene->faces[i];
    float t = rayTriangleIntersection(scene, ray, face);

    if (t > 0 && t < minimumDistance) {
      isSphere = false;
      minimumDistance = t;
      closestFace = face;
      hit = true;
    }
  }

  if (!hit) {
    fprintf(scene->output, "%d %d %d\n", (int)(scene->bkgcolor.r * 255),
            (int)(scene->bkgcolor.g * 255), (int)(scene->bkgcolor.b * 255));
    return;
  }

  // todo: shaderay or shadetriangle
  if (isSphere) {
    shadeSphere(scene, ray, closestEllipsoid, minimumDistance);
  } else {
    shadeTriangle(scene, ray, closestFace, minimumDistance);   
  }
}

float raySphereIntersection(Ray* ray, Ellipsoid* ellipsoid) {
  float a, b, c;
  {
    Vec3 temp;
    Vec3 oneVector = (Vec3){1, 1, 1};

    temp = pointDiv(ray->direction, ellipsoid->radii);
    a = dot(&temp, &temp);

    temp = pointDiv(
        pointMult(pointSub(ray->origin, ellipsoid->center), ray->direction),
        pointPower(ellipsoid->radii, 2));
    b = 2 * dot(&temp, &oneVector);

    temp = pointDiv(pointSub(ray->origin, ellipsoid->center), ellipsoid->radii);
    c = dot(&temp, &temp) - 1;
  }

  float discriminant = b * b - 4 * a * c;
  if (discriminant < 0) {
    return -1;
  }

  float t1 = (-b + sqrt(discriminant)) / (2 * a);
  float t2 = (-b - sqrt(discriminant)) / (2 * a);

  if (t1 < 0) {
    return t2;
  }

  if (t2 < 0) {
    return t1;
  }

  return t1 < t2 ? t1 : t2;
}

float rayTriangleIntersection(Scene* scene, Ray* ray, Triangle* face) {
  Indices vertices = face->vertices;
  Vec3 e1 = pointSub(scene->vertices[vertices.v2], scene->vertices[vertices.v1]);
  Vec3 e2 = pointSub(scene->vertices[vertices.v3], scene->vertices[vertices.v1]);
  Vec3 n = cross(e1, e2);
  float D = -dot(&n, &scene->vertices[vertices.v1]);
  if (dot(&n, &ray->direction) == 0) {
    return -1;
  }

  float t = -(dot(&n, &ray->origin) + D) / dot(&n, &ray->direction);

  if (t < 0) {
    return -1;
  }

  float d11 = dot(&e1, &e1);
  float d12 = dot(&e1, &e2);
  float d22 = dot(&e2, &e2);

  float determinant = d11 * d22 - d12 * d12;
  if (determinant == 0) {
    return -1;
  }

  Vec3 ep = pointSub(pointAdd(ray->origin, scale(ray->direction, t)), scene->vertices[vertices.v1]);
  float d1p = dot(&ep, &e1);
  float d2p = dot(&ep, &e2);

  float beta = (d22 * d1p - d12 * d2p) / determinant;
  float gamma = (d11 * d2p - d12 * d1p) / determinant;

  if (beta < 0 || gamma < 0 ||
      beta > 1 || gamma > 1 ||
      beta + gamma > 1) {
    return -1;
  }

  return t;
  
}

void printProgressBar(int current, int total) {
    const int barWidth = 50;  
    float progress = (float)current / total;
    printf("\r["); 
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) printf("=");
        else if (i == pos) printf(">");
        else printf(" ");
    }
    printf("] %d%%", (int)(progress * 100));
    fflush(stdout);  // Ensures the output is printed immediately
}
