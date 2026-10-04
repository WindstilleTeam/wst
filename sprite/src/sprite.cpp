// SPDX-FileCopyrightText: 2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sprite.hpp"

namespace wstsprite {

Sprite::Sprite(SpriteDataPtr data) :
  m_data(std::move(data)),
  m_state()
{
}

bool
Sprite::set_action(std::string_view name)
{
  return wstsprite::set_action(m_state, *m_data, name);
}

std::string const&
Sprite::get_action() const
{
  return current_action(m_state, *m_data).name;
}

void
Sprite::update(float delta, std::vector<FrameEvent const*>* events)
{
  advance(m_state, *m_data, delta, events);
}

void
Sprite::draw(wstdisplay::Canvas& canvas, geom::fpoint const& pos) const
{
  draw(canvas, wstdisplay::DrawParams().set_pos(pos));
}

void
Sprite::draw(wstdisplay::Canvas& canvas, wstdisplay::DrawParams const& params) const
{
  wstsprite::draw(canvas, *m_data, m_state, params);
}

} // namespace wstsprite

/* EOF */
