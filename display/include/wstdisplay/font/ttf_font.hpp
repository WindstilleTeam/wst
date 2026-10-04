// SPDX-FileCopyrightText: 2005-2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HEADER_WSTDISPLAY_FONT_TTF_FONT_HPP
#define HEADER_WSTDISPLAY_FONT_TTF_FONT_HPP

#include <filesystem>
#include <memory>

#include "font.hpp"
#include "no_font_effect.hpp"

namespace wstdisplay {

class FontEffect;
class TTFFontImpl;
class TTFFontManager;

/** A TrueType font rendered with FreeType. Glyphs are rasterized on
    first use into atlas pages, Latin-1 is prepared up front. */
class TTFFont final : public Font
{
public:
  TTFFont(TTFFontManager& mgr, std::filesystem::path const& file, int size,
          FontEffect const& effect = NoFontEffect());
  ~TTFFont() override;

  Glyph const* get_glyph(char32_t codepoint) const override;

  /** The size given in the constructor, adjusted by the FontEffect */
  float get_height() const override;
  float get_line_height() const override;

  /** The size given in the constructor */
  int get_pixel_size() const;

private:
  std::unique_ptr<TTFFontImpl> impl;

public:
  TTFFont(TTFFont const&) = delete;
  TTFFont& operator=(TTFFont const&) = delete;
};

} // namespace wstdisplay

#endif

/* EOF */
