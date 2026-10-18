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

#ifndef BLOCK_ALLOC_BLOCK_ALLOC_H_
#define BLOCK_ALLOC_BLOCK_ALLOC_H_

#define LOW_BIT_IDX_TOTAL 16 /* Size of calculated table */
#define BITMAP_SIZE       4  /* Bit map size */
#define MAX_BLOCKS        32 /* Maximal blocks count */
#define MAX_BLOCKS_POW    5  /* Power of 2 for maximal blocks */
#define BLOCK_SIZE        16 /* Block size */
#define BLOCK_SIZE_POW    4  /* Power of block size */

typedef unsigned char UCharT;

/* Calculated table for fast work. */
static const UCharT kLowBitIdx[LOW_BIT_IDX_TOTAL] = {
  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4
};

/* Memory chunk */
typedef struct Plate {
  UCharT* data;
  UCharT bitmap[BITMAP_SIZE];
} Plate;

/* Alignment union for Plate. */
typedef union PlateStorage {
  double d;
  long l;
  void* p;
  void (*f)(void);
  UCharT array[MAX_BLOCKS << BLOCK_SIZE_POW];
} PlateStorage;

/* Init Plate */
void InitPlate(Plate*, void*);

/* Allocate block in Plate */
void* PlateAlloc(Plate*);

/* Free block, that was allocated by PlateAlloc */
void PlateFree(Plate*, void*);

#endif /* BLOCK_ALLOC_BLOCK_ALLOC_H_ */

#ifdef BLOCK_ALLOC_SOURCE

static int GetIdxByByte(UCharT byte) {
  UCharT low = byte & 0x0F;
  if (low != 0x0F) {
    return kLowBitIdx[low];
  }

  return 4 + kLowBitIdx[byte >> 4];
}

#include <stddef.h>

void InitPlate(Plate* plate, void* data) {
  size_t i = 0;
  plate->data = (UCharT*)data;

  for (; i < BITMAP_SIZE; i++) {
    plate->bitmap[i] = 0x00;
  }
}

void* PlateAlloc(Plate* plate) {
  size_t cur_bitmap_idx = 0;
  int bit_idx = -1;
  size_t offset = 0;

  for (; cur_bitmap_idx < BITMAP_SIZE; cur_bitmap_idx++) {
    if (plate->bitmap[cur_bitmap_idx] != 0xFF) {
      bit_idx = GetIdxByByte(plate->bitmap[cur_bitmap_idx]);
      break;
    }
  }

  if (bit_idx >= 8 || bit_idx == -1) {
    return NULL;
  }

  plate->bitmap[cur_bitmap_idx] |= (1 << bit_idx);

  offset = (cur_bitmap_idx << 3) + (size_t)bit_idx;

  return (void*)&plate->data[offset << 4];
}

void PlateFree(Plate* plate, void* ptr) {
  ptrdiff_t offset = 0;
  size_t offset_bit = 0;
  int bitmap_byte = 0;
  int bitmap_bit = 0;

  if (ptr == NULL) {
    return;
  }

  if ((UCharT*)ptr < plate->data) {
    return;
  }

  offset = (UCharT*)ptr - plate->data;
  offset_bit = (size_t)(offset >> 4);

  if (offset & 15) {
    return;
  }

  if ((UCharT*)ptr >= (plate->data + (BLOCK_SIZE << MAX_BLOCKS_POW))) {
    return;
  }
  
  bitmap_byte = (int)(offset_bit >> 3);
  bitmap_bit = offset_bit & 7;

  if (bitmap_byte >= BITMAP_SIZE) {
    return;
  }

  plate->bitmap[bitmap_byte] &= ~(1 << bitmap_bit);
}

#undef BLOCK_ALLOC_SOURCE

#endif /* BLOCK_ALLOC_SOURCE */
