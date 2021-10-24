/*
**  Windstille - A Sci-Fi Action-Adventure Game
**  Copyright (C) 2005 Ingo Ruhnke <grumbel@gmail.com>
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

#include "gui_manager.hpp"
#include "root_component.hpp"

namespace wstgui {

GUIManager::GUIManager(Style& style) :
  root(new RootComponent(geom::frect(0,0,800,600), style)),
  m_style(style)
{
}

GUIManager::~GUIManager()
{
}

void
GUIManager::draw(wstdisplay::GraphicsContext& gc)
{
  root->draw(gc);
}

void
GUIManager::update(float delta, const Controller& controller)
{
  root->update(delta, controller);

  if (!root->is_active()) {
    finish();
  }
}

RootComponent*
GUIManager::get_root() const
{
  return root.get();
}

} // namespace wstgui

/* EOF */
