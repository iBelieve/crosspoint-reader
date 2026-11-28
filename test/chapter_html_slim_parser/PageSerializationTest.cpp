#include <Epub/Page.h>
#include <gtest/gtest.h>

#include <cstring>
#include <filesystem>
#include <memory>

#include "test/FailedWriteCheck.h"
#include "test/ScopedTempDir.h"

namespace {

// One element of every kind, plus a footnote and a link, so each writer is exercised.
Page makePage() {
  Page page;
  page.elements.push_back(std::make_unique<PageLine>(
      std::make_unique<TextBlock>(std::vector<std::string>{"kan", "ji"}, std::vector<int16_t>{0, 30},
                                  std::vector<EpdFontFamily::Style>{EpdFontFamily::REGULAR, EpdFontFamily::REGULAR},
                                  std::vector<uint8_t>{}, std::vector<uint16_t>{}, BlockStyle(),
                                  std::vector<std::string>{"", "ruby"}),
      4, 8));
  page.elements.push_back(
      std::make_unique<PageImage>(std::make_unique<ImageBlock>("/cache/img.pxc", "OEBPS/a.png", 120, 80), 10, 20));
  page.elements.push_back(std::make_unique<PageHorizontalRule>(200, 2, 0, 100));
  page.elements.push_back(std::make_unique<PageDropCap>(3, 512, EpdFontFamily::BOLD, "W", 0, 120));
  CssBorderSide sides[4];
  for (auto& side : sides) side = {1, CssBorderStyle::Solid};
  page.elements.push_back(std::make_unique<PageBorderBox>(300, 40, sides, true, 0, 140));
  page.addFootnote("1", "notes.xhtml#n1");
  EXPECT_TRUE(page.addLink("#target", 1, 2, 30, 12));
  return page;
}

}  // namespace

TEST(PageSerialization, RoundTripsEveryElementKind) {
  const Page page = makePage();
  const ScopedTempDir tempDir;
  const auto path = tempDir.path() / "page.bin";
  {
    HalFile out;
    ASSERT_TRUE(out.open(path.c_str(), "wb"));
    ASSERT_TRUE(page.serialize(out));
  }
  HalFile in;
  ASSERT_TRUE(in.open(path.c_str(), "rb"));
  const auto restored = Page::deserialize(in);
  ASSERT_NE(restored, nullptr);
  ASSERT_EQ(restored->elements.size(), page.elements.size());
  for (size_t i = 0; i < page.elements.size(); ++i) {
    EXPECT_EQ(restored->elements[i]->getTag(), page.elements[i]->getTag()) << "element " << i;
  }
  const auto& line = static_cast<const PageLine&>(*restored->elements[0]);
  EXPECT_EQ(line.getBlock()->rubyText(1), "ruby");
  const auto& image = static_cast<const PageImage&>(*restored->elements[1]).getImageBlock();
  EXPECT_EQ(image.getWidth(), 120);
  EXPECT_EQ(image.getHeight(), 80);
  ASSERT_EQ(restored->footnotes.size(), 1u);
  EXPECT_STREQ(restored->footnotes[0].href, "notes.xhtml#n1");
  ASSERT_EQ(restored->links.size(), 1u);
  EXPECT_STREQ(restored->links[0].href, "#target");
  EXPECT_EQ(in.position(), in.size());
}

// A full SD card fails a write. Section::onPageComplete relies on serialize() failing
// so the build drops the cache instead of keeping a page it cannot read back.
TEST(PageSerialization, ReportsEveryFailedWrite) {
  const Page page = makePage();
  const ScopedTempDir tempDir;
  const auto path = (tempDir.path() / "failed-write.bin").string();
  expectEveryFailedWriteReported(path.c_str(), [&](HalFile& out) { return page.serialize(out); });
}

TEST(ImageBlockSerialization, RejectsEveryTruncation) {
  ImageBlock block("/cache/img.pxc", "OEBPS/a.png", 120, 80);
  const ScopedTempDir tempDir;
  const auto path = tempDir.path() / "image.bin";
  size_t fullSize = 0;
  {
    HalFile out;
    ASSERT_TRUE(out.open(path.c_str(), "wb"));
    ASSERT_TRUE(block.serialize(out));
    fullSize = out.position();
  }
  // Longest first: growing the file again would pad it with zeros, not restore the bytes.
  for (size_t size = fullSize; size-- > 0;) {
    std::filesystem::resize_file(path, size);
    HalFile in;
    ASSERT_TRUE(in.open(path.c_str(), "rb"));
    EXPECT_EQ(ImageBlock::deserialize(in), nullptr) << size << " of " << fullSize << " bytes";
  }
}
