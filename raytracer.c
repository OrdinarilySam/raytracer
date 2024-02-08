/**
 * Author: Sam Martin (MART6353)
 */

#include "raytracer.h"

int materialIndex = 0;
int ellipsoidIndex = -1;
Vec3 *materials;
EllipsoidType *ellipsoids;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: ./<program> <inputfile>\n");
        return 1;
    }

    // count number of ellipsoids and materials
    int ellipsoidCount = 0;
    int materialCount = 1;

    char buffer[100];
    FILE *fscan = fopen(argv[1], "r");
    if (fscan == NULL)
    {
        printf("Couldn't open file %s\n", argv[1]);
        return 1;
    }
    while (fscanf(fscan, "%s", buffer) != EOF)
    {
        if (strcmp(buffer, "sphere") == 0 || strcmp(buffer, "ellipse") == 0)
        {
            ellipsoidCount++;
        }
        else if (strcmp(buffer, "mtlcolor") == 0)
        {
            materialCount++;
        }
    }
    fclose(fscan);

    // allocate material array
    materials = malloc(materialCount * sizeof(Vec3));
    if (materials == NULL)
    {
        printf("Failed to initialize a material array. Exiting...\n");
        return 1;
    }

    // allocate sphere array
    ellipsoids = malloc(ellipsoidCount * sizeof(EllipsoidType));
    if (ellipsoids == NULL)
    {
        printf("Failed to initialize an object array. Exiting...\n");
        free(materials);
        return 1;
    }

    // variable declaration
    Vec3 eye;
    Vec3 viewdir = {0, 0, 0};
    Vec3 updir = {0, 0, 0};
    float hfov = -1;
    int imgWidth, imgHeight = -1;
    Vec3 bkgcolor = {-1, -1, -1};

    short parallelViewEnabled, foundEye = 0;
    float frustumWidth, frustumHeight;

    /* ========================= INPUT HANDLING ========================= */

    FILE *fptr = fopen(argv[1], "r");
    if (fptr == NULL)
    {
        printf("Couldn't open file %s\n", argv[1]);
        return 1;
    }

    while (fscanf(fptr, "%s", buffer) != EOF)
    {
        if (strcmp(buffer, "eye") == 0)
        {
            // set the eye pos
            fscanf(fptr, "%f %f %f", &eye.x, &eye.y, &eye.z);
            foundEye = 1;
        }
        else if (strcmp(buffer, "parallel") == 0)
        {
            // create a parallel view image
            parallelViewEnabled = 1;
            fscanf(fptr, "%f", &frustumWidth);
        }
        else if (strcmp(buffer, "viewdir") == 0)
        {
            // set the viewdir vector
            fscanf(fptr, "%f", &viewdir.x);
            fscanf(fptr, "%f", &viewdir.y);
            fscanf(fptr, "%f", &viewdir.z);
        }
        else if (strcmp(buffer, "updir") == 0)
        {
            // seet the updir vector
            fscanf(fptr, "%f", &updir.x);
            fscanf(fptr, "%f", &updir.y);
            fscanf(fptr, "%f", &updir.z);
        }
        else if (strcmp(buffer, "hfov") == 0)
        {
            // set the horizontal fov
            fscanf(fptr, "%f", &hfov);
        }
        else if (strcmp(buffer, "imsize") == 0)
        {
            // set the image size
            fscanf(fptr, "%d", &imgWidth);
            fscanf(fptr, "%d", &imgHeight);
        }
        else if (strcmp(buffer, "bkgcolor") == 0)
        {
            // set the background color
            fscanf(fptr, "%f", &bkgcolor.r);
            fscanf(fptr, "%f", &bkgcolor.g);
            fscanf(fptr, "%f", &bkgcolor.b);
            materials[0] = bkgcolor;
        }
        else if (strcmp(buffer, "mtlcolor") == 0)
        {
            // create the new material
            Vec3 newMaterial;
            fscanf(fptr, "%f", &newMaterial.r);
            fscanf(fptr, "%f", &newMaterial.g);
            fscanf(fptr, "%f", &newMaterial.b);

            // ensure the colors are valid
            if (
                ((newMaterial.r < 0) || (newMaterial.r > 1)) ||
                ((newMaterial.g < 0) || (newMaterial.g > 1)) ||
                ((newMaterial.b < 0) || (newMaterial.b > 1)))
            {
                printf("Material color must be between 0 and 1\n");
                return cleanExit(1);
            }

            // add the material to the array
            materialIndex++;
            materials[materialIndex] = newMaterial;
        }
        else if (strcmp(buffer, "sphere") == 0)
        {
            // create the new sphere
            EllipsoidType newSphere;
            fscanf(fptr, "%f", &newSphere.center.x);
            fscanf(fptr, "%f", &newSphere.center.y);
            fscanf(fptr, "%f", &newSphere.center.z);

            float radius;
            fscanf(fptr, "%f", &radius);
            newSphere.radius.x = radius;
            newSphere.radius.y = radius;
            newSphere.radius.z = radius;

            // ensure the sphere has size
            if (radius <= 0)
            {
                printf("Sphere radius must be greater than 0\n");
                return cleanExit(1);
            }

            // make sure a material was assigned
            if (materialIndex == 0)
            {
                printf("Must have at least one mtlcolor before a sphere.\n");
                return cleanExit(1);
            }
            newSphere.m = materialIndex;

            // add the sphere to the array
            ellipsoidIndex++;
            ellipsoids[ellipsoidIndex] = newSphere;
        }
        else if (strcmp(buffer, "ellipse") == 0)
        {
            // create the new ellipsoid
            EllipsoidType newEllipsoid;
            fscanf(fptr, "%f", &newEllipsoid.center.x);
            fscanf(fptr, "%f", &newEllipsoid.center.y);
            fscanf(fptr, "%f", &newEllipsoid.center.z);

            fscanf(fptr, "%f", &newEllipsoid.radius.x);
            fscanf(fptr, "%f", &newEllipsoid.radius.y);
            fscanf(fptr, "%f", &newEllipsoid.radius.z);

            // ensure the ellipsoid has size
            if (newEllipsoid.radius.x <= 0)
            {
                printf("Ellipsoid radii must be greater than 0\n");
                return cleanExit(1);
            }
            if (newEllipsoid.radius.y <= 0)
            {
                printf("Ellipsoid radii must be greater than 0\n");
                return cleanExit(1);
            }
            if (newEllipsoid.radius.z <= 0)
            {
                printf("Ellipsoid radii must be greater than 0\n");
                return cleanExit(1);
            }

            // make sure a material was assigned
            if (materialIndex == 0)
            {
                printf("Must have at least one mtlcolor before an ellipsoid.\n");
                return cleanExit(1);
            }
            newEllipsoid.m = materialIndex;

            // add the sphere to the array
            ellipsoidIndex++;
            ellipsoids[ellipsoidIndex] = newEllipsoid;
        }
    }

    fclose(fptr);

    /* ========================= ERROR CHECKING ========================= */

    // check for missing eye parameter
    if (!foundEye)
    {
        printf("Missing eye parameter\n");
        return cleanExit(1);
    }

    // check that viewdir has length
    if (length(&viewdir) <= 0.0)
    {
        printf("viewdir must have length\n");
        return cleanExit(1);
    }

    // check that updir has length
    if (length(&updir) <= 0.0)
    {
        printf("updir must have length\n");
        return cleanExit(1);
    }
    // normalize updir
    normalize(&updir);

    // check that 0 < hfov < 360
    if (((hfov <= 0) || (hfov >= 180)) && !parallelViewEnabled)
    {
        printf("hfov must be between 0 and 180 (exclusive)\n");
        return cleanExit(1);
    }

    // check for positive frustum width
    if (parallelViewEnabled && (frustumWidth <= 0))
    {
        printf("frustum width must be greater than 0\n");
        return cleanExit(1);
    }

    // check that imsize is greater than 1 pixel
    if ((1 >= imgHeight) || (1 >= imgWidth))
    {
        printf("Image width and height must be more than 1 pixel\n");
        return cleanExit(1);
    }

    // check that background colors are between 0 and 1
    if (
        ((bkgcolor.r < 0) || (bkgcolor.r > 1)) ||
        ((bkgcolor.g < 0) || (bkgcolor.g > 1)) ||
        ((bkgcolor.b < 0) || (bkgcolor.b > 1)))
    {
        printf("Colors must be between 0 and 1 (inclusive)\n");
        return cleanExit(1);
    }

    /* ========================= CALCULATE VARIABLES ========================= */

    // create a normalized viewingdir vector
    Vec3 w = viewdir;
    normalize(&w);

    // u is the cross product of viewdir and updir
    Vec3 u = w;
    cross(&u, &updir);

    // normalize u
    if (length(&u) <= 0.001)
    {
        printf("the viewing and up directions are too close to parallel\n");
        return cleanExit(1);
    }
    normalize(&u);

    // v is the cross product of u and viewdir
    Vec3 v = u;
    cross(&v, &w);

    float viewWidth, viewHeight;

    Vec3 ul, ur, ll, lr;
    Vec3 hChange, vChange;

    if (!parallelViewEnabled)
    {
        /* ========================= PERSPECTIVE PROJECTION ========================= */

        // calculate viewing window size
        viewWidth = 2 * tanf((hfov * (M_PI / 180.0)) / 2.0);
        viewHeight = viewWidth / ((float)imgWidth / (float)imgHeight);

        // calculate corners of viewing window
        ul.x = eye.x + w.x - (viewWidth / 2) * u.x + (viewHeight / 2) * v.x;
        ul.y = eye.y + w.y - (viewWidth / 2) * u.y + (viewHeight / 2) * v.y;
        ul.z = eye.z + w.z - (viewWidth / 2) * u.z + (viewHeight / 2) * v.z;

        ur.x = eye.x + w.x + (viewWidth / 2) * u.x + (viewHeight / 2) * v.x;
        ur.y = eye.y + w.y + (viewWidth / 2) * u.y + (viewHeight / 2) * v.y;
        ur.z = eye.z + w.z + (viewWidth / 2) * u.z + (viewHeight / 2) * v.z;

        ll.x = eye.x + w.x - (viewWidth / 2) * u.x - (viewHeight / 2) * v.x;
        ll.y = eye.y + w.y - (viewWidth / 2) * u.y - (viewHeight / 2) * v.y;
        ll.z = eye.z + w.z - (viewWidth / 2) * u.z - (viewHeight / 2) * v.z;

        lr.x = eye.x + w.x + (viewWidth / 2) * u.x - (viewHeight / 2) * v.x;
        lr.y = eye.y + w.y + (viewWidth / 2) * u.y - (viewHeight / 2) * v.y;
        lr.z = eye.z + w.z + (viewWidth / 2) * u.z - (viewHeight / 2) * v.z;

        // calculate the horizontal and vertical offset per pixel
        hChange.x = (ur.x - ul.x) / (imgWidth - 1);
        hChange.y = (ur.y - ul.y) / (imgWidth - 1);
        hChange.z = (ur.z - ul.z) / (imgWidth - 1);

        vChange.x = (ll.x - ul.x) / (imgHeight - 1);
        vChange.y = (ll.y - ul.y) / (imgHeight - 1);
        vChange.z = (ll.z - ul.z) / (imgHeight - 1);
    }
    else
    {
        /* ========================= PARALLEL PROJECTION ========================= */

        frustumHeight = frustumWidth / ((float)imgWidth / (float)imgHeight);

        // calculate corners of viewing window
        ul.x = eye.x - (frustumWidth / 2) * u.x + (frustumHeight / 2) * v.x;
        ul.y = eye.y - (frustumWidth / 2) * u.y + (frustumHeight / 2) * v.y;
        ul.z = eye.z - (frustumWidth / 2) * u.z + (frustumHeight / 2) * v.z;

        ur.x = eye.x + (frustumWidth / 2) * u.x + (frustumHeight / 2) * v.x;
        ur.y = eye.y + (frustumWidth / 2) * u.y + (frustumHeight / 2) * v.y;
        ur.z = eye.z + (frustumWidth / 2) * u.z + (frustumHeight / 2) * v.z;

        ll.x = eye.x - (frustumWidth / 2) * u.x - (frustumHeight / 2) * v.x;
        ll.y = eye.y - (frustumWidth / 2) * u.y - (frustumHeight / 2) * v.y;
        ll.z = eye.z - (frustumWidth / 2) * u.z - (frustumHeight / 2) * v.z;

        lr.x = eye.x + (frustumWidth / 2) * u.x - (frustumHeight / 2) * v.x;
        lr.y = eye.y + (frustumWidth / 2) * u.y - (frustumHeight / 2) * v.y;
        lr.z = eye.z + (frustumWidth / 2) * u.z - (frustumHeight / 2) * v.z;

        // calculate the horizontal and vertical offset per pixel
        hChange.x = (ur.x - ul.x) / (imgWidth - 1);
        hChange.y = (ur.y - ul.y) / (imgWidth - 1);
        hChange.z = (ur.z - ul.z) / (imgWidth - 1);

        vChange.x = (ll.x - ul.x) / (imgHeight - 1);
        vChange.y = (ll.y - ul.y) / (imgHeight - 1);
        vChange.z = (ll.z - ul.z) / (imgHeight - 1);
    }

    // create image matrix
    Vec3 image[imgHeight][imgWidth];

    /* ========================= DEFINING PIXEL COLORS ========================= */

    for (int i = 0; i < imgHeight; i++)
    {
        for (int j = 0; j < imgWidth; j++)
        {
            // calculate the point on the viewing window the ray will shoot
            Vec3 pointThrough = {
                ul.x + (i * vChange.x) + (j * hChange.x),
                ul.y + (i * vChange.y) + (j * hChange.y),
                ul.z + (i * vChange.z) + (j * hChange.z)};

            RayType curRay;

            if (parallelViewEnabled == 1)
            {
                // calculate the parallel view
                curRay.pos = pointThrough;

                Vec3 parallelView = {
                    w.x,
                    w.y,
                    w.z};

                curRay.dir = parallelView;
            }
            else
            {
                // calculate the perspective view
                curRay.pos = eye;

                float rayLength = sqrtf(
                    pow(pointThrough.x - eye.x, 2.0) +
                    pow(pointThrough.y - eye.y, 2.0) +
                    pow(pointThrough.z - eye.z, 2.0));

                Vec3 perspectiveView = {
                    (pointThrough.x - eye.x) / rayLength,
                    (pointThrough.y - eye.y) / rayLength,
                    (pointThrough.z - eye.z) / rayLength,
                };

                curRay.dir = perspectiveView;
            }

            image[i][j] = *traceRay(&curRay);
        }
    }

    /* ========================= OUTPUT TO FILE ========================= */

    // find where the dot is
    char *dotPosition = strrchr(argv[1], '.');

    // create a new file name and name it accordingly
    char newFileName[strlen(argv[1]) + strlen(".ppm") + 1];

    if (dotPosition != NULL || dotPosition == argv[1])
    {
        strncpy(newFileName, argv[1], dotPosition - argv[1]);
        strcpy(newFileName + (dotPosition - argv[1]), ".ppm");
    }
    else
    {
        strcpy(newFileName, argv[1]);
        strcat(newFileName, ".ppm");
    }

    // open the new file for writing
    FILE *outputFile = fopen(newFileName, "w");
    if (fptr == NULL)
    {
        printf("Couldn't create file %s\n", argv[1]);
        return cleanExit(1);
    }

    // create the header for the ppm
    fprintf(outputFile, "P3 %d %d %d\n", imgWidth, imgHeight, 255);

    // write the pixel contents to the file
    for (int i = 0; i < imgHeight; i++)
    {
        for (int j = 0; j < imgWidth; j++)
        {
            fprintf(outputFile, "%d %d %d\n",
                    (int)(image[i][j].r * 255),
                    (int)(image[i][j].g * 255),
                    (int)(image[i][j].b * 255));
        }
    }

    fclose(outputFile);
    return cleanExit(0);
}

Vec3 *traceRay(RayType *ray)
{
    float minimumDistance = MAXFLOAT;
    EllipsoidType currentEllipsoid;

    for (int i = 0; i <= ellipsoidIndex; i++)
    {

        // calculate components of distance solution
        float A = (powf(ray->dir.x / ellipsoids[i].radius.x, 2.0) +
                   powf(ray->dir.y / ellipsoids[i].radius.y, 2.0) +
                   powf(ray->dir.z / ellipsoids[i].radius.z, 2.0));

        float B = 2.0 * ((((ray->pos.x - ellipsoids[i].center.x) * ray->dir.x) /
                          powf(ellipsoids[i].radius.x, 2.0)) +
                         (((ray->pos.y - ellipsoids[i].center.y) * ray->dir.y) /
                          powf(ellipsoids[i].radius.y, 2.0)) +
                         (((ray->pos.z - ellipsoids[i].center.z) * ray->dir.z) /
                          powf(ellipsoids[i].radius.z, 2.0)));

        float C = (powf((ray->pos.x - ellipsoids[i].center.x) / ellipsoids[i].radius.x, 2.0) +
                   powf((ray->pos.y - ellipsoids[i].center.y) / ellipsoids[i].radius.y, 2.0) +
                   powf((ray->pos.z - ellipsoids[i].center.z) / ellipsoids[i].radius.z, 2.0) - 1);

        // if discriminant is less than 0, ray does not hit object
        float discriminant = powf(B, 2.0) - (4 * A * C);
        if (discriminant < 0)
        {
            continue;
        }

        float plusT = (-B + sqrtf(discriminant)) / (2 * A);
        float minusT = (-B - sqrtf(discriminant)) / (2 * A);

        if ((plusT > 0) && (plusT < minimumDistance))
        {
            minimumDistance = plusT;
            currentEllipsoid = ellipsoids[i];
        }

        if ((minusT > 0) && (minusT < minimumDistance))
        {
            minimumDistance = minusT;
            currentEllipsoid = ellipsoids[i];
        }
    }

    // check for unchanged value
    if (minimumDistance == MAXFLOAT)
    {
        return &materials[0];
    }
    return shadeRay(&currentEllipsoid);
}

Vec3 *shadeRay(EllipsoidType *closestEllipsoid)
{
    return &materials[closestEllipsoid->m];
}

int cleanExit(int value)
{
    free(ellipsoids);
    free(materials);
    return value;
}

