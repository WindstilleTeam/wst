// SPDX-FileCopyrightText: 2005 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HEADER_WINDSTILLE_FONT_FONT_EFFECT_HPP
#define HEADER_WINDSTILLE_FONT_FONT_EFFECT_HPP

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <memory>

#include <surf/software_surface.hpp>

namespace wstdisplay {

/**
 *  The FontEffect class manages the blitting from a glyph bitmap to
 *  the SDL_Surface, it allows to apply different kinds of effects to
 *  a font, such as adding a border around it.
 */
class FontEffect
{
private:
public:
  FontEffect() {}
  virtual ~FontEffect() {}

  virtual std::unique_ptr<FontEffect> clone() const = 0;

  /** Returns the new height of the font after its transformation */
  virtual int get_height(int orig_font_size) const =0;

  virtual int get_glyph_width(int orig_glyph_width) const =0;
  virtual int get_glyph_height(int orig_glyph_height) const =0;

  virtual int get_x_offset(int orig_glyph_offset) const =0;
  virtual int get_y_offset(int orig_glyph_offset) const =0;

  virtual void blit(surf::SoftwareSurface& target, const FT_Bitmap& brush, int x_pos, int y_pos) const =0;
};

} // namespace wstdisplay

#endif

/* EOF */
