add_executable(XtcParserTest
  XtcParserTest.cpp
  ${REPO_ROOT}/lib/Xtc/Xtc/XtcParser.cpp
)
target_include_directories(XtcParserTest PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/stubs
  ${REPO_ROOT}/lib/Xtc/Xtc
)
target_link_libraries(XtcParserTest PRIVATE crosspoint_test_common GTest::gtest_main)
gtest_discover_tests(XtcParserTest)

add_executable(XtcCoverTest
  XtcCoverTest.cpp
  ${REPO_ROOT}/lib/Xtc/Xtc.cpp
  ${REPO_ROOT}/lib/Xtc/Xtc/XtcParser.cpp
  ${REPO_ROOT}/lib/GfxRenderer/BitmapHelpers.cpp
)
target_include_directories(XtcCoverTest PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/stubs
  ${REPO_ROOT}/lib/Xtc
  ${REPO_ROOT}/lib/Xtc/Xtc
  ${REPO_ROOT}/lib/GfxRenderer
)
target_link_libraries(XtcCoverTest PRIVATE crosspoint_test_common GTest::gtest_main)
gtest_discover_tests(XtcCoverTest)
