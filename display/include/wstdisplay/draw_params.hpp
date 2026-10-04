// SPDX-FileCopyrightText: 2002-2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HEADER_WSTDISPLAY_DRAW_PARAMS_HPP
#define HEADER_WSTDISPLAY_DRAW_PARAMS_HPP

#include <glm/glm.hpp>
#include <geom/origin.hpp>
#include <geom/offset.hpp>
#include <geom/point.hpp>
#include <surf/color.hpp>

#include "blend.hpp"
#include "fwd.hpp"

namespace wstdisplay {

/** Parameters for drawing a Surface. The anchor point of the surface
    is placed at \a pos, scaling and rotation happen around it. */
struct DrawParams
{
  geom::fpoint pos = {};
  geom::origin anchor = geom::origin::TOP_LEFT;

  /** Additional offset of the surface relative to the anchor in
      unscaled surface pixels, scaled and rotated with the surface.
      Use TOP_LEFT and offset = -pivot for an arbitrary pivot point. */
  geom::foffset offset = {};
  glm::vec2 scale = {1.0f, 1.0f};

  /** Rotation in degrees, clockwise on screen */
  float angle = 0.0f;

  surf::Color color = {1.0f, 1.0f, 1.0f, 1.0f};
  Blend blend = Blend::Alpha;
  bool hflip = false;
  bool vflip = false;

  /** Custom shader, nullptr for the default one. Must stay alive
      until the canvas is rendered. */
  Material const* material = nullptr;

  DrawParams& set_pos(geom::fpoint const& p) { pos = p; return *this; }
  DrawParams& set_anchor(geom::origin a) { anchor = a; return *this; }
  DrawParams& set_offset(geom::foffset const& o) { offset = o; return *this; }
  DrawParams& set_scale(float s) { scale = {s, s}; return *this; }
  DrawParams& set_scale(glm::vec2 const& s) { scale = s; return *this; }
  DrawParams& set_angle(float degrees) { angle = degrees; return *this; }
  DrawParams& set_color(surf::Color const& c) { color = c; return *this; }
  DrawParams& set_alpha(float a) { color.a = a; return *this; }
  DrawParams& set_blend(Blend b) { blend = b; return *this; }
  DrawParams& set_hflip(bool v) { hflip = v; return *this; }
  DrawParams& set_vflip(bool v) { vflip = v; return *this; }
  DrawParams& set_material(Material const* m) { material = m; return *this; }
};

} // namespace wstdisplay

#endif

/* EOF */
