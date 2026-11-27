# Built once per chip: the C3 and S3 request paths differ (SyncRequest.cpp).
foreach(chip ESP32C3 ESP32S3)
  set(target ClippingSyncTest_${chip})
  add_executable(${target}
    ClippingSyncTest.cpp
    ${REPO_ROOT}/src/clippings/ClippingSync.cpp
    ${REPO_ROOT}/src/sync/SyncRequest.cpp
    ${REPO_ROOT}/src/ClippingStore.cpp
    ${REPO_ROOT}/lib/Utf8/Utf8.cpp
    ${REPO_ROOT}/lib/Memory/BuildScratch.cpp
  )
  target_compile_definitions(${target} PRIVATE CONFIG_IDF_TARGET_${chip}=1)
  # The suite checks with assert(); keep it on in Release builds.
  target_compile_options(${target} PRIVATE -fno-exceptions -Wno-unused-parameter -UNDEBUG)
  target_include_directories(${target} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/stubs
    ${REPO_ROOT}/test/clipping_store/stubs
    ${REPO_ROOT}/test/library_builder/stubs
    ${REPO_ROOT}/src
    ${REPO_ROOT}/lib/Memory
    ${REPO_ROOT}/lib/Serialization
    ${REPO_ROOT}/lib/Utf8
  )
  target_link_libraries(${target} PRIVATE crosspoint_test_common ArduinoJson)
  add_test(NAME ${target} COMMAND ${target})
endforeach()
