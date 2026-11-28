add_executable(SyncExtensionsTest
  SyncExtensionsTest.cpp
  ${REPO_ROOT}/src/sync/SyncEngine.cpp
  ${REPO_ROOT}/src/sync/SyncRequest.cpp
  ${REPO_ROOT}/src/sync/BookmarkSync.cpp
  ${REPO_ROOT}/src/clippings/ClippingSync.cpp
  ${REPO_ROOT}/src/ClippingStore.cpp
  ${REPO_ROOT}/src/sync/ReadingSessionLog.cpp
  ${REPO_ROOT}/src/util/BookmarkFile.cpp
  ${REPO_ROOT}/src/util/BookmarkUtil.cpp
  ${REPO_ROOT}/lib/Utf8/Utf8.cpp
  ${REPO_ROOT}/lib/Memory/BuildScratch.cpp
)
# The suite checks with assert(); keep it on in Release builds.
target_compile_options(SyncExtensionsTest PRIVATE -fno-exceptions -UNDEBUG)
target_include_directories(SyncExtensionsTest PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/stubs
  ${REPO_ROOT}/test/clipping_sync/stubs
  ${REPO_ROOT}/test/library_builder/stubs
  ${REPO_ROOT}/test/clipping_store/stubs
  ${REPO_ROOT}/src
  ${REPO_ROOT}/lib/Memory
  ${REPO_ROOT}/lib/KOReaderSync
  ${REPO_ROOT}/lib/Serialization
  ${REPO_ROOT}/lib/Utf8
)
target_link_libraries(SyncExtensionsTest PRIVATE crosspoint_test_common crosspoint_arduinojson)
add_test(NAME SyncExtensionsTest COMMAND SyncExtensionsTest)
