#include <HalStorage.h>
#include <Xtc.h>
#include <gtest/gtest.h>

#include <cstring>
#include <ostream>
#include <vector>

using namespace xtc;

namespace {

constexpr const char* kBookPath = "/book.xtc";
constexpr const char* kCacheDir = "/.crosspoint";

uint32_t pageBytes(uint32_t pageMagic, uint16_t width, uint16_t height) {
  return pageMagic == XTH_MAGIC ? width * ((height + 7u) / 8u) * 2u : (width + 7u) / 8u * height;
}

// One-page book of `side` x `side` pixels, filled with a pattern.
std::vector<uint8_t> buildBook(uint32_t containerMagic, uint32_t pageMagic, uint16_t side) {
  XtcHeader header{};
  header.magic = containerMagic;
  header.versionMajor = 1;
  header.pageCount = 1;
  header.pageTableOffset = sizeof(XtcHeader);
  header.dataOffset = sizeof(XtcHeader) + sizeof(PageTableEntry);

  XtgPageHeader page{};
  page.magic = pageMagic;
  page.width = side;
  page.height = side;
  page.dataSize = pageBytes(pageMagic, side, side);

  const PageTableEntry entry{header.dataOffset, static_cast<uint32_t>(sizeof(XtgPageHeader) + page.dataSize), side,
                             side};
  std::vector<uint8_t> bytes(header.dataOffset + sizeof(XtgPageHeader) + page.dataSize);
  for (size_t i = 0; i < bytes.size(); i++) bytes[i] = static_cast<uint8_t>(i * 37);
  std::memcpy(bytes.data(), &header, sizeof(header));
  std::memcpy(bytes.data() + header.pageTableOffset, &entry, sizeof(entry));
  std::memcpy(bytes.data() + header.dataOffset, &page, sizeof(page));
  return bytes;
}

struct CoverCase {
  const char* name;
  uint32_t containerMagic;
  uint32_t pageMagic;
  uint16_t side;
  int thumbHeight;  // larger than side: the thumbnail is a copy of the cover
};

void PrintTo(const CoverCase& c, std::ostream* os) { *os << c.name; }

class XtcCoverWrites : public ::testing::TestWithParam<CoverCase> {
 protected:
  void SetUp() override {
    Storage.reset();
    const CoverCase& c = GetParam();
    Storage.setFile(kBookPath, buildBook(c.containerMagic, c.pageMagic, c.side));
    ASSERT_TRUE(xtc.load());
  }
  void TearDown() override { Storage.reset(); }

  // Counts the write calls `generate` makes, then fails each in turn. Every failure must
  // be reported and leave no file at `path`, or later calls would reuse a broken BMP.
  template <typename Generate>
  void expectEveryFailedWriteDropsFile(const std::string& path, Generate generate) {
    Storage.writeCalls = 0;
    ASSERT_TRUE(generate());
    const size_t calls = Storage.writeCalls;
    const std::vector<uint8_t> complete = *Storage.file(path);
    ASSERT_GT(calls, 0u);
    for (size_t i = 0; i < calls; i++) {
      Storage.remove(path.c_str());
      Storage.writeCalls = 0;
      Storage.failingWriteCall = i;
      EXPECT_FALSE(generate()) << "write call " << i << " of " << calls;
      EXPECT_EQ(Storage.file(path), nullptr) << "partial file kept after write call " << i;
    }
    Storage.failingWriteCall = SIZE_MAX;
    Storage.remove(path.c_str());
    ASSERT_TRUE(generate());
    EXPECT_EQ(*Storage.file(path), complete);
  }

  Xtc xtc{kBookPath, kCacheDir};
};

TEST_P(XtcCoverWrites, CoverDropsPartialFile) {
  expectEveryFailedWriteDropsFile(xtc.getCoverBmpPath(), [&] { return xtc.generateCoverBmp(); });
}

TEST_P(XtcCoverWrites, CoverDropsFileWhenCloseFails) {
  Storage.failClose = true;
  EXPECT_FALSE(xtc.generateCoverBmp());
  EXPECT_EQ(Storage.file(xtc.getCoverBmpPath()), nullptr);
}

TEST_P(XtcCoverWrites, ScaledThumbDropsPartialFile) {
  const int height = GetParam().side / 2;
  expectEveryFailedWriteDropsFile(xtc.getThumbBmpPath(height), [&] { return xtc.generateThumbBmp(height); });
}

TEST_P(XtcCoverWrites, ScaledThumbDropsFileWhenCloseFails) {
  const int height = GetParam().side / 2;
  Storage.failClose = true;
  EXPECT_FALSE(xtc.generateThumbBmp(height));
  EXPECT_EQ(Storage.file(xtc.getThumbBmpPath(height)), nullptr);
}

TEST_P(XtcCoverWrites, CopiedThumbDropsPartialFile) {
  const int height = GetParam().thumbHeight;
  ASSERT_TRUE(xtc.generateCoverBmp());
  expectEveryFailedWriteDropsFile(xtc.getThumbBmpPath(height), [&] { return xtc.generateThumbBmp(height); });
}

// Widths that are not a multiple of 32 pixels exercise the 1-bit row padding writes.
INSTANTIATE_TEST_SUITE_P(Depths, XtcCoverWrites,
                         ::testing::Values(CoverCase{"OneBit", XTC_MAGIC, XTG_MAGIC, 20, 200},
                                           CoverCase{"TwoBit", XTCH_MAGIC, XTH_MAGIC, 20, 200}),
                         [](const ::testing::TestParamInfo<CoverCase>& info) { return info.param.name; });

}  // namespace
