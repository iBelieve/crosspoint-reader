#pragma once

#include <HalStorage.h>
#include <gtest/gtest.h>

#include <cstddef>

// Fails each write call of `serialize` in turn (all others succeed) and expects every
// failure to be reported, so no writer anywhere in the chain ignores a write result.
// Needs the test HalFile from test/chapter_html_slim_parser/stubs.
template <typename Serialize>
void expectEveryFailedWriteReported(const char* path, Serialize serialize) {
  size_t calls = 0;
  {
    HalFile out;
    ASSERT_TRUE(out.open(path, "wb"));
    ASSERT_TRUE(serialize(out));
    calls = out.writeCalls();
  }
  ASSERT_GT(calls, 0u);
  for (size_t i = 0; i < calls; ++i) {
    HalFile out;
    ASSERT_TRUE(out.open(path, "wb"));
    out.failWriteCall(i);
    EXPECT_FALSE(serialize(out)) << "write call " << i << " of " << calls << " failed unnoticed";
  }
}
