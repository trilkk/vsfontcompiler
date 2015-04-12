cmake_minimum_required(VERSION 2.6)

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

add_definitions(-DVERSION="r31")

add_subdirectory(src)
