cmake_minimum_required(VERSION 3.16)
project(crosspoint_reader_tests C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Keep in step with bblanchon/ArduinoJson in platformio.ini. Only src/ is used,
# never its own build, so a local copy (FETCHCONTENT_SOURCE_DIR_ARDUINOJSON)
# such as .pio/libdeps/default/ArduinoJson works too. lib/BlePageTurner/test
# declares the same source.
FetchContent_Declare(
  arduinojson
  GIT_REPOSITORY https://github.com/bblanchon/ArduinoJson.git
  GIT_TAG v7.4.2
  GIT_SHALLOW TRUE
  SOURCE_SUBDIR no-build
)
FetchContent_MakeAvailable(arduinojson)
add_library(crosspoint_arduinojson INTERFACE)
target_include_directories(crosspoint_arduinojson SYSTEM INTERFACE ${arduinojson_SOURCE_DIR}/src)

set(REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")

# lib/OtShaper, the Indic shaping engine, for tests that shape.
file(GLOB OT_SHAPER_SOURCES ${REPO_ROOT}/lib/OtShaper/*.cpp)
set(OT_SHAPER_INCLUDE_DIR ${REPO_ROOT}/lib/OtShaper)

add_library(crosspoint_test_common INTERFACE)
target_include_directories(crosspoint_test_common INTERFACE
  ${REPO_ROOT}
  ${REPO_ROOT}/lib
)
option(CROSSPOINT_WERROR "Fail host test builds on compiler warnings" ON)

# Warnings fail the build (see CROSSPOINT_WERROR). Omitted struct fields are zero-initialized on purpose
# (generated font headers and designated initializers rely on it).
target_compile_options(crosspoint_test_common INTERFACE
  -Wall
  -Wextra
  -pedantic
  -Wno-missing-field-initializers
  $<$<BOOL:${CROSSPOINT_WERROR}>:-Werror>
)

# GCC reports false out-of-bounds accesses inside libstdc++'s std::sort on
# partly filled fixed arrays. Source properties are per directory, so each
# target that compiles these files calls this.
function(crosspoint_allow_sort_array_bounds)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set_source_files_properties(
      ${REPO_ROOT}/lib/Epub/Epub/css/CssParser.cpp
      ${REPO_ROOT}/lib/LibraryIndex/LibraryText.cpp
      PROPERTIES COMPILE_OPTIONS -Wno-array-bounds
    )
  endif()
endfunction()

enable_testing()
include(GoogleTest)

add_subdirectory(streaming_json_parser)
add_subdirectory(release_json_parser)
add_subdirectory(differential_rounding)
add_subdirectory(hyphenation_eval)
add_subdirectory(utf8_compose)
add_subdirectory(indic_text)
add_subdirectory(ot_shaper)
add_subdirectory(complex_shaper)
add_subdirectory(flash_blob_cache)
add_subdirectory(text_block_display)
add_subdirectory(opds_filename)
add_subdirectory(url_utils)
add_subdirectory(opds_feed_error)
add_subdirectory(html_to_plain_text)
add_subdirectory(library_text)
add_subdirectory(library_format)
add_subdirectory(library_index_file)
add_subdirectory(library_builder)
add_subdirectory(minibidi_arabic)
add_subdirectory(combining_marks)
add_subdirectory(credential_integrity)
add_subdirectory(xml_parser_utils)
add_subdirectory(token_boundary)
add_subdirectory(koreader_xpath_resolver)
add_subdirectory(progress_comparison)
add_subdirectory(font_cache_manager)
add_subdirectory(css_parser)
add_subdirectory(chapter_html_slim_parser)
add_subdirectory(korean_line_breaking)
add_subdirectory(em_dash_line_breaking)
add_subdirectory(ligature_guard)
add_subdirectory(page_link)
add_subdirectory(content_opf_parser)
add_subdirectory(chapter_position)
add_subdirectory(book_metadata_cache)
add_subdirectory(home_button)
add_subdirectory(button_press_navigation)
add_subdirectory(serialization)

add_subdirectory(inflate_stream)
add_subdirectory(inflate_reader)
add_subdirectory(zip_file)
add_subdirectory(xtc_parser)
add_subdirectory(fs_helpers)
add_subdirectory(http_header)
add_subdirectory(txt_to_html)

add_subdirectory(absolute_grayscale)

add_subdirectory(gif_support)

add_subdirectory(bmp_to_bmp)

add_subdirectory(sd_card_font)
add_subdirectory(sd_font_registry)
add_subdirectory(clipping_preview)
add_subdirectory(glyph_bitmap)
add_subdirectory(bmp_stream_writer)

add_subdirectory(clipping_store)
add_subdirectory(reader_session)
add_subdirectory(plugin_catalog_paging)
add_subdirectory(plugin_manifest_classification)
add_subdirectory(plugin_picker_layout)
add_subdirectory(protected_paths)
add_subdirectory(clipping_sync)
add_subdirectory(sync_extensions)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_test(NAME VectorFontIdentity COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_vector_font_identity.py)
add_test(NAME translation_spacing COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_translation_spacing.py)
add_test(NAME TrustedTimeTimezone COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_trusted_time.py)
add_test(NAME RemoteButtonInputIntegration COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_remote_button_input.py)
add_test(NAME SideButtonOrientation COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_side_button_orientation.py)
add_test(NAME confirmation_translations COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_confirmation_translations.py)
add_test(NAME BleLearning COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_ble_learning.py)
add_subdirectory(ble_link_note_title)
add_subdirectory(ble_chapter_build_gate)
add_subdirectory(ble_remote_turn_gate)

add_subdirectory(${REPO_ROOT}/lib/BlePageTurner/test BlePageTurnerTests)

add_test(NAME selection_buttons COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_selection_buttons.py)
add_test(NAME button_remap_release COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_button_remap_release.py)
add_test(NAME list_viewport COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_list_viewport.py)
add_test(NAME clipping_highlight_planes COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_clipping_highlight_planes.py)
add_test(NAME clipping_selection_text COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_clipping_selection_text.py)

add_test(NAME recursive_dictionary COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_recursive_dictionary.py)
add_test(NAME builtin_header_layout COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_builtin_header_layout.py)
add_test(NAME dictionary_setting_name COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_dictionary_setting_name.py)
add_test(NAME dictionary_side_button_navigation COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_dictionary_side_button_navigation.py)
add_test(NAME release_license COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_release_license.py)
add_test(NAME text_settings_layout COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_text_settings_layout.py)
