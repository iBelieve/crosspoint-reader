#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <vector>

// In-memory files with the subset of HalFile/HalStorage that XtcParser and Xtc use.
// Writes can be made to fail to model a full or failing SD card.
class HalFile {
  friend class HalStorage;

 public:
  uint64_t fileSize64() const { return data ? data->size() : 0; }
  bool seek(uint64_t offset) { return seek64(offset); }
  bool seek64(uint64_t offset) {
    if (!data || offset > data->size()) return false;
    pos = static_cast<size_t>(offset);
    return true;
  }
  int available() const { return data ? static_cast<int>(data->size() - pos) : 0; }
  int read(void* dst, size_t len) {
    if (!data) return 0;
    const size_t remaining = data->size() - pos;
    const size_t count = len < remaining ? len : remaining;
    if (count > 0) std::memcpy(dst, data->data() + pos, count);
    pos += count;
    return static_cast<int>(count);
  }
  size_t write(const uint8_t* src, size_t len);
  bool close();
  bool isOpen() const { return data != nullptr; }

 private:
  std::vector<uint8_t>* data = nullptr;
  size_t pos = 0;
  bool writable = false;
};

class HalStorage {
 public:
  static HalStorage& getInstance() {
    static HalStorage instance;
    return instance;
  }

  bool openFileForRead(const char*, const std::string& path, HalFile& file) {
    const auto it = files.find(path);
    if (it == files.end()) return false;
    file.data = &it->second;
    file.pos = 0;
    file.writable = false;
    return true;
  }
  bool openFileForWrite(const char*, const std::string& path, HalFile& file) {
    auto& bytes = files[path];
    bytes.clear();
    file.data = &bytes;
    file.pos = 0;
    file.writable = true;
    return true;
  }
  bool exists(const char* path) const { return files.contains(path) || dirs.contains(path); }
  bool remove(const char* path) { return files.erase(path) > 0; }
  bool replaceFile(const char* tmpPath, const char* path) {
    const auto it = files.find(tmpPath);
    if (it == files.end() || failReplace) return false;
    files[path] = std::move(it->second);
    files.erase(tmpPath);
    return true;
  }
  bool mkdir(const char* path) {
    dirs.insert(path);
    return true;
  }
  bool removeDir(const char* path) {
    const std::string prefix = std::string(path) + "/";
    for (auto it = files.begin(); it != files.end();) {
      it = it->first.rfind(prefix, 0) == 0 ? files.erase(it) : std::next(it);
    }
    dirs.erase(path);
    return true;
  }

  void setFile(const std::string& path, std::vector<uint8_t> bytes) { files[path] = std::move(bytes); }
  const std::vector<uint8_t>* file(const std::string& path) const {
    const auto it = files.find(path);
    return it == files.end() ? nullptr : &it->second;
  }
  void reset() {
    files.clear();
    dirs.clear();
    writeCalls = 0;
    failingWriteCall = SIZE_MAX;
    failClose = false;
    failReplace = false;
  }

  // Zero-based index of the one write call that comes up short, counted across files.
  size_t writeCalls = 0;
  size_t failingWriteCall = SIZE_MAX;
  bool failClose = false;
  bool failReplace = false;

 private:
  std::map<std::string, std::vector<uint8_t>> files;
  std::set<std::string> dirs;
};

#define Storage HalStorage::getInstance()

inline size_t HalFile::write(const uint8_t* src, size_t len) {
  if (!data || !writable) return 0;
  // A failing call writes half its bytes, as a card that fills mid-write would.
  const size_t count = Storage.writeCalls++ == Storage.failingWriteCall ? len / 2 : len;
  data->insert(data->end(), src, src + count);
  pos = data->size();
  return count;
}

inline bool HalFile::close() {
  const bool ok = !(writable && Storage.failClose);
  data = nullptr;
  pos = 0;
  writable = false;
  return ok;
}
