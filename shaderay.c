#include "shaderay.h"

void shadeRay(Scene *scene, Ray *ray, Ellipsoid *ellipsoid, float t) {
  Material material = scene->materials[ellipsoid->material];
  // fprintf(scene->output, "%d %d %d\n", (int)(material.diffuseColor.r * 255),
  //         (int)(material.diffuseColor.g * 255),
  //         (int)(material.diffuseColor.b * 255));
  // return;
  Vec3 color = scale(material.diffuseColor, material.ka);

  Vec3 normal = ellipsoidNormal(ellipsoid, ray->origin, t);
  Vec3 viewDir = scale(ray->direction, -1);
  Vec3 pointHit = pointAdd(ray->origin, scale(ray->direction, t));

  for (int i = 0; i < scene->numLights; i++) {
    Light light = scene->lights[i];

    Vec3 lightDir;
    if (light.type) {
      lightDir = pointSub(light.position, pointHit);
    } else {
      lightDir = scale(light.direction, -1);
    }
    normalize(&lightDir);

    // {
    //   Ray shadowRay = {pointHit, lightDir};
    //   Vec3 toLight = pointSub(light.position, pointHit);
    //   float distance = length(&toLight);
    //   if (shadowCheck(scene, &shadowRay, ellipsoid) < distance) {
    //     continue;
    //   }
    // }

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

    color = pointAdd(color, pointAdd(diffuse, specular));
  }

  fprintf(scene->output, "%d %d %d\n", (int)(color.r * 255),
          (int)(color.g * 255), (int)(color.b * 255));
}

Vec3 ellipsoidNormal(Ellipsoid *ellipsoid, Vec3 point, float t) {
  Vec3 normal = scale(pointSub(point, ellipsoid->center), 2);
  normal = pointDiv(normal, pointPower(ellipsoid->radii, 2));
  normalize(&normal);
  return normal;
}

float shadowCheck(Scene *scene, Ray *shadowRay, Ellipsoid *ellipsoid) {
  float minimumDistance = INFINITY;

  for (int i = 0; i < scene->numEllipsoids; i++) {
    Ellipsoid currentEllipsoid = scene->ellipsoids[i];
    if (&currentEllipsoid == ellipsoid) {
      continue;
    }

    float t = rayIntersection(shadowRay, &currentEllipsoid);
    if (t > 0 && t < minimumDistance) {
      minimumDistance = t;
    }
  }

  return minimumDistance;
}
