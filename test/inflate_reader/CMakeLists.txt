find_package(ZLIB REQUIRED)

add_executable(InflateReaderTest
  InflateReaderTest.cpp
  ${REPO_ROOT}/lib/InflateReader/InflateReader.cpp
  ${REPO_ROOT}/lib/uzlib/src/tinflate.c
)

target_include_directories(InflateReaderTest PRIVATE
  ${REPO_ROOT}/lib/InflateReader
  ${REPO_ROOT}/lib/uzlib/src
)

target_link_libraries(InflateReaderTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
  ZLIB::ZLIB
)

gtest_discover_tests(InflateReaderTest)
# Vendored code: not ours to make warning-clean.
set_source_files_properties(${REPO_ROOT}/lib/uzlib/src/tinflate.c PROPERTIES COMPILE_OPTIONS -w)
