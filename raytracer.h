typedef struct {
    float r, g, b;
} ColorType;

typedef struct {
    float x, y, z;
} CoordType;

typedef struct {
    CoordType center;
    CoordType radius;
    int m;
} EllipsoidType;

typedef struct {
    CoordType pos;
    CoordType dir;
} RayType;


ColorType traceRay(RayType ray);
ColorType shadeRay(EllipsoidType closestEllipsoid);
int cleanExit(int value);