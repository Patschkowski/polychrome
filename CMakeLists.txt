cmake_minimum_required(VERSION 3.26)

project(Polychrome)

find_package(Doxygen)

add_executable(polychrome WIN32 src/main.cpp)
target_include_directories(polychrome SYSTEM PRIVATE inc)
target_compile_features(polychrome PRIVATE cxx_std_23)
target_compile_definitions(polychrome PRIVATE _UNICODE UNICODE)
target_compile_options(polychrome PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/WX /W4 /EHsc>)
target_link_libraries(polychrome PRIVATE opengl32)

# https://github.com/ekcoh/cpp-coverage/blob/master/cmake/cpp_coverage.cmake
#include(CTest)
#enable_testing()

#if(DOXYGEN_FOUND)
#  set(DOXYGEN_EXCLUDE_PATTERNS */out/* */.vs/*)
#  set(DOXYGEN_PLANTUML_JAR_PATH $ENV{PLANTUML_JAR_PATH})
#  doxygen_add_docs(doxygen ${CMAKE_CURRENT_SOURCE_DIR} ALL)
#endif()