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

using serialization::MAX_STRING_BYTES;

// Writes raw bytes so a test can cut a length prefix or payload short.
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

std::string lengthPrefix(const uint32_t len) { return std::string(reinterpret_cast<const char*>(&len), sizeof(len)); }

// Reads one string with each reader from the same bytes; both must agree.
struct ReadResult {
  bool ok;
  std::string value;
};

ReadResult readWithHalFile(const ScopedTempDir& dir, const std::string& bytes, const size_t maxLength) {
  HalFile file;
  EXPECT_TRUE(file.open(writeBytes(dir, bytes).c_str(), "rb"));
  ReadResult result{false, "stale"};
  result.ok = serialization::tryReadString(file, result.value, maxLength);
  return result;
}

ReadResult readWithBufferedReader(const ScopedTempDir& dir, const std::string& bytes, const size_t maxLength) {
  HalFile file;
  EXPECT_TRUE(file.open(writeBytes(dir, bytes).c_str(), "rb"));
  serialization::BufferedFileReader reader(file, 64);
  ReadResult result{false, "stale"};
  result.ok = serialization::tryReadString(reader, result.value, maxLength);
  return result;
}

class TryReadString : public testing::TestWithParam<bool> {
 protected:
  ReadResult read(const std::string& bytes, const size_t maxLength = MAX_STRING_BYTES) {
    return GetParam() ? readWithBufferedReader(dir, bytes, maxLength) : readWithHalFile(dir, bytes, maxLength);
  }
  ScopedTempDir dir;
};

}  // namespace

TEST_P(TryReadString, RoundTrip) {
  const auto result = read(encoded("chapter.xhtml"));
  EXPECT_TRUE(result.ok);
  EXPECT_EQ(result.value, "chapter.xhtml");
}

TEST_P(TryReadString, EmptyString) {
  const auto result = read(encoded(""));
  EXPECT_TRUE(result.ok);
  EXPECT_TRUE(result.value.empty());
}

TEST_P(TryReadString, LengthAtTheLimitIsRead) {
  const auto result = read(encoded("abcd"), 4);
  EXPECT_TRUE(result.ok);
  EXPECT_EQ(result.value, "abcd");
}

TEST_P(TryReadString, CutOffLengthFails) { EXPECT_FALSE(read(std::string("\x05\x00", 2)).ok); }

TEST_P(TryReadString, CutOffPayloadFails) { EXPECT_FALSE(read(lengthPrefix(5) + "ab").ok); }

// A corrupt length must not size the allocation, even when that many bytes follow.
TEST_P(TryReadString, LengthOverTheLimitFailsWithTheBytesPresent) {
  EXPECT_FALSE(read(lengthPrefix(5) + "abcde", 4).ok);
}

TEST_P(TryReadString, HugeLengthFails) { EXPECT_FALSE(read(lengthPrefix(0xFFFFFFF0u) + "abc").ok); }

INSTANTIATE_TEST_SUITE_P(Readers, TryReadString, testing::Bool(), [](const testing::TestParamInfo<bool>& info) {
  return info.param ? "BufferedFileReader" : "HalFile";
});

TEST(TryReadStringSequence, BufferedReaderReadsConsecutiveStringsThenFailsOnACutOffOne) {
  ScopedTempDir dir;
  HalFile file;
  ASSERT_TRUE(file.open(writeBytes(dir, encoded("a") + encoded("bc") + std::string("\x07", 1)).c_str(), "rb"));
  serialization::BufferedFileReader reader(file, 64);
  std::string s;
  ASSERT_TRUE(serialization::tryReadString(reader, s, MAX_STRING_BYTES));
  EXPECT_EQ(s, "a");
  ASSERT_TRUE(serialization::tryReadString(reader, s, MAX_STRING_BYTES));
  EXPECT_EQ(s, "bc");
  EXPECT_FALSE(serialization::tryReadString(reader, s, MAX_STRING_BYTES));
}

// writeString caps what it stores, so every string it writes reads back.
TEST(WriteString, StoresAtMostTheReadLimit) {
  const std::string longest(MAX_STRING_BYTES, 'x');
  EXPECT_EQ(encoded(longest).size(), sizeof(uint32_t) + MAX_STRING_BYTES);
  EXPECT_EQ(encoded(longest + "yz"), encoded(longest));
}

TEST(WriteString, CappedStringsReadBackFromEveryWriter) {
  ScopedTempDir dir;
  const std::string path = (dir.path() / "out.bin").string();
  const std::string tooLong(MAX_STRING_BYTES + 10, 'q');
  {
    HalFile file;
    ASSERT_TRUE(file.open(path.c_str(), "wb"));
    serialization::writeString(file, tooLong);
    serialization::BufferedFileWriter writer(file, 64);
    serialization::writeString(writer, tooLong);
    ASSERT_TRUE(writer.flush());
  }
  HalFile file;
  ASSERT_TRUE(file.open(path.c_str(), "rb"));
  std::string s;
  ASSERT_TRUE(serialization::tryReadString(file, s, MAX_STRING_BYTES));
  EXPECT_EQ(s, tooLong.substr(0, MAX_STRING_BYTES));
  ASSERT_TRUE(serialization::tryReadString(file, s, MAX_STRING_BYTES));
  EXPECT_EQ(s, tooLong.substr(0, MAX_STRING_BYTES));
  EXPECT_EQ(file.available(), 0);
}

TEST(WriteString, CutsAtAUtf8Boundary) {
  // "é" is two bytes and straddles the limit, so the cut drops it whole.
  const std::string s = std::string(MAX_STRING_BYTES - 1, 'a') + "\xC3\xA9" + "tail";
  EXPECT_EQ(serialization::cappedStringLength(s), MAX_STRING_BYTES - 1);
  EXPECT_EQ(encoded(s), encoded(std::string(MAX_STRING_BYTES - 1, 'a')));
}
