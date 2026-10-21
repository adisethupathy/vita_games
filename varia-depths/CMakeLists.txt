cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "VITASDK is not set. Use scripts/build-docker.sh or export VITASDK=/usr/local/vitasdk")
  endif()
endif()

project(varia-depths C)

include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "Varia Depths")
set(VITA_TITLEID "VARI00001")
set(VITA_VERSION "01.00")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -Wall -O2")

add_executable(${PROJECT_NAME}
  src/main.c
  src/game.c
  src/player.c
  src/world.c
  src/entities.c
  src/render.c
)

set(SDL2_DIR "$ENV{VITASDK}/arm-vita-eabi/lib/cmake/SDL2")
find_package(SDL2 REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PRIVATE
  SDL2::SDL2
  SDL2_image
  png
  jpeg
  webpdemux
  webp
  pthread
  z
  m
)

set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d ATTRIBUTE2=12")

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE assets/player.png assets/player.png
  FILE assets/enemy_crawler.png assets/enemy_crawler.png
  FILE assets/tiles.png assets/tiles.png
  FILE assets/beam.png assets/beam.png
  FILE assets/pickup_morph.png assets/pickup_morph.png
  FILE assets/hud_energy.png assets/hud_energy.png
  FILE assets/bg_gradient.png assets/bg_gradient.png
  FILE assets/bg_silhouettes.png assets/bg_silhouettes.png
)
