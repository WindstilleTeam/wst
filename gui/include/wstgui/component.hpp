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

#ifndef HEADER_WINDSTILLE_GUI_COMPONENT_HPP
#define HEADER_WINDSTILLE_GUI_COMPONENT_HPP

#include <geom/geom.hpp>
#include <wstdisplay/fwd.hpp>

#include "style.hpp"

namespace wstinput { class Controller; }

class FileReader;

using Controller = wstinput::Controller;

namespace gui {

class Component
{
public:
  Component(Component* parent);
  Component(const geom::frect& rect, Component* parent);
  virtual ~Component();

  virtual void set_parent(Component* parent);

  virtual bool is_active() const;
  virtual void set_active(bool a);

  /** Called when the Component receives focus */
  virtual void on_activation() {}

  virtual void draw(wstdisplay::GraphicsContext& gc) =0;
  virtual void update(float delta, const Controller& controller) =0;

  virtual geom::frect get_screen_rect() const;
  virtual void  set_screen_rect(const geom::frect& rect);

  virtual float get_prefered_width() const;
  virtual float get_prefered_height() const;

  virtual Style& get_style() const;

protected:
  geom::frect m_rect;
  Component* m_parent;
  bool m_active;

private:
  Component (const Component&);
  Component& operator= (const Component&);
};

} // namespace gui

#endif

/* EOF */
