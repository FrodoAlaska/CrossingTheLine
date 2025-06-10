cmake_minimum_required(VERSION 3.27)
project(game)

### FetchContent ###
############################################################
include(FetchContent)

set(NIKOLA_BUILD_TESTBED OFF)
set(NIKOLA_BUILD_NBR     OFF)
set(qu3e_build_demo      OFF)

FetchContent_Declare(
  nikola
  GIT_REPOSITORY https://github.com/FrodoAlaska/Nikola.git 
)

FetchContent_MakeAvailable(nikola)
############################################################

### Project variables ###
############################################################
set(PROJECT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(PROJECT_LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)

set(PROJECT_INCLUDES 
  ${PROJECT_INCLUDE_DIR}
  ${PROJECT_LIBS_DIR}
  
  ${nikola_SOURCE_DIR}/nikola/include
  ${nikola_SOURCE_DIR}/libs
  ${glfw_SOURCE_DIR}/include
  ${glm_SOURCE_DIR}
)

include(cmake/variables.cmake)
############################################################

### CMake variables ###
############################################################
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
############################################################

### Project sources ###
############################################################
set(PROJECT_SOURCES
  # Core
  ${PROJECT_SRC_DIR}/app.cpp
  ${PROJECT_SRC_DIR}/main.cpp

  # Entities
  ${PROJECT_SRC_DIR}/entities/entity.cpp
  ${PROJECT_SRC_DIR}/entities/player.cpp
  ${PROJECT_SRC_DIR}/entities/vehicle.cpp
  ${PROJECT_SRC_DIR}/entities/tile.cpp
  ${PROJECT_SRC_DIR}/entities/entity_manager.cpp
  
  # Level
  ${PROJECT_SRC_DIR}/levels/level.cpp
  ${PROJECT_SRC_DIR}/levels/nklvl.cpp
)
############################################################

### Library sources ###
############################################################
############################################################

### Targets ###
############################################################
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
############################################################

### Linking ###
############################################################
target_include_directories(${PROJECT_NAME} PRIVATE BEFORE ${PROJECT_INCLUDES})
target_link_libraries(${PROJECT_NAME} PRIVATE nikola)

target_precompile_headers(${PROJECT_NAME} PRIVATE 
  "$<$<COMPILE_LANGUAGE:CXX>:${nikola_SOURCE_DIR}/nikola/include/nikola/nikola.h>"
)
############################################################

### Compiling options ###
############################################################
target_compile_options(${PROJECT_NAME} PUBLIC ${PROJECT_BUILD_FLAGS})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_compile_definitions(${PROJECT_NAME} PUBLIC ${PROJECT_BUILD_DEFINITIONS})
############################################################
