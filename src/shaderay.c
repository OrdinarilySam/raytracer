#include "shaderay.h"

void shadeTriangle(Scene *scene, Ray *ray, Triangle *face, float t) {
  Material material = scene->materials[face->material];
  Vec3 color = scale(material.diffuseColor, material.ka);
  
  Vec3 viewDir = scale(ray->direction, -1);
  Vec3 pointHit = pointAdd(ray->origin, scale(ray->direction, t));
  Vec3 normal = cross(pointSub(scene->vertices[face->vertices.v2], scene->vertices[face->vertices.v1]),
                      pointSub(scene->vertices[face->vertices.v3], scene->vertices[face->vertices.v1]));

  normalize(&normal);
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
    color = pointAdd(color, scale(pointAdd(diffuse, specular), light.intensity));
    if (color.r > 1) {
      color.r = 1;
    }
    if (color.g > 1) {
      color.g = 1;
    }
    if (color.b > 1) {
      color.b = 1;
    }

  }

  fprintf(scene->output, "%d %d %d\n", (int)(color.r * 255),
          (int)(color.g * 255), (int)(color.b * 255));

}

// todo: modify to utilize two separate functions
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
    float shadow = 1;
    float shadowCount = 0;

    srand(time(NULL));

    {
      Ray shadowRay = {pointHit, lightDir};
      Vec3 sampleLight;
      Vec3 orthogonal = generateOrthogonal(&lightDir);
      normalize(&orthogonal);

      Vec3 toLight = pointSub(light.position, pointHit);
      float distToLight = length(&toLight);

      if (light.type) {
        attenuation = getAttenuation(&light, distToLight);
      }

      if (scene->softShadows) {
        int numSteps = 100;
        float maxScalingFactor = 0.2;
        for (int i = 0; i < numSteps; i++) {
          jitterShadowRay(&shadowRay, &orthogonal, &lightDir, &pointHit,
                          maxScalingFactor);
          float distToObject = shadowCheck(scene, &shadowRay, ellipsoid);

          if (light.type && distToObject < distToLight) {
            shadowCount++;
          } else if (!light.type && distToObject != INFINITY) {
            shadowCount++;
          }
        }
        shadow = 1 - (shadowCount / numSteps);
      } else {
        if (light.type &&
            shadowCheck(scene, &shadowRay, ellipsoid) < distToLight) {
          shadow = 0;
        } else if (!light.type &&
                   shadowCheck(scene, &shadowRay, ellipsoid) != INFINITY) {
          shadow = 0;
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

    float factors = light.intensity * attenuation * shadow;
    color = pointAdd(color, scale(pointAdd(diffuse, specular), factors));
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

    float t = raySphereIntersection(shadowRay, currentEllipsoid);
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

Vec3 generateOrthogonal(Vec3 *v) {
  Vec3 result;
  float vx = fabsf(v->x);
  float vy = fabsf(v->y);
  float vz = fabsf(v->z);

  if (vx < vy) {
    if (vx < vz) {
      result = (Vec3){0, v->z, -v->y};
    } else {
      result = (Vec3){v->y, -v->x, 0};
    }
  } else {
    if (vy < vz) {
      result = (Vec3){-v->z, 0, v->x};
    } else {
      result = (Vec3){v->y, -v->x, 0};
    }
  }
  return result;
}

Vec3 rotateAroundAxis(Vec3 *v, Vec3 *axis, float angle) {
  Vec3 result;
  float c = cosf(angle);
  float s = sinf(angle);
  float t = 1 - c;

  result.x = v->x * (t * axis->x * axis->x + c) +
             v->y * (t * axis->x * axis->y - s * axis->z) +
             v->z * (t * axis->x * axis->z + s * axis->y);
  result.y = v->x * (t * axis->x * axis->y + s * axis->z) +
             v->y * (t * axis->y * axis->y + c) +
             v->z * (t * axis->y * axis->z - s * axis->x);
  result.z = v->x * (t * axis->x * axis->z - s * axis->y) +
             v->y * (t * axis->y * axis->z + s * axis->x) +
             v->z * (t * axis->z * axis->z + c);
  return result;
}

void jitterShadowRay(Ray *shadowRay, Vec3 *orthogonal, Vec3 *lightDir,
                     Vec3 *pointHit, float maxScalingFactor) {
  int angle = (float)rand() / (float)(RAND_MAX) * 360;
  Vec3 rotated = rotateAroundAxis(orthogonal, lightDir, (angle * M_PI / 180));
  float scalingFactor = (float)rand() / (float)(RAND_MAX)*maxScalingFactor;
  rotated = scale(rotated, scalingFactor);
  shadowRay->origin = pointAdd(*pointHit, rotated);
}