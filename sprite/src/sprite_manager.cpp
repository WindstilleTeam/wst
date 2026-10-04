// wst - Windstille game framework
// Copyright (C) 2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sprite_manager.hpp"

#include <wstdisplay/surface_manager.hpp>

namespace wstsprite {

SpriteManager::SpriteManager(wstdisplay::SurfaceManager& surfaces) :
  m_surfaces(surfaces),
  m_data()
{
}

SpriteDataPtr
SpriteManager::get_data(std::filesystem::path const& filename)
{
  auto it = m_data.find(filename);
  if (it != m_data.end()) {
    return it->second;
  }

  SpriteDataPtr data = (filename.extension() == ".sprite") ?
    SpriteData::from_file(filename, m_surfaces) :
    SpriteData::from_surface(m_surfaces.get(filename));

  m_data[filename] = data;
  return data;
}

Sprite
SpriteManager::create(std::filesystem::path const& filename)
{
  return Sprite(get_data(filename));
}

void
SpriteManager::cleanup()
{
  std::erase_if(m_data, [](auto const& item) { return item.second.use_count() <= 1; });
}

} // namespace wstsprite

/* EOF */
