// SPDX-FileCopyrightText: 2018 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HEADER_WINDSTILLE_FONT_TTF_FONT_MANAGER_HPP
#define HEADER_WINDSTILLE_FONT_TTF_FONT_MANAGER_HPP

#include <memory>
#include <filesystem>

#include <wstdisplay/font/no_font_effect.hpp>

namespace wstdisplay {

class Device;
class FontEffect;
class TTFFont;

/** Owns the FreeType library and creates fonts whose glyph pages
    live on \a device */
class TTFFontManager
{
public:
  explicit TTFFontManager(Device& device);
  ~TTFFontManager();

  std::unique_ptr<TTFFont> create_font(std::filesystem::path const& filename, int size, const FontEffect& effect = NoFontEffect());

  FT_Library get_handle() const { return m_freetype; }
  Device& get_device() const { return m_device; }

private:
  Device& m_device;
  FT_Library m_freetype;

private:
  TTFFontManager(const TTFFontManager&) = delete;
  TTFFontManager& operator=(const TTFFontManager&) = delete;
};

} // namespace wstdisplay

#endif

/* EOF */
