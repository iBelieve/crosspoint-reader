#include <BufferedFile.h>
#include <HalStorage.h>
#include <Serialization.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>

#include "test/ScopedTempDir.h"

namespace {

// Writes raw bytes so a test can cut a length prefix short.
std::string writeBytes(const ScopedTempDir& dir, const std::string& bytes) {
  const std::string path = (dir.path() / "data.bin").string();
  std::ofstream(path, std::ios::binary) << bytes;
  return path;
}

std::string encoded(const std::string& value) {
  std::ostringstream out;
  serialization::writeString(out, value);
  return out.str();
}

}  // namespace

TEST(SerializationReadString, HalFileRoundTrip) {
  ScopedTempDir dir;
  HalFile file;
  ASSERT_TRUE(file.open(writeBytes(dir, encoded("chapter.xhtml")).c_str(), "rb"));
  std::string s;
  serialization::readString(file, s);
  EXPECT_EQ(s, "chapter.xhtml");
}

TEST(SerializationReadString, HalFileTruncatedLengthGivesEmptyString) {
  ScopedTempDir dir;
  HalFile file;
  ASSERT_TRUE(file.open(writeBytes(dir, std::string("\x05\x00", 2)).c_str(), "rb"));
  std::string s = "stale";
  serialization::readString(file, s);
  EXPECT_TRUE(s.empty());
}

TEST(SerializationReadString, BufferedReaderTruncatedLengthGivesEmptyString) {
  ScopedTempDir dir;
  HalFile file;
  ASSERT_TRUE(file.open(writeBytes(dir, encoded("a") + std::string("\x07", 1)).c_str(), "rb"));
  serialization::BufferedFileReader reader(file, 64);
  std::string s;
  serialization::readString(reader, s);
  EXPECT_EQ(s, "a");
  serialization::readString(reader, s);
  EXPECT_TRUE(s.empty());
}

TEST(SerializationReadString, StreamTruncatedLengthGivesEmptyString) {
  std::istringstream in(std::string("\x01\x02\x03", 3));
  std::string s = "stale";
  serialization::readString(in, s);
  EXPECT_TRUE(s.empty());
}
