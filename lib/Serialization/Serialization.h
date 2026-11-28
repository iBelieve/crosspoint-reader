#pragma once
#include <HalStorage.h>

#include <algorithm>
#include <iostream>
#include <limits>

namespace serialization {
template <typename T>
void writePod(std::ostream& os, const T& value) {
  os.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

template <typename T>
void writePod(HalFile& file, const T& value) {
  file.write(reinterpret_cast<const uint8_t*>(&value), sizeof(T));
}

template <typename T>
bool tryWritePod(HalFile& file, const T& value) {
  return file.write(reinterpret_cast<const uint8_t*>(&value), sizeof(T)) == sizeof(T);
}

template <typename T>
void readPod(std::istream& is, T& value) {
  is.read(reinterpret_cast<char*>(&value), sizeof(T));
}

template <typename T>
void readPod(HalFile& file, T& value) {
  file.read(reinterpret_cast<uint8_t*>(&value), sizeof(T));
}

template <typename T>
bool tryReadPod(HalFile& file, T& value) {
  return file.read(reinterpret_cast<uint8_t*>(&value), sizeof(T)) == sizeof(T);
}

// Cached strings are read with tryReadString(..., MAX_STRING_BYTES), so a
// corrupt length cannot ask for a huge allocation. Callers that look strings
// up by key skip longer ones; writeString() cuts any other longer string at a
// UTF-8 boundary so everything it writes reads back.
constexpr size_t MAX_STRING_BYTES = 4096;

inline uint32_t cappedStringLength(const std::string& s) {
  if (s.size() <= MAX_STRING_BYTES) return static_cast<uint32_t>(s.size());
  size_t len = MAX_STRING_BYTES;
  // Back off continuation bytes (10xxxxxx) so the cut lands before a lead byte.
  while (len > 0 && (static_cast<uint8_t>(s[len]) & 0xC0) == 0x80) len--;
  return static_cast<uint32_t>(len);
}

// For strings used as lookup keys (paths, anchors): a cut key could match the
// wrong target, so an over-long one is stored empty instead.
inline const std::string& keyOrEmpty(const std::string& s) {
  static const std::string EMPTY_KEY;
  return s.size() <= MAX_STRING_BYTES ? s : EMPTY_KEY;
}

inline void writeString(std::ostream& os, const std::string& s) {
  const uint32_t len = cappedStringLength(s);
  writePod(os, len);
  os.write(s.data(), len);
}

inline void writeString(HalFile& file, const std::string& s) {
  const uint32_t len = cappedStringLength(s);
  writePod(file, len);
  file.write(reinterpret_cast<const uint8_t*>(s.data()), len);
}

// Writes the whole string; callers bound it themselves (see ClippingStore).
inline bool tryWriteString(HalFile& file, const std::string& s) {
  const uint32_t len = s.size();
  return tryWritePod(file, len) && (len == 0 || file.write(reinterpret_cast<const uint8_t*>(s.data()), len) == len);
}

// Fails, leaving `s` unspecified, if the length prefix or the bytes are cut off,
// or the length exceeds maxLength or the bytes left in the file.
inline bool tryReadString(HalFile& file, std::string& s, const size_t maxLength) {
  uint32_t len = 0;
  if (!tryReadPod(file, len) || len > maxLength) return false;
  if (len == 0) {
    s.clear();
    return true;
  }
  const int remaining = file.available();
  if (static_cast<size_t>(len) > s.max_size() || len > static_cast<uint32_t>(std::numeric_limits<int>::max()) ||
      remaining < 0 || len > static_cast<uint32_t>(remaining)) {
    return false;
  }
  s.resize(len);
  return file.read(&s[0], len) == static_cast<int>(len);
}
}  // namespace serialization
