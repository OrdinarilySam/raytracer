#include <stdlib.h>
#include <math.h>

typedef struct
{
    union
    {
        struct
        {
            float x, y, z;
        };
        struct
        {
            float r, g, b;
        };
        struct
        {
            float u, v, w;
        };
    };
} Vec3;

float dot(Vec3 *a, Vec3 *b);
float length(Vec3 *a);

Vec3 *add(Vec3 *a, Vec3 *b);
Vec3 *sub(Vec3 *a, Vec3 *b);
Vec3 *mul(Vec3 *a, Vec3 *b);
Vec3 *div(Vec3 *a, Vec3 *b);

Vec3 *scale(Vec3 *a, float b);

Vec3 *normalize(Vec3 *a);
Vec3 *cross(Vec3 *a, Vec3 *b);