#pragma once

#ifdef __cplusplus
#include_next <Arduino.h>

#include <cstring>

class __FlashStringHelper;

#ifndef F
#define F(str) reinterpret_cast<const __FlashStringHelper*>(str)
#endif

#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(reinterpret_cast<const uint8_t*>(addr)))
#endif

#ifndef pgm_read_word
#define pgm_read_word(addr) (*(reinterpret_cast<const uint16_t*>(addr)))
#endif

#ifndef pgm_read_dword
#define pgm_read_dword(addr) (*(reinterpret_cast<const uint32_t*>(addr)))
#endif

#ifndef pgm_read_ptr
#define pgm_read_ptr(addr) (*(reinterpret_cast<const void* const*>(addr)))
#endif

#ifndef memcpy_P
#define memcpy_P(dest, src, len) std::memcpy((dest), (src), (len))
#endif

class Print;
class Printable {
 public:
  virtual ~Printable() = default;
  virtual size_t printTo(Print& p) const = 0;
};

#else
#define PROGMEM
#define ICACHE_RODATA_ATTR
#define IRAM_ATTR
#define DRAM_ATTR
#define RTC_NOINIT_ATTR
#endif
