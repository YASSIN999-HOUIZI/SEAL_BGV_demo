cmake_minimum_required(VERSION 3.22.1)

project(seal)
# Check for C++17 support
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
if (COMPILER_SUPPORTS_CXX17)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++17 support. Please use a different C++ compiler.")
endif()

include_directories(${CMAKE_SOURCE_DIR}/include/)

add_library(seal STATIC IMPORTED)
set_target_properties(seal PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libseal-4.1.a)

add_library(sealmain SHARED test.cpp)
find_library(log-lib log)

target_link_libraries(sealmain seal ${log-lib})
