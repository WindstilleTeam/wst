/*
**  Windstille - A Sci-Fi Action-Adventure Game
**  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

#include <functional>

#include "group_component.hpp"
#include "gui_manager.hpp"
#include "menu.hpp"
#include "menu_component.hpp"
#include "menu_item.hpp"
#include "root_component.hpp"
#include "screen_manager.hpp"

namespace wstgui {

Menu::Menu(const std::string& name, const geom::frect& rect, Style& style, bool allow_cancel, Component* parent)
  : manager(),
    group(),
    menu()
{
  if (!parent)
  {
    manager.reset(new GUIManager(style));
    parent = manager->get_root();
  }

  group.reset(new GroupComponent(name, parent));
  group->set_screen_rect(rect);
  menu.reset(new MenuComponent(allow_cancel, group.get()));
  menu->set_screen_rect(group->get_child_rect());
  menu->set_font(style.get_font());
}

Menu::~Menu()
{
}

EnumMenuItem&
Menu::add_enum(const std::string& name, int index,
               const std::function<void (int)>& callback)
{
  std::unique_ptr<EnumMenuItem> enum_item(new EnumMenuItem(menu.get(), name, index));

  if (callback)
    enum_item->sig_change().connect(callback);

  EnumMenuItem& obj = *enum_item;
  menu->add_item(std::move(enum_item));
  return obj;
}

void
Menu::add_slider(const std::string& name,
                 int value, int min_value, int max_value, int step,
                 const std::function<void (int)>& callback)
{
  std::unique_ptr<SliderMenuItem> slider(new SliderMenuItem(menu.get(), name, value, min_value, max_value, step));
  if (callback)
    slider->sig_change().connect(callback);
  menu->add_item(std::move(slider));
}

void
Menu::add_button(const std::string& name,
                 const std::function<void ()>& callback)
{
  std::unique_ptr<ButtonMenuItem> scenario_button(new ButtonMenuItem(menu.get(), name));
  if (callback)
    scenario_button->sig_click().connect(callback);
  menu->add_item(std::move(scenario_button));
}

RootComponent*
Menu::get_root() const
{
  assert(manager.get());
  return manager->get_root();
}

void
Menu::show(ScreenManager& screen_manager)
{
  assert(manager.get());

  {
    geom::frect rect = group->get_screen_rect();

    glm::vec2 center((rect.left() + rect.right()) / 2.0f,
                     (rect.top() + rect.bottom()) / 2.0f);

    geom::fsize size(menu->get_prefered_size().width(),
                     group->has_title() ?
                     static_cast<float>(manager->get_style().get_font()->get_height()) + 18.0f :
                     0.0f);

    group->set_screen_rect(geom::frect(glm::vec2(center.x - size.width()/2.0f,
                                                 center.y - size.height()/2.0f),
                                       size));

    menu->set_screen_rect(group->get_child_rect());
  }

  group->pack(menu.release());
  manager->get_root()->add_child(std::move(group));

  screen_manager.push_overlay(std::move(manager));
}

} // namespace wstgui

/* EOF */
