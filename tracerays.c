#include "tracerays.h"

void traceRays(Scene* scene) {
  Vec3 w = scene->viewdir;
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
      viewWidth = 2 * tanf(scene->hfov * (M_PI / 180.0) / 2);
      eye = pointAdd(scene->eye, w);

    } else {
      viewWidth = scene->frustum;
      eye = scene->eye;
    }

    viewHeight = viewWidth /
                 ((float)scene->imgsize.width * (float)scene->imgsize.height);

    halfWidth = viewWidth / 2;
    halfHeight = viewHeight / 2;

    scaledU = scale(u, halfWidth);
    scaledV = scale(v, halfHeight);

    ul = pointSub(pointAdd(eye, scaledU), scaledV);
    ur = pointAdd(pointAdd(eye, scaledU), scaledV);
    ll = pointSub(pointSub(eye, scaledU), scaledV);
    lr = pointAdd(pointSub(eye, scaledU), scaledV);

    hChange = scale(pointSub(ur, ul), 1.0 / (scene->imgsize.width - 1));
    vChange = scale(pointSub(ll, ul), 1.0 / (scene->imgsize.height - 1));
  }

  for (int i = 0; i < scene->imgsize.height; i++) {
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
}

void traceRay(Scene* scene, Ray* ray) {
  float minimumDistance = INFINITY;
  Ellipsoid closestEllipsoid;
  bool hit = false;

  for (int i = 0; i < scene->numEllipsoids; i++) {
    Ellipsoid ellipsoid = scene->ellipsoids[i];
    float t = rayIntersection(ray, &ellipsoid);

    if (t > 0 && t < minimumDistance) {
      minimumDistance = t;
      closestEllipsoid = ellipsoid;
      hit = true;
    }
  }

  if (hit) {
    shadeRay(scene, ray, &closestEllipsoid, minimumDistance);
  } else {
    fprintf(scene->output, "%d %d %d\n", (int)(scene->bkgcolor.r * 255),
            (int)(scene->bkgcolor.g * 255), (int)(scene->bkgcolor.b * 255));
  }
}

float rayIntersection(Ray* ray, Ellipsoid* ellipsoid) {
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
