cmake_minimum_required(VERSION 2.8)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
project(vsfontcompiler)

enable_language(C)
enable_language(CXX)

include("defaults.cmake")

find_boost_program_options(TRUE)
find_boost_system(TRUE)
find_boost_thread(TRUE)
find_freetype(TRUE STATIC)
find_png(TRUE STATIC)

output_flags("DEBUG" on)

add_definitions(-DVERSION="r3")

include_directories("${PROJECT_SOURCE_DIR}/src")

set(GFX_SRC
  "src/gfx/image_png.cpp"
  "src/gfx/image_png.hpp")

set(THR_SRC
  "src/thr/dispatch.cpp"
  "src/thr/dispatch.hpp"
  "src/thr/generic.hpp"
  "src/thr/promise.hpp"
  "src/thr/thr_generic.cpp"
  "src/thr/thread_storage.cpp"
  "src/thr/thread_storage.hpp"
  "src/thr/worker_thread.hpp")

add_executable(vsfontcompiler
  ${GFX_SRC}
  ${THR_SRC}
  "src/defaults.hpp"
  "src/ft_face.cpp"
  "src/ft_face.hpp"
  "src/ft_glyph.cpp"
  "src/ft_glyph.hpp"
  "src/ft_library.cpp"
  "src/ft_library.hpp"
  "src/glyph_range.cpp"
  "src/glyph_range.hpp"
  "src/glyph_storage.cpp"
  "src/glyph_storage.hpp"
  "src/main.cpp"
  "src/sky_line.cpp"
  "src/sky_line.hpp"
  "src/sky_line_fitter.cpp"
  "src/sky_line_fitter.hpp")
if(${MSVC})
  target_link_libraries(vsfontcompiler "WINMM")
  target_link_libraries(vsfontcompiler debug ${FREETYPE_LIBRARY_DEBUG} optimized ${FREETYPE_LIBRARY})
  target_link_libraries(vsfontcompiler debug ${PNG_LIBRARY_DEBUG} optimized ${PNG_LIBRARY})
  target_link_libraries(vsfontcompiler debug ${ZLIB_LIBRARY_DEBUG} optimized ${ZLIB_LIBRARY})
else()
  target_link_libraries(vsfontcompiler ${BOOST_PROGRAM_OPTIONS_LIBRARY})
  target_link_libraries(vsfontcompiler ${BOOST_SYSTEM_LIBRARY})
  target_link_libraries(vsfontcompiler ${BOOST_THREAD_LIBRARY})
endif()
target_link_libraries(vsfontcompiler general ${FREETYPE_LIBRARY})
target_link_libraries(vsfontcompiler general ${PNG_LIBRARY})
