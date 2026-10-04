// SPDX-FileCopyrightText: 2018 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "font/ttf_font_manager.hpp"

#include <ft2build.h>

#include "font/ttf_font.hpp"

namespace wstdisplay {

TTFFontManager::TTFFontManager() :
  m_freetype()
{
  FT_Error error;

  error = FT_Init_FreeType(&m_freetype);
  if (error) {
    throw std::runtime_error( "could not initialize FreeType" );
  }
}

TTFFontManager::~TTFFontManager()
{
  FT_Done_FreeType(m_freetype);
}

std::unique_ptr<TTFFont>
TTFFontManager::create_font(std::filesystem::path const& filename, int size, const FontEffect& effect)
{
  return std::make_unique<TTFFont>(*this, filename, size, effect);
}

} // namespace wstdisplay

/* EOF */
