cmake_minimum_required(VERSION 3.16)
project(cpp_chess)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)



# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")


# -----------------------
# Engine library
# -----------------------
# This assumes the SDL source is available in vendored/SDL
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
# GLM (header-only)
add_subdirectory(vendored/glm EXCLUDE_FROM_ALL)


file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/*.cpp"
)
add_library(engine STATIC ${ENGINE_SOURCES})

target_include_directories(engine PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendored/EnTT/single_include"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendored/glm"
)

target_link_libraries(engine PUBLIC
    SDL3::SDL3
)

# -----------------------
# Game executable
# -----------------------
file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/game/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
)
add_executable(cpp_chess ${GAME_SOURCES})

target_include_directories(cpp_chess PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(cpp_chess PRIVATE
    engine
)
