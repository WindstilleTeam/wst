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
project(windstille VERSION 0.3.0)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/external/cmake-modules/)

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
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_search_module(SDL2 REQUIRED sdl2 IMPORTED_TARGET)
pkg_search_module(SDL2_IMAGE REQUIRED SDL2_image IMPORTED_TARGET)

# Build dependencies
function(build_dependencies)
  set(BUILD_TESTS OFF)
  set(BUILD_EXTRA OFF)
  # add_subdirectory(external/argparser/ EXCLUDE_FROM_ALL)
  # add_subdirectory(external/babyxml/ EXCLUDE_FROM_ALL)
  add_subdirectory(external/geomcpp/ EXCLUDE_FROM_ALL)
  # add_subdirectory(external/logmich/ EXCLUDE_FROM_ALL)
  # add_subdirectory(external/sexp-cpp/ EXCLUDE_FROM_ALL)
  # add_subdirectory(external/biiocpp/ EXCLUDE_FROM_ALL)
  # add_subdirectory(external/priocpp/ EXCLUDE_FROM_ALL)
  # add_subdirectory(external/miniswig/ EXCLUDE_FROM_ALL)
  add_subdirectory(external/surfcpp/ EXCLUDE_FROM_ALL)

  # add_subdirectory(external/wstinput/ EXCLUDE_FROM_ALL)
  # add_subdirectory(external/wstsound/ EXCLUDE_FROM_ALL)
endfunction()
build_dependencies()

file(GLOB WSTDISPLAY_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  src/display/*.cpp
  src/scenegraph/*.cpp)
add_library(wstdisplay STATIC ${WSTDISPLAY_SOURCES})
target_include_directories(wstdisplay PRIVATE src/)
target_include_directories(wstdisplay PUBLIC include/)
target_compile_options(wstdisplay PUBLIC ${WARNINGS_CXX_FLAGS})
target_link_libraries(wstdisplay PUBLIC
  geomcpp
  logmich
  surf
  GLEW::GLEW
  JPEG::JPEG
  OpenGL::GL
  OpenGL::GLU
  PNG::PNG
  PkgConfig::SDL2
  PkgConfig::SDL2_IMAGE
  Threads::Threads)

if(BUILD_TESTS)
  file(GLOB TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    test/*_test.cpp)
  foreach(SRC ${TEST_SOURCES})
    get_filename_component(SRC_BASENAME ${SRC} NAME_WE)
    add_executable(${SRC_BASENAME} ${SRC})
    target_link_libraries(${SRC_BASENAME} wstlib)
  endforeach(SRC)
endif()

if(BUILD_EXTRA)
  file(GLOB EXTRA_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    extra/*.cpp)
  foreach(SRC ${EXTRA_SOURCES})
    get_filename_component(SRC_BASENAME ${SRC} NAME_WE)
    add_executable(${SRC_BASENAME} ${SRC})
    target_link_libraries(${SRC_BASENAME} wstlib)
  endforeach(SRC)
endif()

# EOF #
