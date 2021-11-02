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

#include "component.hpp"

namespace wstgui {

Component::Component(Component* parent) :
  m_parent(parent),
  m_rect(),
  m_prefered_size(),
  m_active(false)
{
}

Component::Component(const geom::frect& rect, Component* parent) :
  m_parent(parent),
  m_rect(rect),
  m_prefered_size(),
  m_active(false)
{
}

Component::~Component()
{
}

void
Component::set_parent(Component* parent)
{
  m_parent = parent;
}

bool
Component::is_active() const
{
  return m_active;
}

void
Component::set_active(bool active)
{
  if (!m_active && active) {
    on_activation();
  }
  m_active = active;
}

geom::frect
Component::get_screen_rect() const
{
  return m_rect;
}

void
Component::set_screen_rect(const geom::frect& rect)
{
  m_rect = rect;
}

geom::fsize
Component::get_prefered_size() const
{
  return m_prefered_size;
}

Style&
Component::get_style() const
{
  return m_parent->get_style();
}

} // namespace wstgui

/* EOF */
