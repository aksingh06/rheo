cmake_minimum_required(VERSION 3.23)

project(rheo
    VERSION 1.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM_DIR: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(rheo
    src/main.cpp
    src/Driver/Driver.cpp
)

llvm_map_components_to_libnames(LLVM_LIBS
    Core
    Support
    IRReader
)

target_include_directories(rheo
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(rheo
    PRIVATE
        ${LLVM_LIBS}
)
