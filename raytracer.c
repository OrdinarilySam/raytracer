#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "raytracer.h"

// todo: find out how to dynamically size an array


int materialIndex = 0;
int sphereIndex = -1;
ColorType *materials;
SphereType *spheres;


int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ./<program> <inputfile>\n");
        return 1;
    }

    FILE *fptr = fopen(argv[1], "r");
    if (fptr == NULL) {
        printf("Couldn't open file %s\n", argv[1]);
        return 1;
    }

    int materialsSize = 100;
    materials = malloc(100 * sizeof(ColorType));
    if (materials == NULL) {
        printf("Failed to initialize a material array. Exiting...\n");
        return 1;
    }

    int spheresSize = 200;
    spheres = malloc(200 * sizeof(SphereType));
    if (spheres == NULL) {
        printf("Failed to initialize an object array. Exiting...\n");
        return 1;
    }

    CoordType eye;
    CoordType viewdir;
    CoordType updir;
    float hfov;
    int imgWidth;
    int imgHeight;
    ColorType bkgcolor;

    short parallelViewEnabled = 0;

    short foundEye = 0;
    short foundViewdir = 0;
    short foundUpdir = 0; 
    short foundHfov = 0;
    short foundImsize = 0;
    short foundBkgcolor = 0;

    int d = 10;

    char buffer[100];
    while (fscanf(fptr, "%s", buffer) != EOF) {
        if (strcmp(buffer, "eye") == 0) {
            fscanf(fptr, "%f", &eye.x);
            fscanf(fptr, "%f", &eye.y);
            fscanf(fptr, "%f", &eye.z);
            foundEye = 1;
        } else if (strcmp(buffer, "parallel") == 0) {
            parallelViewEnabled = 1;
        } else if (strcmp(buffer, "distance") == 0) {
            fscanf(fptr, "%d", &d);
        } else if (strcmp(buffer, "viewdir") == 0) {
            fscanf(fptr, "%f", &viewdir.x);
            fscanf(fptr, "%f", &viewdir.y);
            fscanf(fptr, "%f", &viewdir.z);
            foundViewdir = 1;
        } else if (strcmp(buffer, "updir") == 0) {
            fscanf(fptr, "%f", &updir.x);
            fscanf(fptr, "%f", &updir.y);
            fscanf(fptr, "%f", &updir.z);
            foundUpdir = 1;
        } else if (strcmp(buffer, "hfov") == 0) {
            fscanf(fptr, "%f", &hfov);
            foundHfov = 1;
        } else if (strcmp(buffer, "imsize") == 0) {
            fscanf(fptr, "%d", &imgWidth);
            fscanf(fptr, "%d", &imgHeight);
            foundImsize = 1;
        } else if (strcmp(buffer, "bkgcolor") == 0) {
            fscanf(fptr, "%f", &bkgcolor.r);
            fscanf(fptr, "%f", &bkgcolor.g);
            fscanf(fptr, "%f", &bkgcolor.b);
            materials[0] = bkgcolor;
            foundBkgcolor = 1;
        } else if (strcmp(buffer, "mtlcolor") == 0) {
            ColorType newMaterial;
            fscanf(fptr, "%f", &newMaterial.r);
            fscanf(fptr, "%f", &newMaterial.g);
            fscanf(fptr, "%f", &newMaterial.b);

            // printf("New Material: %f %f %f\n", newMaterial.r, newMaterial.g, newMaterial.b);

            if (
                ((newMaterial.r < 0) || (newMaterial.r > 1)) ||
                ((newMaterial.g < 0) || (newMaterial.g > 1)) ||
                ((newMaterial.b < 0) || (newMaterial.b > 1))
            ) {
                printf("Material color must be between 0 and 1\n");
                free(spheres);
                free(materials);
                return 1;
            }

            materialIndex++;
            if (materialIndex >= materialsSize) {
                materials = realloc(materials, materialsSize * 2);
                if (materials == NULL) {
                    printf("Failed to resize materials array.\n");
                    free(spheres);
                    return 1;
                }
                materialsSize *= 2;
            }
            
            materials[materialIndex] = newMaterial;
            
        } else if (strcmp(buffer, "sphere") == 0) {
            SphereType newSphere;
            fscanf(fptr, "%f", &newSphere.center.x);
            fscanf(fptr, "%f", &newSphere.center.y);
            fscanf(fptr, "%f", &newSphere.center.z);
            fscanf(fptr, "%f", &newSphere.r);

            if (newSphere.r <= 0) {
                printf("Sphere radius must be greater than 0\n");
                free(spheres);
                free(materials);
                return 1;
            }

            if (materialIndex == 0) {
                printf("Must have at least one mtlcolor before a sphere.\n");
                free(spheres);
                free(materials);
                return 1;
            }
            newSphere.m = materialIndex;

            sphereIndex++;
            if (sphereIndex >= spheresSize) {
                spheres = realloc(spheres, spheresSize * 2);
                if (spheres== NULL) {
                    printf("Failed to resize spheres array.\n");
                    free(materials);
                    return 1;
                }
                spheresSize *= 2;
            }

            spheres[sphereIndex] = newSphere;
        }
    }

    fclose(fptr);

    // ? print statements for debugging
    printf("Eye: %f %f %f\n", eye.x, eye.y, eye.z);
    printf("Viewdir: %f %f %f\n", viewdir.x, viewdir.y, viewdir.z);
    printf("Updir: %f %f %f\n", updir.x, updir.y, updir.z);
    printf("Hfov: %f\n", hfov);
    printf("Imsize: %d %d\n", imgWidth, imgHeight);
    printf("Background color: %f %f %f\n", materials[0].r, materials[0].g, materials[0].b);

    // for (int i = 1; i <= materialIndex; i++) {
    //     printf("Material: %f %f %f\n", materials[i].r, materials[i].g, materials[i].b);
    // }

    // for (int i = 0; i <= sphereIndex; i++) {
    //     printf("Sphere: %f %f %f %f mat: %d\n", 
    //         spheres[i].center.x, spheres[i].center.y, spheres[i].center.z, spheres[i].r, spheres[i].m);
    // }

    // TODO ERR CHECK
    // Assume correct input for now, use bools to test that any given required input was answered
    // use sentinal values where possible (-1 or non allowed value)
    // error checking:
        // viewdir is normalized
        // updir is normalized
        // hfov is greater than 0 and less than 360
        // imsize is greater than 1x1
        // bkgcolor includes only values 0 <= color <= 1
        // same for material color
        // sphere radius is greater than 0

    // check for missing field
    if (
        !foundEye || !foundViewdir || !foundUpdir || 
        !foundHfov || !foundImsize || !foundBkgcolor
    ) {
        printf("Missing at least one required parameter\n");
        free(spheres);
        free(materials);
        return 1;
    }

    // check that updir is normalized

    // check that 0 < hfov < 360
    if ((hfov <= 0) || (hfov >= 360)) {
        printf("hfov must be between 0 and 360 (exclusive)\n");
        free(spheres);
        free(materials);
        return 1;
    }

    // check that imsize is greater than 1 pixel
    if ((1 >= imgHeight) || (1 >= imgWidth)) {
        printf("Image width and height must be more than 1 pixel\n");
        free(spheres);
        free(materials);
        return 1;
    }

    // check that background colors are between 0 and 1
    if (
        ((bkgcolor.r < 0) || (bkgcolor.r > 1)) ||
        ((bkgcolor.g < 0) || (bkgcolor.g > 1)) ||
        ((bkgcolor.b < 0) || (bkgcolor.b > 1))
    ) {
        printf("Colors must be between 0 and 1 (inclusive)\n");
        free(spheres);
        free(materials);
        return 1;
    }

    
    // TODO ERR CHECK
    // make sure viewdir and updir aren't parallel or close to parallel
    // perhaps calculate an n to be the unit vector in the viewing direction
    // viewdir doesn't need to be normalized
        // calculate n = normalized viewdir
        // w = -viewdir

    float viewdirLength = sqrtf(
        powf(viewdir.x, 2.0) + 
        powf(viewdir.y, 2.0) + 
        powf(viewdir.z, 2.0)
    );
    CoordType n = {
        viewdir.x /= viewdirLength,
        viewdir.y /= viewdirLength,
        viewdir.z /= viewdirLength
    };

    // calculate u and v
    // cross product of viewdir and updir
    CoordType u = {
        (n.y * updir.z) - (n.z * updir.y),
        (n.z * updir.x) - (n.x * updir.z),
        (n.x * updir.y) - (n.y * updir.x)
    };
    float uLength = sqrtf(powf(u.x, 2.0) + powf(u.y, 2.0) + powf(u.z, 2.0));
    u.x /= uLength;
    u.y /= uLength;
    u.z /= uLength;

    // cross product of u and viewdir
    CoordType v = {
        (u.y * n.z) - (u.z * n.y),
        (u.z * n.x) - (u.x * n.z),
        (u.x * n.y) - (u.y * n.x),
    };

    // ? print statements for debugging
    // printf("u: %f %f %f\n", u.x, u.y, u.z);
    // printf("v: %f %f %f\n", v.x, v.y, v.z);

    // calculate the width and height of the viewing window
    // calculate the four corners

    // account for radian values
    float viewWidth = 2 * d * tanf((hfov * (M_PI / 180.0)) / 2.0);
    float viewHeight = viewWidth / ((float)imgWidth / (float)imgHeight);

    CoordType ul = {
        eye.x + (d * n.x) - ((viewWidth / 2) * u.x) + ((viewHeight / 2) * v.x),
        eye.y + (d * n.y) - ((viewWidth / 2) * u.y) + ((viewHeight / 2) * v.y),
        eye.z + (d * n.z) - ((viewWidth / 2) * u.z) + ((viewHeight / 2) * v.z)
    };
    CoordType ur = {
        eye.x + (d * n.x) + ((viewWidth / 2) * u.x) + ((viewHeight / 2) * v.x),
        eye.y + (d * n.y) + ((viewWidth / 2) * u.y) + ((viewHeight / 2) * v.y),
        eye.z + (d * n.z) + ((viewWidth / 2) * u.z) + ((viewHeight / 2) * v.z)
    };
    CoordType ll = {
        eye.x + (d * n.x) - ((viewWidth / 2) * u.x) - ((viewHeight / 2) * v.x),
        eye.y + (d * n.y) - ((viewWidth / 2) * u.y) - ((viewHeight / 2) * v.y),
        eye.z + (d * n.z) - ((viewWidth / 2) * u.z) - ((viewHeight / 2) * v.z)
    };
    CoordType lr = {
        eye.x + (d * n.x) + ((viewWidth / 2) * u.x) - ((viewHeight / 2) * v.x),
        eye.y + (d * n.y) + ((viewWidth / 2) * u.y) - ((viewHeight / 2) * v.y),
        eye.z + (d * n.z) + ((viewWidth / 2) * u.z) - ((viewHeight / 2) * v.z)
    };


    CoordType hChange = {
        (ur.x - ul.x) / (imgWidth - 1),
        (ur.y - ul.y) / (imgWidth - 1),
        (ur.z - ul.z) / (imgWidth - 1)
    };

    CoordType vChange = {
        (ll.x - ul.x) / (imgWidth - 1),
        (ll.y - ul.y) / (imgWidth - 1),
        (ll.z - ul.z) / (imgWidth - 1)
    };

    ColorType image[imgHeight][imgWidth];

    for (int i = 0; i < imgHeight; i++) {
        for (int j = 0; j < imgWidth; j++) {
            // because we shoot the ray from the eye,
            // the distance to the viewing window is negligible
            CoordType pointThrough = {
                ul.x + (i * vChange.x) + (j * hChange.x),
                ul.y + (i * vChange.y) + (j * hChange.y),
                ul.z + (i * vChange.z) + (j * hChange.z)
            };
            float rayLength = sqrtf(
                pow(pointThrough.x - eye.x, 2.0) +
                pow(pointThrough.y - eye.y, 2.0) +
                pow(pointThrough.z - eye.z, 2.0)
            );

            // for parallel projection use the pointThrough as the starting location
            // and the viewdir as the direction
            RayType curRay;
            if (parallelViewEnabled == 1) {
                curRay.pos = pointThrough;
                CoordType parallelView = {
                    n.x,
                    n.y,
                    n.z
                };
                curRay.dir = parallelView;
            } else {
                curRay.pos = eye;
                CoordType perspectiveView = {
                    (pointThrough.x - eye.x) / rayLength,
                    (pointThrough.y - eye.y) / rayLength,
                    (pointThrough.z - eye.z) / rayLength,
                };
                curRay.dir = perspectiveView;

            }
            image[i][j] = traceRay(curRay);

        }
    }

    char fileName[100];
    sprintf(fileName, "%s.ppm", argv[1]);

    FILE *outputFile = fopen(fileName, "w");
    if (fptr == NULL) {
        printf("Couldn't create file %s\n", argv[1]);
        free(materials);
        free(spheres);
        return 1;
    }

    fprintf(outputFile, "P3 %d %d %d\n", imgWidth, imgHeight, 255);

    for (int i = 0; i < imgHeight; i++) {
        for (int j = 0; j < imgWidth; j++) {
            fprintf(outputFile, "%d %d %d\n", 
                (int) (image[i][j].r * 255),
                (int) (image[i][j].g * 255),
                (int) (image[i][j].b * 255)
            );
        }
    }

    fclose(outputFile);
    free(spheres);
    free(materials);
    return 0;
}


ColorType traceRay(RayType ray) {
    // TODO
    // keep array of spheres that are hit by ray as well as their distances

    float minimumDistance = -1;
    ColorType currentColor = materials[0];

    for (int i = 0; i <= sphereIndex; i++) {
        
        float B = 2.0 * (
            (ray.dir.x * (ray.pos.x - spheres[i].center.x)) + 
            (ray.dir.y * (ray.pos.y - spheres[i].center.y)) + 
            (ray.dir.z * (ray.pos.z - spheres[i].center.z)) 
        );

        float C = (
            powf(ray.pos.x - spheres[i].center.x, 2.0) +
            powf(ray.pos.y - spheres[i].center.y, 2.0) +
            powf(ray.pos.z - spheres[i].center.z, 2.0) -
            powf(spheres[i].r, 2.0)
        );

        if ((powf(B, 2.0) - (4 * C)) < 0) {
            continue;
        }


        // todo outsource logic to shadeRay
        float plusT = (-B + sqrtf(powf(B, 2) - (4 * C))) / 2;
        float minusT = (-B + sqrtf(powf(B, 2) - (4 * C))) / 2;

        if ((minimumDistance == -1) || ((plusT > 0) && (plusT < minimumDistance))) {
            // printf("Switching color to %d\n", spheres[i].m);
            minimumDistance = plusT;
            currentColor = materials[spheres[i].m];
        }

        if ((minimumDistance == -1) || ((minusT > 0) && (minusT < minimumDistance))) {
            // printf("Switching color to %d\n", spheres[i].m);
            minimumDistance = minusT;
            currentColor = materials[spheres[i].m];
        }

        // if B^2 - 4C < 0, sphere not in view
        // if it's 0, ray grazes sphere
        // if positive, two intersections
    } 
    return currentColor;
}

// ColorType shadeRay(SphereType sphere[], SphereType *spheres, int numSpheres) {
//     // TODO
// }
