cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER "clang-16")
set(C_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Testosterone VERSION 1.0)

file(GLOB_RECURSE srcFiles src/*.c)
add_executable(${PROJECT_NAME} ${srcFiles})

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<C_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(raylib REQUIRED PATHS cmake)
target_link_libraries(${PROJECT_NAME} raylib)
target_link_libraries(${PROJECT_NAME} m)
