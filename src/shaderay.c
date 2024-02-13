#include "shaderay.h"

void shadeRay(Scene *scene, Ray *ray, Ellipsoid *ellipsoid, float t) {
  Material material = scene->materials[ellipsoid->material];
  Vec3 color = scale(material.diffuseColor, material.ka);

  Vec3 viewDir = scale(ray->direction, -1);
  Vec3 pointHit = pointAdd(ray->origin, scale(ray->direction, t));
  Vec3 normal = ellipsoidNormal(ellipsoid, pointHit);

  normalize(&viewDir);

  for (int i = 0; i < scene->numLights; i++) {
    Light light = scene->lights[i];

    Vec3 lightDir;
    if (light.type) {
      lightDir = pointSub(light.position, pointHit);
    } else {
      lightDir = scale(light.direction, -1);
    }
    normalize(&lightDir);

    float attenuation = 1;

    {
      Ray shadowRay = {pointHit, lightDir};
      if (light.type) {
        Vec3 toLight = pointSub(light.position, pointHit);
        float distance = length(&toLight);

        attenuation = getAttenuation(&light, distance);

        if (shadowCheck(scene, &shadowRay, ellipsoid) < distance) {
          continue;
        }
      } else {
        if (shadowCheck(scene, &shadowRay, ellipsoid) != INFINITY) {
          continue;
        }
      }
    }

    float NdotL = dot(&normal, &lightDir);
    if (NdotL < 0) {
      continue;
    }

    Vec3 halfVec = scale(pointAdd(viewDir, lightDir), 0.5);
    normalize(&halfVec);

    float NdotH = dot(&normal, &halfVec);
    if (NdotH < 0) {
      NdotH = 0;
    }

    Vec3 diffuse = scale(material.diffuseColor, material.kd * NdotL);
    Vec3 specular =
        scale(material.specularColor, material.ks * pow(NdotH, material.n));

    color = pointAdd(color, scale(pointAdd(diffuse, specular),
                                  light.intensity * attenuation));
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

  if (scene->depthcue.enabled) {
    Vec3 eyeToHit = pointSub(scene->eye, pointHit);
    float depthCueing = getDepthCueing(&scene->depthcue, length(&eyeToHit));
    color = pointAdd(scale(color, depthCueing),
                     scale(scene->depthcue.color, 1 - depthCueing));
  }

  fprintf(scene->output, "%d %d %d\n", (int)(color.r * 255),
          (int)(color.g * 255), (int)(color.b * 255));
}

Vec3 ellipsoidNormal(Ellipsoid *ellipsoid, Vec3 point) {
  Vec3 normal = scale(pointSub(point, ellipsoid->center), 2);
  normal = pointDiv(normal, pointPower(ellipsoid->radii, 2));
  normalize(&normal);
  return normal;
}

float shadowCheck(Scene *scene, Ray *shadowRay, Ellipsoid *ellipsoid) {
  float minimumDistance = INFINITY;

  for (int i = 0; i < scene->numEllipsoids; i++) {
    Ellipsoid *currentEllipsoid = &scene->ellipsoids[i];
    if (currentEllipsoid == ellipsoid) {
      continue;
    }

    float t = rayIntersection(shadowRay, currentEllipsoid);
    if (t > 0 && t < minimumDistance) {
      minimumDistance = t;
    }
  }

  return minimumDistance;
}

float getAttenuation(Light *light, float distance) {
  if (light->attenuation.x + light->attenuation.y + light->attenuation.z > 0) {
    return 1 / (light->attenuation.x + light->attenuation.y * distance +
                light->attenuation.z * distance * distance);
  } else {
    return 1;
  }
}

float getDepthCueing(DepthCue *depthcue, float t) {
  if (t < depthcue->minDist) {
    return depthcue->maxA;
  }

  if (t > depthcue->maxDist) {
    return depthcue->minA;
  }

  return depthcue->minA + (depthcue->maxA - depthcue->minA) *
                              (depthcue->maxDist - t) /
                              (depthcue->maxDist - depthcue->minDist);
}