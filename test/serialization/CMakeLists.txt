add_executable(SerializationTest
  SerializationTest.cpp
)

target_include_directories(SerializationTest PRIVATE
  ${REPO_ROOT}/lib/Memory
  ${REPO_ROOT}/lib/Serialization
  ${REPO_ROOT}/test/chapter_html_slim_parser/stubs
)

target_link_libraries(SerializationTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(SerializationTest)
