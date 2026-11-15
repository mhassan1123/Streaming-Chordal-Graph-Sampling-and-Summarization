cmake_minimum_required(VERSION 3.10)
project(ChordalGraphMCMC CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include_directories(include)

# Everything except main.cpp goes into a library shared by both executables.
file(GLOB CORE_SOURCES "src/*.cpp")
list(REMOVE_ITEM CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
add_library(chordal_core STATIC ${CORE_SOURCES})

# Original console demo
add_executable(ChordalGraphMCMC src/main.cpp)
target_link_libraries(ChordalGraphMCMC chordal_core)

# JSON engine used by the web server
add_executable(chordal_api api/engine.cpp)
target_link_libraries(chordal_api chordal_core)
