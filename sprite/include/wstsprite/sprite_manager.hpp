// wst - Windstille game framework
// Copyright (C) 2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HEADER_WSTSPRITE_SPRITE_MANAGER_HPP
#define HEADER_WSTSPRITE_SPRITE_MANAGER_HPP

#include <filesystem>
#include <map>

#include <wstdisplay/fwd.hpp>

#include "sprite.hpp"
#include "sprite_data.hpp"

namespace wstsprite {

/** Loads and caches SpriteData by filename. Plain images are accepted
    as single frame sprites. */
class SpriteManager final
{
public:
  explicit SpriteManager(wstdisplay::SurfaceManager& surfaces);

  SpriteDataPtr get_data(std::filesystem::path const& filename);
  Sprite create(std::filesystem::path const& filename);

  /** Forget data only referenced by the cache */
  void cleanup();

private:
  wstdisplay::SurfaceManager& m_surfaces;
  std::map<std::filesystem::path, SpriteDataPtr> m_data;

public:
  SpriteManager(SpriteManager const&) = delete;
  SpriteManager& operator=(SpriteManager const&) = delete;
};

} // namespace wstsprite

#endif

/* EOF */
