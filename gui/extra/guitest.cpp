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
#include <wstdisplay/font/ttf_font_manager.hpp>
#include <wstdisplay/opengl_window.hpp>
#include <wstgui/button.hpp>
#include <wstgui/gui_manager.hpp>
#include <wstgui/label.hpp>
#include <wstgui/menu.hpp>
#include <wstgui/root_component.hpp>
#include <wstgui/style.hpp>
#include <wstgui/text_view.hpp>
#include <wstinput/controller.hpp>

using namespace wstinput;
using namespace wstdisplay;
using namespace wstgui;

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
  std::unique_ptr<TTFFont> smallfont = ttf_font_manager.create_font("external/wstdisplay/extra/Vera.ttf", 16);
  Style style(font.get());
  GUIManager gui_manager(style);
  auto* root = gui_manager.get_root();

  auto label = std::make_unique<Label>("GUI Test", root);
  label->set_screen_rect(geom::frect(20, 20, 100, 50));
  root->add_child(std::move(label));

  auto textview = std::make_unique<TextView>(geom::frect(20, 50, 620, 400), root);
  textview->set_font(smallfont.get());
  textview->set_text(
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Morbi in "
    "consectetur sapien. Pellentesque a cursus est. Vestibulum nec "
    "ultricies sapien, et tristique dui. Donec pulvinar turpis sed risus "
    "eleifend, eget malesuada tortor lacinia. Aliquam ut dolor purus. "
    "Mauris porta diam a iaculis aliquet. Phasellus maximus arcu sed nisi "
    "efficitur viverra. Donec sed libero sed dui varius sollicitudin eu non "
    "elit. Quisque eu arcu ornare, feugiat urna non, consectetur odio. Duis "
    "lobortis et leo at finibus. Aliquam condimentum ligula id nulla "
    "facilisis mollis.\n"
    "\n"
    "Duis vulputate consequat ipsum in suscipit. Nunc porttitor convallis "
    "eros, sit amet gravida justo lobortis nec. Vivamus vehicula velit eu "
    "nisl fringilla, at posuere sem placerat. Class aptent taciti sociosqu "
    "ad litora torquent per conubia nostra, per inceptos himenaeos. Proin "
    "vel magna risus. Integer ullamcorper, dui eu fringilla sollicitudin, "
    "nisi lorem varius massa, sit amet vehicula sem lorem nec nulla. Nam "
    "volutpat condimentum mi in vestibulum. Maecenas malesuada risus non "
    "lectus egestas blandit. Nullam convallis nunc quis tellus vulputate "
    "congue. Cras id diam vitae nisi rhoncus congue. Sed nisi neque, ornare "
    "a leo et, vehicula condimentum sapien. Maecenas maximus consequat "
    "tellus vel condimentum. Maecenas imperdiet elit quis enim auctor "
    "lacinia.\n"
    "\n"
    "Phasellus vel suscipit massa. Phasellus in magna nec magna placerat "
    "egestas. Nullam id condimentum lacus, ac bibendum est. Donec sit amet "
    "facilisis tellus. Ut molestie non tortor sit amet porta. Duis feugiat "
    "scelerisque ipsum non hendrerit. Aenean vel odio faucibus, pharetra "
    "felis id, pharetra felis. Donec tempus augue lorem, at suscipit tortor "
    "aliquet et. Phasellus molestie justo in odio dignissim, sit amet "
    "molestie ex convallis. Suspendisse vitae tortor sapien.\n"
    );
  root->add_child(std::move(textview));

  auto ok_button = std::make_unique<Button>("Ok", root);
  auto cancel_button = std::make_unique<Button>("Cancel", root);
  ok_button->set_screen_rect(geom::frect(500, 420, 600, 460));
  cancel_button->set_screen_rect(geom::frect(380, 420, 480, 460));
  root->add_child(std::move(ok_button));
  root->add_child(std::move(cancel_button));

  auto menu = std::make_unique<wstgui::Menu>("Options", geom::frect(geom::fpoint(100, 100), geom::fsize(500, 340)), style);
  menu->add_slider("Master Volume",  50, 0, 100, 10, {});
  menu->add_slider("Music Volume", 75, 100, 10, {});
  menu->add_slider("Sound Volume", 100, 0, 100, 10, {});
  menu->add_slider("Voice Volume", 50, 0, 100, 10, {});
  //root->add_child(std::move(menu));

  //ScreenManager screen_manager;
  //screen_manager.run();

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
    menu->get_root()->draw(gc);
    window.swap_buffers();

    gui_manager.update(1.0f / 60, Controller());
  }

  return 0;
}

/* EOF */
