#include "shaderay.h"

Vec3 shadeRay(Scene *scene, Ray *ray, Ellipsoid *ellipsoid, Triangle *face,
              float t) {
  Vec3 pointHit = pointAdd(ray->origin, scale(ray->direction, t));
  Vec3 incident = scale(ray->direction, -1);

  Vec3 normal;
  Material material;
  Vec3 color;

  if (ellipsoid != NULL) {
    material = scene->materials[ellipsoid->material];
    normal = ellipsoidNormal(ellipsoid, pointHit);

    if (ellipsoid->usingTexture) {
      Texel texel = getSphereTexel(&normal);
      Texture texture = scene->textures[ellipsoid->texture];
      int height = texture.height - 1;
      int width = texture.width - 1;

      float x, y;

      {
        float dummy;
        float x = modff(texel.u, &dummy) * (float)width;
        float y = modff(texel.v, &dummy) * (float)height;
      }

      int i = (int)x;
      int j = (int)y;

      int i2 = i + 1 > width ? i : i + 1;
      int j2 = j + 1 > height ? j : j + 1;

      float alpha = x - (float)i;
      float beta = y - (float)j;

      Vec3 color1 = scale(texture.pixels[j][i], (1 - alpha) * (1 - beta));
      Vec3 color2 = scale(texture.pixels[j][i2], alpha * (1 - beta));
      Vec3 color3 = scale(texture.pixels[j2][i], (1 - alpha) * beta);
      Vec3 color4 = scale(texture.pixels[j2][i2], alpha * beta);

      material.diffuseColor =
          pointAdd(color1, pointAdd(color2, pointAdd(color3, color4)));
    }

  } else if (face != NULL) {
    material = scene->materials[face->material];
    Vec3 barycentric = calculateBarycentric(scene, face, &pointHit);

    if (face->type == VERTICES_TEXTURES ||
        face->type == VERTICES_NORMALS_TEXTURES) {
      Texel texel = getTriangleTexel(scene, face, &barycentric);
      Texture texture = scene->textures[face->texture];
      int height = texture.height - 1;
      int width = texture.width - 1;

      float x, y;

      {
        float dummy;
        x = modff(texel.u, &dummy) * (float)width;
        y = modff(texel.v, &dummy) * (float)height;
      }

      int i = (int)x;
      int j = (int)y;

      int i2 = i + 1 > width ? i : i + 1;
      int j2 = j + 1 > height ? j : j + 1;

      float alpha = x - (float)i;
      float beta = y - (float)j;

      Vec3 color1 = scale(texture.pixels[j][i], (1 - alpha) * (1 - beta));
      Vec3 color2 = scale(texture.pixels[j][i2], alpha * (1 - beta));
      Vec3 color3 = scale(texture.pixels[j2][i], (1 - alpha) * beta);
      Vec3 color4 = scale(texture.pixels[j2][i2], alpha * beta);

      material.diffuseColor =
          pointAdd(color1, pointAdd(color2, pointAdd(color3, color4)));
    }

    if (face->type == VERTICES_NORMALS ||
        face->type == VERTICES_NORMALS_TEXTURES) {
      Vec3 normal1 = scene->normals[face->normals.v1];
      Vec3 normal2 = scene->normals[face->normals.v2];
      Vec3 normal3 = scene->normals[face->normals.v3];

      normal = pointAdd(
          scale(scene->normals[face->normals.n1], barycentric.x),
          pointAdd(scale(scene->normals[face->normals.n2], barycentric.y),
                   scale(scene->normals[face->normals.n3], barycentric.z)));
    } else {
      Indices vertices = face->vertices;
      Vec3 e1 =
          pointSub(scene->vertices[vertices.v2], scene->vertices[vertices.v1]);
      Vec3 e2 =
          pointSub(scene->vertices[vertices.v3], scene->vertices[vertices.v1]);

      normal = cross(e1, e2);
    }

  } else {
    return (Vec3){0, 0, 0};
  }

  color = scale(material.diffuseColor, material.ka);

  normalize(&normal);
  normalize(&incident);

  float eta;

  color = scale(material.diffuseColor, material.ka);

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
      } else {
        distToLight = INFINITY;
      }

      if (scene->softShadows) {
        int numSteps = 100;
        float maxScalingFactor = 0.2;
        for (int i = 0; i < numSteps; i++) {
          jitterShadowRay(&shadowRay, &orthogonal, &lightDir, &pointHit,
                          maxScalingFactor);
          shadow +=
              shadowCheck(scene, &shadowRay, ellipsoid, face, distToLight);
        }
        shadow /= numSteps;
      } else {
        shadow = shadowCheck(scene, &shadowRay, ellipsoid, face, distToLight);
      }
    }

    float NdotL = dot(&normal, &lightDir);
    if (NdotL < 0) {
      continue;
    }

    Vec3 halfVec = scale(pointAdd(incident, lightDir), 0.5);
    normalize(&halfVec);

    float NdotH = dot(&normal, &halfVec);
    if (NdotH < 0) {
      NdotH = 0;
    }

    Vec3 diffuse = scale(material.diffuseColor, material.kd * NdotL);
    Vec3 specular =
        scale(material.specularColor, material.ks * pow(NdotH, material.n));

    float factors = light.intensity * attenuation * shadow;
    // float factors = light.intensity * attenuation;
    // float factors = light.intensity;
    color = pointAdd(color, scale(pointAdd(diffuse, specular), factors));
  }
  // if (color.r < 0.5 || color.g < 0.5 || color.b < 0.5) {
  //   printf("low color values: %f %f %f\n", color.r, color.g, color.b);
  // }

  float IdotN = dot(&incident, &normal);
  if (IdotN < 0) {
    IdotN = -IdotN;
    eta = material.eta / scene->backgroundEta;
    normal = scale(normal, -1);
  } else {
    eta = scene->backgroundEta / material.eta;
  }

  Vec3 reflection = (Vec3){0, 0, 0};
  Vec3 refraction = (Vec3){0, 0, 0};

  float fresnel;

  {
    float F0 = pow((material.eta - 1) / (material.eta + 1), 2);
    fresnel = F0 + (1 - F0) * pow(1 - IdotN, 5);
  }

  if (material.ks > 0 && ray->depth < MAX_DEPTH) {
    Vec3 reflectDir = pointSub(scale(normal, 2 * IdotN), incident);
    Ray reflectedRay = {pointHit, reflectDir};
    reflectedRay.depth = ray->depth + 1;
    reflection = traceRay(scene, &reflectedRay);
    reflection = scale(reflection, fresnel);
  }

  if (material.alpha < 1 && ray->depth < MAX_DEPTH) {
    if (IdotN > 1) {
      IdotN = 1;
    }

    float k = 1 - eta * eta * (1 - IdotN * IdotN);

    if (k >= 0) {
      Vec3 refractDir =
          pointAdd(scale(normal, -1 * sqrt(k)),
                   scale(pointSub(scale(normal, IdotN), incident), eta));
      normalize(&refractDir);

      Vec3 newPoint = pointSub(pointHit, scale(normal, EPSILON));
      Ray refractedRay = {pointHit, refractDir, ray->depth + 1};
      refraction = traceRay(scene, &refractedRay);

      refraction = scale(refraction, (1 - fresnel) * (1 - material.alpha));
    }
  }

  color = pointAdd(pointAdd(color, reflection), refraction);

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

  return color;
}

Vec3 ellipsoidNormal(Ellipsoid *ellipsoid, Vec3 point) {
  Vec3 normal = scale(pointSub(point, ellipsoid->center), 2);
  normal = pointDiv(normal, pointPower(ellipsoid->radii, 2));
  normalize(&normal);
  return normal;
}

float shadowCheck(Scene *scene, Ray *shadowRay, Ellipsoid *ellipsoid,
                  Triangle *face, float minimumDistance) {
  float shadow = 1;

  for (int i = 0; i < scene->numEllipsoids; i++) {
    Ellipsoid *currentEllipsoid = &scene->ellipsoids[i];
    if (currentEllipsoid == ellipsoid) {
      continue;
    }

    float t = raySphereIntersection(shadowRay, currentEllipsoid);
    if (t > 0 && t < minimumDistance) {
      shadow *= (1 - scene->materials[currentEllipsoid->material].alpha);
    }
  }

  for (int i = 0; i < scene->numFaces; i++) {
    Triangle *currentFace = &scene->faces[i];
    if (currentFace == face) {
      continue;
    }

    float t = rayTriangleIntersection(scene, shadowRay, currentFace);
    if (t > 0 && t < minimumDistance) {
      shadow *= (1 - scene->materials[currentFace->material].alpha);
    }
  }

  return shadow;
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

Vec3 calculateBarycentric(Scene *scene, Triangle *face, Vec3 *pointHit) {
  Vec3 v0 = pointSub(scene->vertices[face->vertices.v2],
                     scene->vertices[face->vertices.v1]);
  Vec3 v1 = pointSub(scene->vertices[face->vertices.v3],
                     scene->vertices[face->vertices.v1]);
  Vec3 v2 = pointSub(*pointHit, scene->vertices[face->vertices.v1]);

  float d00 = dot(&v0, &v0);
  float d01 = dot(&v0, &v1);
  float d11 = dot(&v1, &v1);
  float d20 = dot(&v2, &v0);
  float d21 = dot(&v2, &v1);

  float denom = d00 * d11 - d01 * d01;
  float beta = (d11 * d20 - d01 * d21) / denom;
  float gamma = (d00 * d21 - d01 * d20) / denom;
  float alpha = 1 - beta - gamma;

  return (Vec3){alpha, beta, gamma};
}

Texel getSphereTexel(Vec3 *normal) {
  float phi = acosf(normal->z);
  float theta = atan2f(normal->y, normal->x);
  float v = phi / M_PI;
  if (theta < 0) {
    theta += 2 * M_PI;
  }
  float u = theta / (2 * M_PI);

  if (u < 0) {
    u = 0;
  }

  if (v < 0) {
    v = 0;
  }

  return (Texel){u, v};
}

Texel getTriangleTexel(Scene *scene, Triangle *face, Vec3 *barycentric) {
  float wIndex1 = (scene->vertexTextures[face->textures.v1].u);
  float hIndex1 = (scene->vertexTextures[face->textures.v1].v);

  float wIndex2 = (scene->vertexTextures[face->textures.v2].u);
  float hIndex2 = (scene->vertexTextures[face->textures.v2].v);

  float wIndex3 = (scene->vertexTextures[face->textures.v3].u);
  float hIndex3 = (scene->vertexTextures[face->textures.v3].v);

  float u = barycentric->x * wIndex1 + barycentric->y * wIndex2 +
            barycentric->z * wIndex3;
  float v = barycentric->x * hIndex1 + barycentric->y * hIndex2 +
            barycentric->z * hIndex3;

  if (u < 0) {
    u = 0;
  }

  if (v < 0) {
    v = 0;
  }

  return (Texel){u, v};
}
