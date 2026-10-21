cmake_minimum_required(VERSION 3.16)

# VitaSDK toolchain (set automatically inside the official Docker image).
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "VITASDK is not set. Use scripts/build-docker.sh or export VITASDK=/usr/local/vitasdk")
  endif()
endif()

project(tiny-loop C)

include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "tiny-loop")
set(VITA_TITLEID "TINY00001")
set(VITA_VERSION "01.00")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -Wall -O2")

add_executable(${PROJECT_NAME}
  src/main.c
)

# Official image ships SDL2 via the sdl2 or sdl2_vitagl package.
set(SDL2_DIR "$ENV{VITASDK}/arm-vita-eabi/lib/cmake/SDL2")
find_package(SDL2 REQUIRED)

# SDL2::SDL2 pulls include paths + Vita stubs (and vitaGL if that backend is installed).
target_link_libraries(${PROJECT_NAME} PRIVATE
  SDL2::SDL2
)

# Extended USER_RW memory (+~109 MiB) — typical for games.
set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d ATTRIBUTE2=12")

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
)
