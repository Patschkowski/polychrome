cmake_minimum_required(VERSION 3.26)

project(Polychrome)

find_package(Doxygen)
find_package(WIL REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(polychrome WIN32 "src/main.cpp" "src/rendering.ixx" "src/rendering.cpp")
target_compile_features(polychrome PRIVATE cxx_std_23)
target_compile_definitions(polychrome PRIVATE _UNICODE UNICODE)
target_compile_options(polychrome PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/WX /W4 /EHsc>)
target_link_libraries(polychrome PRIVATE WIL::WIL Vulkan::Vulkan)

# https://github.com/ekcoh/cpp-coverage/blob/master/cmake/cpp_coverage.cmake
#include(CTest)
#enable_testing()

#if(DOXYGEN_FOUND)
#  set(DOXYGEN_EXCLUDE_PATTERNS */out/* */.vs/*)
#  set(DOXYGEN_PLANTUML_JAR_PATH $ENV{PLANTUML_JAR_PATH})
#  doxygen_add_docs(doxygen ${CMAKE_CURRENT_SOURCE_DIR} ALL)
#endif()