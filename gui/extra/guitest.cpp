/*
**  Windstille - A Sci-Fi Action-Adventure Game
**  Copyright (C) 2021 Ingo Ruhnke <grumbel@gmail.com>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <SDL.h>

#include <surf/palette.hpp>
#include <wstdisplay/opengl_window.hpp>
#include <wstdisplay/font/ttf_font_manager.hpp>
#include <wstgui/gui_manager.hpp>
#include <wstgui/root_component.hpp>
#include <wstgui/label.hpp>
#include <wstgui/style.hpp>
#include <wstinput/controller.hpp>

using namespace wstinput;
using namespace wstdisplay;
using namespace gui;

int main()
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::ostringstream msg;
    msg << "Couldn't initialize SDL: " << SDL_GetError();
    throw std::runtime_error(msg.str());
  }
  atexit(SDL_Quit);

  OpenGLWindow window("my window", geom::isize(640, 480), geom::isize(640, 480));

  TTFFontManager ttf_font_manager;
  std::unique_ptr<TTFFont> font = ttf_font_manager.create_font("external/wstdisplay/extra/Vera.ttf", 20);
  Style style(font.get());
  GUIManager gui_manager(style);
  auto* root = gui_manager.get_root();
  Label label("Hello World", root);
  label.set_screen_rect(geom::frect(100, 100, 500, 300));
  root->add_child(&label);

  bool quit = false;
  while (!quit) {
    // process input
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch(event.type)
      {
        case SDL_QUIT:
          // FIXME: This should be a bit more gentle, but will do for now
          std::cout << "Ctrl-c or Window-close pressed, game is going to quit" << std::endl;
          quit = true;
          break;

        case SDL_KEYDOWN:
          switch (event.key.keysym.sym)
          {
            case SDLK_ESCAPE:
              quit = true;
              break;
          }
          break;
      }
    }

    auto& gc = window.get_gc();

    gc.clear(surf::palette::black);
    gui_manager.draw(gc);
    window.swap_buffers();

    gui_manager.update(1.0f / 60, Controller());
  }

  return 0;
}

/* EOF */
