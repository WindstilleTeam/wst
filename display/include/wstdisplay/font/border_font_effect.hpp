// SPDX-FileCopyrightText: 2005 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HEADER_WINDSTILLE_FONT_BORDER_FONT_EFFECT_HPP
#define HEADER_WINDSTILLE_FONT_BORDER_FONT_EFFECT_HPP

#include "font_effect.hpp"

namespace wstdisplay {

class BorderFontEffect : public FontEffect
{
private:
  int  size;
  bool outline;

public:
  /** Create a pixel border around the font

      @param size     geom::isize of the border in pixel
      @param outline  If true add the border as an outline to the font, else add it to the font itself
  */
  BorderFontEffect(int size, bool outline);
  ~BorderFontEffect() override;

  std::unique_ptr<FontEffect> clone() const override { return std::make_unique<BorderFontEffect>(*this); }

  int get_height(int orig_font_size) const override;

  int get_glyph_width(int orig_glyph_width) const override;
  int get_glyph_height(int orig_glyph_height) const override;

  int get_x_offset(int orig_glyph_offset) const override;
  int get_y_offset(int orig_glyph_offset) const override;

  void blit(surf::SoftwareSurface& target, const FT_Bitmap& brush, int x_pos, int y_pos) const override;

  /* disabled for g++-4.2 compatibilty
     private:
     BorderFontEffect (const BorderFontEffect&);
     BorderFontEffect& operator= (const BorderFontEffect&);
  */
};

} // namespace wstdisplay

#endif

/* EOF */
