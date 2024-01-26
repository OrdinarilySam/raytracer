typedef struct {
    float r, g, b;
} ColorType;

typedef struct {
    float x, y, z;
} CoordType;

typedef struct {
    CoordType center;
    float r;
    int m;
} SphereType;

typedef struct {
    CoordType pos;
    CoordType dir;
} RayType;


ColorType traceRay(RayType ray);
ColorType shadeRay(SphereType closestSphere);
int exit(int value);