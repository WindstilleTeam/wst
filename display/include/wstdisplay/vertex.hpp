// SPDX-FileCopyrightText: 2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HEADER_WSTDISPLAY_VERTEX_HPP
#define HEADER_WSTDISPLAY_VERTEX_HPP

#include <array>
#include <bit>
#include <cstdint>

#include <surf/color.hpp>

namespace wstdisplay {

/** A color packed into four bytes in R, G, B, A memory order, the
    layout the GPU expects for normalized unsigned byte attributes. */
using PackedColor = uint32_t;

inline PackedColor pack_color(surf::Color const& color)
{
  return std::bit_cast<PackedColor>(std::array<uint8_t, 4>{
      color.r8(), color.g8(), color.b8(), color.a8()});
}

inline surf::Color unpack_color(PackedColor color)
{
  auto const bytes = std::bit_cast<std::array<uint8_t, 4>>(color);
  return surf::Color::from_rgba8888(bytes[0], bytes[1], bytes[2], bytes[3]);
}

inline constexpr PackedColor packed_white = 0xffffffffu;

/** The single vertex format used for all 2D drawing: position,
    texture coordinate and color, 20 bytes. Positions are in canvas
    coordinates with the canvas transform already applied. */
struct Vertex
{
  float x;
  float y;
  float u;
  float v;
  PackedColor color;
};

static_assert(sizeof(Vertex) == 20);

} // namespace wstdisplay

#endif

/* EOF */
