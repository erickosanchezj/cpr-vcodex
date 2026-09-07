#pragma once

#include <Arduino.h>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

#define MALLOC_CAP_8BIT 0x00000001
#define MALLOC_CAP_DEFAULT 0x00000002

inline uint32_t heap_caps_get_largest_free_block(uint32_t /*caps*/) {
  return ESP.getMaxAllocHeap();
}

inline void* heap_caps_malloc(size_t size, uint32_t /*caps*/) {
  return std::malloc(size);
}

inline void heap_caps_free(void* ptr) {
  std::free(ptr);
}
