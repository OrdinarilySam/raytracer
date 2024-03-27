#include "main.h"

int main(int argc, char* argv[]) {
  Scene* scene = malloc(sizeof(Scene));

  if (scene == NULL) {
    printf("Error: malloc failed\n");
    exit(1);
  }

  if (argc != 2) {
    printf("Usage: %s <scene file>\n", argv[0]);
    exit(1);
  }

  readScene(scene, argv[1]);

  traceRays(scene);

  printImage(scene, argv[1]);

  // fclose(scene->output);
  freeAll(scene);
  printf("Done\n\n");
  return 0;
}

void printImage(Scene* scene, char* filename) {
  FILE* output =
      createOutputFile(filename, scene->imgsize.width, scene->imgsize.height);
  
  if (output == NULL) {
    printf("Error: could not create output file\n");
    freeAll(scene);
    exit(1);
  }

  fprintf(output, "P3\n");
  fprintf(output, "%d %d\n", scene->imgsize.width, scene->imgsize.height);
  fprintf(output, "255\n");

  for (int i = 0; i < scene->imgsize.height; i++) {
    for (int j = 0; j < scene->imgsize.width; j++) {
      fprintf(output, "%d %d %d ", 
              (int)(scene->pixels[i][j].r * 255),
              (int)(scene->pixels[i][j].g * 255),
              (int)(scene->pixels[i][j].b * 255));
    }
    fprintf(output, "\n");
  }

  fclose(output);
}
