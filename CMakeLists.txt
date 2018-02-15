cmake_minimum_required (VERSION 2.6)
project (WKBRE)

add_subdirectory (bzip2)

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-write-strings -fpermissive")
endif(CMAKE_COMPILER_IS_GNUCXX)

option(WKBRE_RELEASE "Build the release version of wkbre without test functions and debugging code." OFF)
if(WKBRE_RELEASE)
	add_definitions(-DWKBRE_RELEASE)
endif(WKBRE_RELEASE)

set(WKBRE_RELEASE_VERSION 0.0.0.0 CACHE STRING "Version string indicated in start screen and about message box.")
add_definitions(-DWKBRE_VERSION=\"${WKBRE_RELEASE_VERSION}\")

file(GLOB WKBRE_SRC_CPP ${WKBRE_SOURCE_DIR}/*.cpp)
file(GLOB WKBRE_SRC_C ${WKBRE_SOURCE_DIR}/*.c)
file(GLOB WKBRE_SRC_GAMESET ${WKBRE_SOURCE_DIR}/gameset/*.cpp)
file(GLOB WKBRE_SRC_PLAY ${WKBRE_SOURCE_DIR}/play/*.cpp)
file(GLOB WKBRE_SRC_SCRIPT ${WKBRE_SOURCE_DIR}/script/*.cpp)
file(GLOB WKBRE_SRC_IMGUI ${WKBRE_SOURCE_DIR}/imgui/*.cpp)

file(GLOB WKBRE_SRC_H ${WKBRE_SOURCE_DIR}/*.h)
file(GLOB WKBRE_SRC_GAMESET_H ${WKBRE_SOURCE_DIR}/gameset/*.h)
file(GLOB WKBRE_SRC_PLAY_H ${WKBRE_SOURCE_DIR}/play/*.h)
file(GLOB WKBRE_SRC_SCRIPT_H ${WKBRE_SOURCE_DIR}/script/*.h)
file(GLOB WKBRE_SRC_IMGUI_H ${WKBRE_SOURCE_DIR}/imgui/*.h)

set (WKBRE_SRC_ALL ${WKBRE_SRC_CPP} ${WKBRE_SRC_C} ${WKBRE_SRC_GAMESET}
 ${WKBRE_SRC_PLAY} ${WKBRE_SRC_SCRIPT} ${WKBRE_SRC_IMGUI})

set (WKBRE_SRC_ALL_H ${WKBRE_SRC_H} ${WKBRE_SRC_GAMESET_H}
 ${WKBRE_SRC_PLAY_H} ${WKBRE_SRC_SCRIPT_H} ${WKBRE_SRC_IMGUI_H})

set (WKBRE_SRC ${WKBRE_SRC_ALL} ${WKBRE_SRC_ALL_H} ${WKBRE_SOURCE_DIR}/wkbre.rc)

add_executable(wkbre ${WKBRE_SRC})
target_link_libraries (wkbre bzip2 winmm d3d9 opengl32 glu32 imm32)