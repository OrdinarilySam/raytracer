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

  fclose(scene->output);
  freeAll(scene);
  printf("Done\n\n");
  return 0;
}
