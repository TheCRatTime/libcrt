/*
 * Copyright (C) 2026 TheCRatTime
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HASHMAP_HASHMAP_H_
#define HASHMAP_HASHMAP_H_

#include <stddef.h>

#define HASHMAP_HASH_START 5381
#define HASHMAP_REHASH     0.75

typedef struct hashmap_node {
  const char* key;
  void* value;
  struct hashmap_node* next;
} hashmap_node;

typedef struct hashmap {
  hashmap_node* nodes;
  size_t used;
  size_t cap;
  unsigned char get_success;
  unsigned char rehashing;
} hashmap;

/* Init hashmap */
hashmap* create_hashmap(size_t cap);

/* Get element from MAP */
void* hashmap_get(hashmap* map, const char* key);

/* Set VAL to element from MAP */
void hashmap_set(hashmap* map, const char* key, void* val);

/* Push new element to MAP */
int hashmap_push(hashmap* map, const char* key, void* val);

/* Free hashmap */
void free_hashmap(hashmap* map);

#endif /* HASHMAP_HASHMAP_H_ */

#ifdef HASHMAP_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* hashmap_strdup(const char* str) {
  char* ret = NULL;
  
  if (!str) { return NULL; }

  ret = (char*)malloc(strlen(str)+1);
  if (!ret) { return NULL; }

  ret[strlen(str)] = '\0';

  memcpy(ret, str, strlen(str));

  return ret;
}

static size_t hashmap_get_hash(const char* key) {
  size_t hash = HASHMAP_HASH_START;

  if (!key) { return hash; }

  while (*key != '\0') {
    hash = ((hash << 5) + hash) + *key;
    key++;
  }

  return hash;
}

hashmap* create_hashmap(size_t cap) {
  hashmap* map = (hashmap*)malloc(sizeof(hashmap));
  if (!map) {
    return NULL;
  }

  map->nodes = (hashmap_node*)malloc(cap * sizeof(hashmap_node));
  if (!map->nodes) {
    free(map);
    return NULL;
  }

  memset(map->nodes, 0, cap * sizeof(hashmap_node));

  map->cap = cap;
  map->get_success = 0;
  map->rehashing = 0;
  map->used = 0;

  return map;
}

void* hashmap_get(hashmap* map, const char* key) {
  void* ret = NULL;
  hashmap_node* cur = NULL;
  
  map->get_success = 0;
  
  if (!map || !key) { return NULL; }

  cur = &map->nodes[hashmap_get_hash(key) % map->cap];
  if (cur->key == NULL) { return NULL; }

  while (cur) {
    if (!strcmp(key, cur->key)) {
      map->get_success = 1;
      return cur->value;
    }

    cur = cur->next;
  }

  return NULL;
}

void hashmap_set(hashmap* map, const char* key, void* val) {
  void* ret = NULL;
  hashmap_node* cur = NULL;
  
  if (!map || !key) { return; }

  cur = &map->nodes[hashmap_get_hash(key) % map->cap];
  if (cur->key == NULL) { return; }

  while (cur) {
    if (!strcmp(key, cur->key)) {
      cur->value = val;
      return;
    }

    cur = cur->next;
  }
}

static int hashmap_rehash(hashmap* map);

int hashmap_push(hashmap* map, const char* key, void* val) {
  hashmap_node* node = NULL;
  hashmap_node* last = NULL;
  int flag = 0;
  
  if (!map || !key) { return 1; }

  hashmap_get(map, key);
  if (map->get_success) {
    hashmap_set(map, key, val);
    return 0;
  }

  node = &map->nodes[hashmap_get_hash(key) % map->cap];

  if (node->key != NULL) {
    while (node->next != NULL) {
      node = node->next;
    }

    last = node;

    node = (hashmap_node*)malloc(sizeof(hashmap_node));
    if (!node) { return 1; }

    node->next = NULL;
    flag = 1;
  }
  
  node->key = hashmap_strdup(key);
  if (!node->key) { return 1; }
    
  node->value = val;
  map->used++;

  if (flag) { last->next = node; }
  
  if (((double)(map->used) / (double)(map->cap)) > HASHMAP_REHASH &&
      !map->rehashing) {
    return hashmap_rehash(map);
  }

  return 0;
}

void free_hashmap(hashmap* map) {
  size_t i = 0;
  for (; i < map->cap; i++) {
    hashmap_node* cur = map->nodes[i].next;
    
    free((void*)map->nodes[i].key);

    if (!cur) { continue; }

    while (cur) {
      hashmap_node* next = cur->next;
      free((void*)cur->key);
      free(cur);
      cur = next;
    }
  }

  free(map->nodes);
  free(map);
}

static int hashmap_rehash(hashmap* map) {
  size_t old_cap = 0;
  size_t i = 0;
  hashmap* new_map = NULL;
  
  if (!map) { return 1; }

  new_map = create_hashmap(map->cap << 1);
  if (!new_map) {
    return 1;
  }

  new_map->used = map->used;

  map->rehashing = 1;

  for (; i < map->cap; i++) {
    hashmap_node* cur = &map->nodes[i];
    while (cur) {
      if (hashmap_push(new_map, cur->key, cur->value)) {
        map->rehashing = 0;
        free_hashmap(new_map);
        return 1;
      }

      cur = cur->next;
    }
  }

  {
    size_t old_cap = map->cap;
    hashmap_node* old_nodes = map->nodes;
    size_t old_used = map->used;

    map->cap = new_map->cap;
    map->nodes = new_map->nodes;
    map->used = new_map->used;

    new_map->cap = old_cap;
    new_map->nodes = old_nodes;
    new_map->used = old_used;
  }

  map->rehashing = 0;

  free_hashmap(new_map);

  return 0;
}

#endif /* HASHMAP_SOURCE */
