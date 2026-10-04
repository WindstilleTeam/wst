// SPDX-FileCopyrightText: 2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HEADER_WSTSPRITE_SPRITE_HPP
#define HEADER_WSTSPRITE_SPRITE_HPP

#include <string_view>
#include <vector>

#include <wstdisplay/canvas.hpp>

#include "anim_state.hpp"
#include "sprite_data.hpp"

namespace wstsprite {

/** Convenience wrapper bundling SpriteData and AnimState, for code that
    doesn't keep them separately in components */
class Sprite final
{
public:
  Sprite() = default;
  explicit Sprite(SpriteDataPtr data);

  explicit operator bool() const { return static_cast<bool>(m_data); }

  bool set_action(std::string_view name);
  std::string const& get_action() const;

  void update(float delta, std::vector<FrameEvent const*>* events = nullptr);
  void restart() { wstsprite::restart(m_state); }

  void draw(wstdisplay::Canvas& canvas, geom::fpoint const& pos) const;
  void draw(wstdisplay::Canvas& canvas, wstdisplay::DrawParams const& params) const;

  bool is_finished() const { return m_state.finished; }

  AnimState& get_state() { return m_state; }
  AnimState const& get_state() const { return m_state; }
  SpriteDataPtr const& get_data() const { return m_data; }

private:
  SpriteDataPtr m_data = {};
  AnimState m_state = {};
};

} // namespace wstsprite

#endif

/* EOF */
