#define CRT_SOURCE
#define CRT_NOT_USE_NAMESPACE
#define CRT_IMPORT "unsafe/tools/hashmap.h"
#include "../include/libcrt/crt.h"

#include <stdio.h> 

int main(void) {
  hashmap* map = create_hashmap(16);
  const char* get = NULL;

  if (!map) {
    printf("Can't allocate memory\n");
    return 1;
  }

  if (hashmap_push(map, "example", "works") == 1) {
    printf("Can't push to the map\n");
    free_hashmap(map);
    return 1;
  }

  get = (const char*)hashmap_get(map, "example");
  if (get) {
    printf("map[\"example\"] = %s\n", get);
  } else {
    printf("Cannot get value\n");
    free_hashmap(map);
    return 1;
  }

  free_hashmap(map);
  return 0;
}
