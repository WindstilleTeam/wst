// Windstille Display Library
// Copyright (C) 2002-2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "texture_manager.hpp"

#include <exception>

#include <logmich/log.hpp>
#include <surf/software_surface.hpp>

namespace wstdisplay {

TextureManager::TextureManager() :
  m_textures(),
  m_fallback()
{
}

TextureManager::~TextureManager()
{
}

TexturePtr
TextureManager::get(std::filesystem::path const& filename, TextureParams const& params)
{
  Key const key{filename, static_cast<int>(params.filter) |
                          (static_cast<int>(params.wrap_x) << 4) |
                          (static_cast<int>(params.wrap_y) << 8)};
  auto it = m_textures.find(key);
  if (it != m_textures.end()) {
    return it->second;
  }

  try {
    TexturePtr texture = Texture::create(surf::SoftwareSurface::from_file(filename), params);
    m_textures[key] = texture;
    return texture;
  } catch (std::exception const& err) {
    if (!m_fallback) {
      throw;
    }
    log_error("TextureManager: {}", err.what());
    return m_fallback;
  }
}

void
TextureManager::set_fallback(std::filesystem::path const& filename)
{
  m_fallback = Texture::create(surf::SoftwareSurface::from_file(filename));
}

void
TextureManager::cleanup()
{
  std::erase_if(m_textures, [](auto const& item) { return item.second.use_count() <= 1; });
}

} // namespace wstdisplay

/* EOF */
