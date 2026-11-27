add_executable(PluginPickerLayoutTest
  PluginPickerLayoutTest.cpp
  ${REPO_ROOT}/lib/EpdFont/EpdFont.cpp
  ${REPO_ROOT}/lib/Utf8/Utf8.cpp
  ${REPO_ROOT}/freeink-sdk/libs/ui/FreeInkUI/src/FreeInkUI.cpp
)
target_include_directories(PluginPickerLayoutTest PRIVATE
  ${REPO_ROOT}/src
  ${REPO_ROOT}/lib/EpdFont
  ${REPO_ROOT}/lib/Utf8
)
# The SDK is a separate project: its header warnings must not fail this build.
target_include_directories(PluginPickerLayoutTest SYSTEM PRIVATE ${REPO_ROOT}/freeink-sdk/libs/ui/FreeInkUI/include)
target_link_libraries(PluginPickerLayoutTest PRIVATE crosspoint_test_common GTest::gtest_main)
gtest_discover_tests(PluginPickerLayoutTest)
