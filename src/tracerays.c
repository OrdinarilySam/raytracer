#include "tracerays.h"

atomic_int rowsRendered = 0;
int spinnerIndex = 0;

void* traceRaysThread(void* arg) {
  ThreadArgs* data = (ThreadArgs*)arg;
  Scene* scene = data->scene;
  int start = data->start;
  int endRow = data->end;
  Vec3 ul = data->threadData->ul;
  Vec3 vChange = data->threadData->vChange;
  Vec3 hChange = data->threadData->hChange;
  Vec3 w = data->threadData->w;

  for (int i = start; i < endRow; i++) {
    for (int j = 0; j < scene->imgsize.width; j++) {
      Vec3 pointThrough =
          pointAdd(pointAdd(ul, scale(vChange, i)), scale(hChange, j));
      Ray curRay;
      curRay.depth = 1;

      if (!scene->parallel) {
        curRay.origin = scene->eye;
        curRay.direction = pointSub(pointThrough, scene->eye);
        normalize(&curRay.direction);
      } else {
        curRay.origin = pointThrough;
        curRay.direction = w;
      }

      Vec3 color = traceRay(scene, &curRay);
      scene->pixels[i][j] = color;
    }
    atomic_fetch_add(&rowsRendered, 1);
  }

  return NULL;
}

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

  int numThreads = sysconf(_SC_NPROCESSORS_ONLN);

  if (THREADS_ON && numThreads < scene->imgsize.height) {
    ThreadData* threadData = (ThreadData*)malloc(sizeof(ThreadData));
    if (threadData == NULL) {
      printf("Error: malloc failed\n");
      exit(1);
    }

    threadData->ul = ul;
    threadData->vChange = vChange;
    threadData->hChange = hChange;
    threadData->w = w;

    pthread_t threads[numThreads];
    ThreadArgs args[numThreads];
    int rowsPerThread = scene->imgsize.height / numThreads;

    printf("Rendering scene with %d threads\n", numThreads);

    for (int i = 0; i < numThreads; i++) {
      args[i].scene = scene;
      args[i].threadData = threadData;
      args[i].start = i * rowsPerThread;
      args[i].end = (i + 1) * rowsPerThread;
      if (i == numThreads - 1) {
        args[i].end = scene->imgsize.height;
      }
      pthread_create(&threads[i], NULL, traceRaysThread, &args[i]);
    }
    while (atomic_load(&rowsRendered) < scene->imgsize.height) {
      printProgressBar(atomic_load(&rowsRendered), scene->imgsize.height);
      usleep(100000);
    }
    for (int i = 0; i < numThreads; i++) {
      pthread_join(threads[i], NULL);
    }
    free(threadData);

  } else {
    for (int i = 0; i < scene->imgsize.height; i++) {
      printProgressBar(i, scene->imgsize.height);
      for (int j = 0; j < scene->imgsize.width; j++) {
        Vec3 pointThrough =
            pointAdd(pointAdd(ul, scale(vChange, i)), scale(hChange, j));
        Ray curRay;
        curRay.depth = 1;

        if (!scene->parallel) {
          curRay.origin = scene->eye;
          curRay.direction = pointSub(pointThrough, scene->eye);
          normalize(&curRay.direction);
        } else {
          curRay.origin = pointThrough;
          curRay.direction = w;
        }

        Vec3 color = traceRay(scene, &curRay);
        scene->pixels[i][j] = color;
      }
    }
  }

  printProgressBar(scene->imgsize.height, scene->imgsize.height);
  printf("\n");
}

Vec3 traceRay(Scene* scene, Ray* ray) {
  float minimumDistance = INFINITY;
  Ellipsoid* closestEllipsoid;
  Triangle* closestFace;
  bool hit = false;
  bool isSphere = true;

  for (int i = 0; i < scene->numEllipsoids; i++) {
    Ellipsoid* ellipsoid = &scene->ellipsoids[i];
    float t = raySphereIntersection(ray, ellipsoid);

    if (t > 0 && t < minimumDistance && t > EPSILON) {
      minimumDistance = t;
      closestEllipsoid = ellipsoid;
      hit = true;
    }
  }

  for (int i = 0; i < scene->numFaces; i++) {
    Triangle* face = &scene->faces[i];
    float t = rayTriangleIntersection(scene, ray, face);

    if (t > 0 && t < minimumDistance && t > EPSILON) {
      isSphere = false;
      minimumDistance = t;
      closestFace = face;
      hit = true;
    }
  }

  if (!hit) {
    return scene->bkgcolor;
  }

  if (isSphere) {
    closestFace = NULL;
  } else {
    closestEllipsoid = NULL;
  }

  Vec3 color = shadeRay(scene, ray, closestEllipsoid, closestFace, minimumDistance);

  // Vec3 color;
  // if (isSphere) {
  //   color = shadeSphere(scene, ray, closestEllipsoid, minimumDistance);
  // } else {
  //   color = shadeTriangle(scene, ray, closestFace, minimumDistance);
  // }
  // scene->pixels[ray->location.i][ray->location.j] = color;
  return color;
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

  if (t1 < 0 || t1 < EPSILON) {
    return t2;
  }

  if (t2 < 0 || t2 < EPSILON) {
    return t1;
  }

  return t1 < t2 ? t1 : t2;
}

float rayTriangleIntersection(Scene* scene, Ray* ray, Triangle* face) {
  Indices vertices = face->vertices;
  Vec3 e1 =
      pointSub(scene->vertices[vertices.v2], scene->vertices[vertices.v1]);
  Vec3 e2 =
      pointSub(scene->vertices[vertices.v3], scene->vertices[vertices.v1]);
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

  Vec3 ep = pointSub(pointAdd(ray->origin, scale(ray->direction, t)),
                     scene->vertices[vertices.v1]);
  float d1p = dot(&ep, &e1);
  float d2p = dot(&ep, &e2);

  float beta = (d22 * d1p - d12 * d2p) / determinant;
  float gamma = (d11 * d2p - d12 * d1p) / determinant;

  if (beta < 0 || gamma < 0 || beta > 1 || gamma > 1 || beta + gamma > 1) {
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
    if (i < pos)
      printf("=");
    else if (i == pos)
      printf(">");
    else
      printf(" ");
  }
  printf("] ");
  if (progress < 1) {
    printf("%c ", "|/-\\"[spinnerIndex++ % 4]);
  }
  printf("%d%%    ", (int)(progress * 100));

  fflush(stdout);  // Ensures the output is printed immediately
}
