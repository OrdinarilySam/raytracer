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

  for (int i = 0; i < scene->imgsize.height; i++) {
    for (int j = 0; j < scene->imgsize.width; j++) {
      traceray(scene, i, j);
    }
  }

  fclose(scene->output);
  free(scene);
  return 0;
}
