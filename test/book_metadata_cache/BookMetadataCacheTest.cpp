#include <HalStorage.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <new>
#include <string>
#include <vector>

#include "Epub/BookMetadataCache.h"
#include "ZipFile.h"
#include "test/ScopedTempDir.h"

// Counting allocator: heap bytes allocated while `counting` is set and not yet freed.
namespace heap {
bool counting = false;
size_t live = 0;
size_t largest = 0;  // largest single allocation while counting

struct alignas(std::max_align_t) Header {
  size_t size;
  bool counted;
};

void* allocate(const size_t size) {
  auto* header = static_cast<Header*>(std::malloc(sizeof(Header) + size));
  if (!header) return nullptr;
  header->size = size;
  header->counted = counting;
  if (counting) {
    live += size;
    largest = std::max(largest, size);
  }
  return header + 1;
}

void release(void* p) {
  if (!p) return;
  auto* header = static_cast<Header*>(p) - 1;
  if (header->counted) live -= header->size;
  std::free(header);
}
}  // namespace heap

void* operator new(const size_t size) {
  void* p = heap::allocate(size);
  if (!p) throw std::bad_alloc();
  return p;
}
void* operator new[](const size_t size) { return operator new(size); }
void* operator new(const size_t size, const std::nothrow_t&) noexcept { return heap::allocate(size); }
void* operator new[](const size_t size, const std::nothrow_t&) noexcept { return heap::allocate(size); }
void operator delete(void* p) noexcept { heap::release(p); }
void operator delete[](void* p) noexcept { heap::release(p); }
void operator delete(void* p, size_t) noexcept { heap::release(p); }
void operator delete[](void* p, size_t) noexcept { heap::release(p); }

namespace {

std::string chapterHref(const int i) { return "OEBPS/Text/chapter" + std::to_string(i) + ".xhtml"; }
// Empty chapters, some across the window edge at item 32, give equal running totals.
uint32_t chapterBytes(const int i) {
  return (i % 37 == 5 || (i >= 30 && i < 35)) ? 0 : 1500 + static_cast<uint32_t>(i) * 7919 % 30000;
}

// Writes book.bin through the cache's own writer; returns every chapter's cumulative size.
std::vector<uint32_t> buildBook(const std::string& dir, const int spineCount) {
  zipEntrySizes.clear();
  std::vector<uint32_t> cumulative;
  for (int i = 0; i < spineCount; i++) {
    zipEntrySizes[chapterHref(i)] = chapterBytes(i);
    cumulative.push_back((i > 0 ? cumulative.back() : 0) + chapterBytes(i));
  }
  BookMetadataCache cache(dir);
  EXPECT_TRUE(cache.beginWrite() && cache.beginContentOpfPass());
  for (int i = 0; i < spineCount; i++) cache.createSpineEntry(chapterHref(i));
  EXPECT_TRUE(cache.endContentOpfPass() && cache.beginTocPass());
  for (int i = 0; i < spineCount; i++) cache.createTocEntry("Chapter " + std::to_string(i), chapterHref(i), "", 1);
  EXPECT_TRUE(cache.endTocPass() && cache.endWrite());
  EXPECT_TRUE(cache.buildBookBin("book.epub", {}));
  EXPECT_TRUE(cache.cleanupTmpFiles());
  return cumulative;
}

}  // namespace

// A loaded book must not hold heap per chapter: 5,000 chapters may hold at most 1 KB more than 30.
TEST(BookMetadataCacheHeap, ResidentHeapDoesNotGrowWithSpineCount) {
  size_t resident[2] = {};
  for (const int spineCount : {30, 5000}) {
    const ScopedTempDir tempDir;
    const auto cacheDir = tempDir.path().string();
    const std::vector<uint32_t> cumulative = buildBook(cacheDir, spineCount);
    BookMetadataCache cache(cacheDir);
    heap::live = 0;
    heap::counting = true;
    EXPECT_TRUE(cache.load());
    const size_t loaded = heap::live;
    for (int i = 0; i < spineCount; i++) EXPECT_EQ(cache.getCumulativeSize(i), cumulative[i]) << "spine " << i;
    EXPECT_EQ(heap::live, loaded) << spineCount << " chapters";
    heap::counting = false;
    resident[spineCount == 5000] = loaded;
  }
  EXPECT_LE(resident[1], resident[0] + 1024) << "30 chapters: " << resident[0] << " B";
}

// book.bin cut short inside a window: the window answers the total before it, never less.
TEST(BookMetadataCacheSizes, TruncatedWindowFallsBackToTheTotalBeforeIt) {
  const ScopedTempDir tempDir;
  const auto cacheDir = tempDir.path().string();
  const std::vector<uint32_t> cumulative = buildBook(cacheDir, 100);
  const auto path = tempDir.path() / "book.bin";
  std::ifstream in(path, std::ios::binary);
  const std::string bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  const size_t href = bytes.find(chapterHref(64));
  ASSERT_NE(href, std::string::npos);
  BookMetadataCache cache(cacheDir);
  ASSERT_TRUE(cache.load());
  EXPECT_EQ(cache.getCumulativeSize(0), cumulative[0]);
  std::filesystem::resize_file(path, href + chapterHref(64).size());  // just before item 64's size
  EXPECT_EQ(cache.getCumulativeSize(63), cumulative[63]);
  EXPECT_EQ(cache.getCumulativeSize(64), cumulative[63]);
  // A lookup into that window tries the refill once and keeps the fallback; the next lookup retries.
  for (int attempt = 0; attempt < 2; attempt++) {
    const int opens = Storage.readOpens;
    EXPECT_EQ(cache.getSpineIndexForSize(cumulative[70]), 95);
    EXPECT_EQ(Storage.readOpens - opens, 1) << "attempt " << attempt;
  }
}

// The lookup answers what a scan of every item answers, for every boundary, and reads at most one window.
TEST(BookMetadataCacheSizes, SpineIndexForSizeMatchesLinearScan) {
  for (const int spineCount : {100, 5000}) {
    const ScopedTempDir tempDir;
    const auto cacheDir = tempDir.path().string();
    const std::vector<uint32_t> cumulative = buildBook(cacheDir, spineCount);
    BookMetadataCache scanned(cacheDir);
    BookMetadataCache cache(cacheDir);
    ASSERT_TRUE(scanned.load() && cache.load());
    std::vector<uint32_t> totals;
    for (int i = 0; i < spineCount; i++) totals.push_back(scanned.getCumulativeSize(i));
    const uint32_t bookSize = cumulative.back();
    std::vector<uint32_t> targets = {0, bookSize - 1, bookSize, bookSize + 1, 0};
    for (const uint32_t total : cumulative) targets.insert(targets.end(), {total - 1, total, total + 1});
    for (const uint32_t size : targets) {
      int expected = -1;
      for (int i = 0; i < spineCount && expected < 0; i++) {
        if (totals[i] >= size) expected = i;
      }
      const int opens = Storage.readOpens;
      EXPECT_EQ(cache.getSpineIndexForSize(size), expected) << spineCount << " items, size " << size;
      EXPECT_LE(Storage.readOpens - opens, 1) << spineCount << " items, size " << size;
    }
  }
}

// A complete but corrupt string length fails that entry instead of sizing an allocation from it.
TEST(BookMetadataCacheStrings, CorruptTocTitleLengthFailsOnlyThatEntry) {
  const ScopedTempDir tempDir;
  const auto cacheDir = tempDir.path().string();
  buildBook(cacheDir, 600);
  const auto path = tempDir.path() / "book.bin";
  std::string bytes;
  {
    std::ifstream in(path, std::ios::binary);
    bytes.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
  }
  const size_t title = bytes.find("Chapter 150");
  ASSERT_NE(title, std::string::npos);
  const uint32_t corruptLength = serialization::MAX_STRING_BYTES * 2;
  ASSERT_GT(bytes.size() - title, corruptLength) << "the bytes must be present for the test to mean anything";
  bytes.replace(title - sizeof(uint32_t), sizeof(uint32_t), reinterpret_cast<const char*>(&corruptLength),
                sizeof(uint32_t));
  std::ofstream(path, std::ios::binary | std::ios::trunc) << bytes;

  BookMetadataCache cache(cacheDir);
  ASSERT_TRUE(cache.load());
  heap::largest = 0;
  heap::counting = true;
  const auto corrupt = cache.getTocEntry(150);
  heap::counting = false;
  EXPECT_LE(heap::largest, serialization::MAX_STRING_BYTES + 1);
  EXPECT_TRUE(corrupt.title.empty());
  EXPECT_EQ(corrupt.spineIndex, -1);
  const auto next = cache.getTocEntry(151);
  EXPECT_EQ(next.title, "Chapter 151");
  EXPECT_EQ(next.spineIndex, 151);
}

// Over-long strings are cut to what the reader accepts, and the LUT offsets still line up.
TEST(BookMetadataCacheStrings, OverLongStringsAreCutAndTheRestStillLoads) {
  const ScopedTempDir tempDir;
  const auto cacheDir = tempDir.path().string();
  zipEntrySizes.clear();
  const std::string longTitle(serialization::MAX_STRING_BYTES + 100, 't');
  {
    BookMetadataCache cache(cacheDir);
    ASSERT_TRUE(cache.beginWrite() && cache.beginContentOpfPass());
    for (int i = 0; i < 3; i++) cache.createSpineEntry(chapterHref(i));
    ASSERT_TRUE(cache.endContentOpfPass() && cache.beginTocPass());
    cache.createTocEntry(longTitle, chapterHref(0), "", 1);
    cache.createTocEntry("Chapter 1", chapterHref(1), "", 1);
    ASSERT_TRUE(cache.endTocPass() && cache.endWrite());
    BookMetadataCache::BookMetadata metadata;
    metadata.title = longTitle;
    metadata.author = "Author";
    ASSERT_TRUE(cache.buildBookBin("book.epub", metadata));
    ASSERT_TRUE(cache.cleanupTmpFiles());
  }
  BookMetadataCache cache(cacheDir);
  ASSERT_TRUE(cache.load());
  EXPECT_EQ(cache.coreMetadata.title, longTitle.substr(0, serialization::MAX_STRING_BYTES));
  EXPECT_EQ(cache.coreMetadata.author, "Author");
  EXPECT_EQ(cache.getTocEntry(0).title, longTitle.substr(0, serialization::MAX_STRING_BYTES));
  EXPECT_EQ(cache.getTocEntry(1).title, "Chapter 1");
  EXPECT_EQ(cache.getSpineEntry(2).href, chapterHref(2));
}

// A spine href the entry readers would reject makes load() fail, so the book is rebuilt.
TEST(BookMetadataCacheStrings, LoadRejectsASpineHrefOverTheLimit) {
  const ScopedTempDir tempDir;
  const auto cacheDir = tempDir.path().string();
  buildBook(cacheDir, 600);
  const auto path = tempDir.path() / "book.bin";
  std::string bytes;
  {
    std::ifstream in(path, std::ios::binary);
    bytes.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
  }
  const size_t href = bytes.find(chapterHref(10));
  ASSERT_NE(href, std::string::npos);
  const uint32_t corruptLength = serialization::MAX_STRING_BYTES + 1;
  ASSERT_GT(bytes.size() - href, corruptLength);
  bytes.replace(href - sizeof(uint32_t), sizeof(uint32_t), reinterpret_cast<const char*>(&corruptLength),
                sizeof(uint32_t));
  std::ofstream(path, std::ios::binary | std::ios::trunc) << bytes;
  BookMetadataCache cache(cacheDir);
  EXPECT_FALSE(cache.load());
}

// A cut anchor could match the wrong target, so an over-long one is stored empty (chapter start).
TEST(BookMetadataCacheStrings, OverLongTocAnchorIsStoredEmpty) {
  const ScopedTempDir tempDir;
  const auto cacheDir = tempDir.path().string();
  zipEntrySizes.clear();
  {
    BookMetadataCache cache(cacheDir);
    ASSERT_TRUE(cache.beginWrite() && cache.beginContentOpfPass());
    cache.createSpineEntry(chapterHref(0));
    ASSERT_TRUE(cache.endContentOpfPass() && cache.beginTocPass());
    cache.createTocEntry("Long", chapterHref(0), std::string(serialization::MAX_STRING_BYTES + 1, 'a'), 1);
    cache.createTocEntry("Short", chapterHref(0), "note1", 1);
    ASSERT_TRUE(cache.endTocPass() && cache.endWrite());
    ASSERT_TRUE(cache.buildBookBin("book.epub", {}));
  }
  BookMetadataCache cache(cacheDir);
  ASSERT_TRUE(cache.load());
  const auto longEntry = cache.getTocEntry(0);
  EXPECT_TRUE(longEntry.anchor.empty());
  EXPECT_EQ(longEntry.spineIndex, 0);
  EXPECT_EQ(cache.getTocEntry(1).anchor, "note1");
}
