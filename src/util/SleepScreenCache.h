#pragma once

#include <cstdint>
#include <string>

class GfxRenderer;

// Cache rendered BW sleep framebuffers to SD to avoid re-rendering static BMP sleep screens.
// Greyscale sleep images are intentionally skipped because they require the extra grey buffer path.
class SleepScreenCache {
 public:
  static bool load(GfxRenderer& renderer, const std::string& sourcePath);
  static void save(const GfxRenderer& renderer, const std::string& sourcePath);
  static int invalidateAll();

 private:
  static constexpr const char* CACHE_DIR = "/.crosspoint/sleep_cache";

  static uint32_t hashKey(const std::string& sourcePath, uint32_t fileSize);
  static std::string getCachePath(const std::string& sourcePath, uint32_t fileSize);
};
