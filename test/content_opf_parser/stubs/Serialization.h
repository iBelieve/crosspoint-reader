#pragma once

#include <string>

#include "Epub.h"

namespace serialization {

inline void writeString(HalFile&, const std::string&) {}
constexpr size_t MAX_STRING_BYTES = 4096;
inline bool tryReadString(HalFile&, std::string& out, size_t) {
  out.clear();
  return false;
}

}  // namespace serialization
