include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/src")

set(GFX_SRC gfx/image_png.cpp)

set(THR_SRC thr/dispatch.cpp thr/thr_generic.cpp thr/thread_storage.cpp)

add_executable(vsfontcompiler ${GFX_SRC} ${THR_SRC} ft_face.cpp ft_glyph.cpp ft_library.cpp glyph_range.cpp glyph_storage.cpp main.cpp sky_line.cpp sky_line_fitter.cpp)

target_link_libraries(vsfontcompiler ${BOOST_PROGRAM_OPTIONS_LIBRARY})
target_link_libraries(vsfontcompiler ${BOOST_SYSTEM_LIBRARY})
target_link_libraries(vsfontcompiler ${BOOST_THREAD_LIBRARY})
target_link_libraries(vsfontcompiler ${FREETYPE_LIBRARY})
target_link_libraries(vsfontcompiler ${PNG_LIBRARY})
