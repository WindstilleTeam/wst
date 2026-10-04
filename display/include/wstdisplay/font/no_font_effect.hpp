// SPDX-FileCopyrightText: 2005 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HEADER_WINDSTILLE_FONT_NO_FONT_EFFECT_HPP
#define HEADER_WINDSTILLE_FONT_NO_FONT_EFFECT_HPP

#include "font_effect.hpp"

namespace wstdisplay {

class NoFontEffect : public FontEffect
{
private:
public:
  NoFontEffect() {}
  ~NoFontEffect() override {}

  std::unique_ptr<FontEffect> clone() const override { return std::make_unique<NoFontEffect>(*this); }

  int get_height(int orig_font_size) const override { return orig_font_size; }

  int get_glyph_width(int orig_glyph_width) const override   { return orig_glyph_width; }
  int get_glyph_height(int orig_glyph_height) const override { return orig_glyph_height; }

  int get_x_offset(int orig_glyph_offset) const override { return orig_glyph_offset; }
  int get_y_offset(int orig_glyph_offset) const override { return orig_glyph_offset; }

  void blit(surf::SoftwareSurface& target, const FT_Bitmap& brush, int x_pos, int y_pos) const override;
};

} // namespace wstdisplay

#endif

/* EOF */
