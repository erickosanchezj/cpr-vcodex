#include "util/BmpMetadataCache.h"

#include <Bitmap.h>
#include <HalStorage.h>

#include <vector>

namespace {
constexpr size_t MAX_CACHE_ENTRIES = 32;

struct CacheEntry {
  std::string path;
  BmpMetadata metadata;
};

std::vector<CacheEntry>& getCacheEntries() {
  static std::vector<CacheEntry> entries;
  return entries;
}
}  // namespace

bool BmpMetadataCache::get(const std::string& path, BmpMetadata& metadata) {
  metadata = {};
  if (path.empty()) {
    return false;
  }

  auto& entries = getCacheEntries();
  for (auto it = entries.begin(); it != entries.end(); ++it) {
    if (it->path != path) {
      continue;
    }

    metadata = it->metadata;
    if (it != entries.begin()) {
      CacheEntry entry = *it;
      entries.erase(it);
      entries.insert(entries.begin(), std::move(entry));
    }
    return metadata.valid;
  }

  FsFile file;
  if (!Storage.openFileForRead("BMC", path, file)) {
    return false;
  }

  Bitmap bitmap(file);
  if (bitmap.parseHeaders() != BmpReaderError::Ok) {
    file.close();
    return false;
  }

  metadata.valid = true;
  metadata.width = static_cast<uint16_t>(bitmap.getWidth());
  metadata.height = static_cast<uint16_t>(bitmap.getHeight());
  file.close();

  entries.insert(entries.begin(), CacheEntry{path, metadata});
  if (entries.size() > MAX_CACHE_ENTRIES) {
    entries.pop_back();
  }
  return true;
}
