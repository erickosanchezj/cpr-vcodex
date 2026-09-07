#pragma once

#include <cstdint>

enum QrCodeEcc { ECC_LOW = 0, ECC_MEDIUM, ECC_QUARTILE, ECC_HIGH };

class QRCode {
 public:
  uint8_t size = 0;
};

inline uint32_t qrcode_getBufferSize(uint8_t version) {
  return static_cast<uint32_t>((version * 4 + 17) * (version * 4 + 17));
}

inline int8_t qrcode_initText(QRCode* qrcode, uint8_t* /*modules*/, uint8_t version, QrCodeEcc /*ecc*/,
                              const char* /*data*/) {
  if (!qrcode) {
    return -1;
  }
  qrcode->size = version * 4 + 17;
  return 0;
}

inline int8_t qrcode_initBytes(QRCode* qrcode, uint8_t* modules, uint8_t version, QrCodeEcc ecc, uint8_t* /*data*/,
                               uint16_t /*length*/) {
  return qrcode_initText(qrcode, modules, version, ecc, "");
}

inline int qrcode_getModule(QRCode* qrcode, uint8_t x, uint8_t y) {
  if (!qrcode || qrcode->size == 0) {
    return 0;
  }
  return ((x * 17u + y * 31u + x * y) % 7u) < 3u;
}
