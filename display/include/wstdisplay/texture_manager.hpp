// Windstille Display Library
// Copyright (C) 2002-2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HEADER_WSTDISPLAY_TEXTURE_MANAGER_HPP
#define HEADER_WSTDISPLAY_TEXTURE_MANAGER_HPP

#include <filesystem>
#include <map>
#include <utility>

#include "fwd.hpp"
#include "texture.hpp"

namespace wstdisplay {

/** Loads images into textures of their own and caches them, for
    textures that can't live on an atlas, e.g. repeating patterns or
    textures used by custom shaders. */
class TextureManager final
{
public:
  TextureManager();
  ~TextureManager();

  /** Load \a filename, on failure the fallback texture is returned
      if one is set, otherwise the error is rethrown */
  TexturePtr get(std::filesystem::path const& filename, TextureParams const& params = {});

  void set_fallback(std::filesystem::path const& filename);

  /** Forget textures that are only referenced by the cache */
  void cleanup();

private:
  using Key = std::pair<std::filesystem::path, int>;
  std::map<Key, TexturePtr> m_textures;
  TexturePtr m_fallback;

public:
  TextureManager(TextureManager const&) = delete;
  TextureManager& operator=(TextureManager const&) = delete;
};

} // namespace wstdisplay

#endif

/* EOF */
