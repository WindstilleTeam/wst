# Windstille - A Sci-Fi Action-Adventure Game
# Copyright (C) 2015 Ingo Ruhnke <grumbel@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.15)
project(wstdisplay VERSION 0.3.0)

set(TINYCMMC_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/external/cmake-modules/modules/")
find_package(tinycmmc CONFIG)
message(STATUS "tinycmmc module path: ${TINYCMMC_MODULE_PATH}")
list(APPEND CMAKE_MODULE_PATH ${TINYCMMC_MODULE_PATH})

include(GetProjectVersion)
include(GNUInstallDirs)
include(CheckIncludeFile)
include(ClangTidy)
include(MaximumWarnings)
include(InstallWithWrapper)

option(BUILD_TESTS "Build test cases" OFF)
option(BUILD_EXTRA "Build extra stuff" OFF)

if (BUILD_TESTS)
  # add 'make test' target, use 'make test ARGS="-V"' or 'ctest -V' for verbose
  enable_testing()
endif(BUILD_TESTS)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_search_module(SDL2 REQUIRED sdl2 IMPORTED_TARGET)
pkg_search_module(SIGCXX REQUIRED sigc++-2.0 IMPORTED_TARGET)

# Build dependencies
function(build_dependencies)
  set(BUILD_TESTS OFF)
  set(BUILD_EXTRA OFF)

  find_package(babyxml QUIET)
  if(NOT TARGET babyxml)
    add_subdirectory(external/babyxml/ EXCLUDE_FROM_ALL)
  endif()

  find_package(geomcpp QUIET)
  if(NOT TARGET geomcpp)
    add_subdirectory(external/geomcpp/ EXCLUDE_FROM_ALL)
  endif()

  find_package(surfcpp QUIET)
  if(NOT TARGET surf)
    add_subdirectory(external/surfcpp/ EXCLUDE_FROM_ALL)
  endif()

  find_package(logmich QUIET)
  if(NOT TARGET logmich)
    add_subdirectory(external/logmich/ EXCLUDE_FROM_ALL)
  endif()
endfunction()
build_dependencies()

# file(GLOB WSTDISPLAY_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
#   include/wstdisplay/*.hpp
#   include/wstdisplay/font/*.hpp
#   include/wstdisplay/scenegraph/*.hpp
#   include/wstsystem/*.hpp
#   )
file(GLOB WSTDISPLAY_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  src/*.cpp
  src/font/*.?pp
  src/scenegraph/*.cpp
  )
add_library(wstdisplay STATIC ${WSTDISPLAY_SOURCES})
#set_target_properties(wstdisplay PROPERTIES
#  PUBLIC_HEADER "${WSTDISPLAY_HEADERS}")
target_include_directories(wstdisplay PRIVATE src/ include/wstdisplay include/wstsystem/)
target_include_directories(wstdisplay PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  )
target_compile_options(wstdisplay PRIVATE ${WARNINGS_CXX_FLAGS})
target_link_libraries(wstdisplay PUBLIC
  babyxml
  geomcpp
  surf
  logmich
  GLEW::GLEW
  OpenGL::GL
  OpenGL::GLU
  Freetype::Freetype
  PkgConfig::SDL2
  PkgConfig::SIGCXX
  Threads::Threads)

install(TARGETS wstdisplay
  EXPORT wstdisplay-targets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  # PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}"
  )

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/"
  FILES_MATCHING
  PATTERN "*.hpp"
  )

# CMake integration
configure_file(wstdisplay-config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/wstdisplay-config.cmake" @ONLY)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/wstdisplay-config.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/wstdisplay/")
export(TARGETS wstdisplay
  FILE "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/wstdisplay-targets.cmake")
export(PACKAGE wstdisplay)
install(EXPORT wstdisplay-targets
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/wstdisplay/")

if(BUILD_TESTS)
  file(GLOB TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    test/*_test.cpp)
  foreach(SRC ${TEST_SOURCES})
    get_filename_component(SRC_BASENAME ${SRC} NAME_WE)
    add_executable(${SRC_BASENAME} ${SRC})
    target_link_libraries(${SRC_BASENAME} PRIVATE wstdisplay)
  endforeach(SRC)
endif()

if(BUILD_EXTRA)
  file(GLOB EXTRA_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    extra/*.cpp)
  foreach(SRC ${EXTRA_SOURCES})
    get_filename_component(SRC_BASENAME ${SRC} NAME_WE)
    add_executable(${SRC_BASENAME} ${SRC})
    target_link_libraries(${SRC_BASENAME} PRIVATE wstdisplay)
    install(TARGETS ${SRC_BASENAME}
      RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
  endforeach(SRC)
endif()

# EOF #
