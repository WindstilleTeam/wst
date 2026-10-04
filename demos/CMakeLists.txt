# wst - Windstille game framework
# Copyright (C) 2026 Ingo Ruhnke <grumbel@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

include(GNUInstallDirs)

add_library(wstdemo STATIC common/demo_app.cpp)
target_include_directories(wstdemo PUBLIC common)
target_link_libraries(wstdemo PUBLIC wstdisplay::wstdisplay)
target_compile_definitions(wstdemo PRIVATE
  WST_DEMO_SOURCE_DATADIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
  WST_DEMO_INSTALL_DATADIR="${CMAKE_INSTALL_FULL_DATADIR}/wst/demos")
target_compile_options(wstdemo PRIVATE ${WST_WARNINGS_CXX_FLAGS})

file(GLOB DEMO_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} demo_*.cpp)

# demos using more than wstdisplay
if(TARGET wstgui::wstgui)
  set(DEMO_EXTRA_LIBS_demo_gui wstgui::wstgui)
else()
  list(REMOVE_ITEM DEMO_SOURCES demo_gui.cpp)
endif()

if(TARGET wstsprite::wstsprite)
  set(DEMO_EXTRA_LIBS_demo_sprite_viewer wstsprite::wstsprite)
else()
  list(REMOVE_ITEM DEMO_SOURCES demo_sprite_viewer.cpp)
endif()
foreach(SRC ${DEMO_SOURCES})
  get_filename_component(NAME ${SRC} NAME_WE)
  string(REPLACE "_" "-" EXE_NAME "wst-${NAME}")
  add_executable(${NAME} ${SRC})
  set_target_properties(${NAME} PROPERTIES OUTPUT_NAME ${EXE_NAME})
  target_link_libraries(${NAME} PRIVATE wstdemo ${DEMO_EXTRA_LIBS_${NAME}})
  target_compile_options(${NAME} PRIVATE ${WST_WARNINGS_CXX_FLAGS})
  install(TARGETS ${NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  list(APPEND WST_DEMO_TARGETS ${NAME})
endforeach()

install(DIRECTORY data/ DESTINATION ${CMAKE_INSTALL_DATADIR}/wst/demos)

# Render a few frames of every demo with each API and compare with the
# reference images, needs a display or SDL_VIDEODRIVER=offscreen
if(BUILD_TESTS)
  foreach(NAME ${WST_DEMO_TARGETS})
    foreach(API gl gles)
      add_test(NAME ${NAME}-${API}
        COMMAND ${NAME} --${API} --hidden --size 640x360 --frames 30
                --screenshot ${CMAKE_CURRENT_BINARY_DIR}/${NAME}-${API}.png)
      set_tests_properties(${NAME}-${API} PROPERTIES LABELS "gl")
    endforeach()
  endforeach()
endif()

# EOF #
