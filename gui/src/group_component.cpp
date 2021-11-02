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

#include <wstdisplay/graphics_context.hpp>

#include "group_component.hpp"
#include "style.hpp"

namespace wstgui {

GroupComponent::GroupComponent(const std::string& title_, Component* parent_)
  : Component(parent_),
    title(title_),
    child()
{

}

GroupComponent::~GroupComponent()
{
}

void
GroupComponent::draw(wstdisplay::GraphicsContext& gc)
{
  gc.fill_rounded_rect(m_rect, 5.0f, surf::Color(0.0f, 0.0f, 0.0f, 0.7f));
  gc.draw_rounded_rect(m_rect, 5.0f, surf::Color(1.0f, 1.0f, 1.0f, 0.5f));

  if (!title.empty())
  {
    wstdisplay::TTFFont* font = get_style().get_font();
    font->draw_center(gc,
                      glm::vec2(m_rect.left() + m_rect.width() / 2.0f,
                                m_rect.top()  + static_cast<float>(font->get_height()) + 5.0f),
                      title, surf::Color(1.0f, 1.0f, 1.0f));

    gc.fill_rect(geom::frect(m_rect.left()  + 8.0f, m_rect.top() + static_cast<float>(font->get_height()) + 16.0f,
                             m_rect.right() - 8.0f, m_rect.top() + static_cast<float>(font->get_height()) + 18.0f),
                 surf::Color(1.0f, 1.0f, 1.0f, 0.5f));
  }

  if (child)
    child->draw(gc);
}

void
GroupComponent::update(float delta, const Controller& controller)
{
  if (child)
    child->update(delta, controller);
}

void
GroupComponent::pack(std::unique_ptr<Component> component)
{
  child = std::move(component);

  child->set_screen_rect(get_child_rect());
  child->set_active(true);
}

geom::frect
GroupComponent::get_child_rect() const
{
  float padding = 6.0f;

  return geom::frect(m_rect.left() + padding,
                     m_rect.top() + padding + (title.empty() ?
                                               0.0f :
                                               static_cast<float>(get_style().get_font()->get_height()) + 18.0f),
                     m_rect.right()  - padding,
                     m_rect.bottom() - padding);
}

bool
GroupComponent::is_active() const
{
  if (child)
    return child->is_active();
  else
    return false;
}

} // namespace wstgui

/* EOF */
