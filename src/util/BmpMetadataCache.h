#pragma once

#include <cstdint>
#include <string>

struct BmpMetadata {
  bool valid = false;
  uint16_t width = 0;
  uint16_t height = 0;
};

namespace BmpMetadataCache {

bool get(const std::string& path, BmpMetadata& metadata);

}  // namespace BmpMetadataCache
