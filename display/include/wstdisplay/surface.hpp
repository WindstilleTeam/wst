// Windstille Display Library
// Copyright (C) 2002-2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HEADER_WSTDISPLAY_SURFACE_HPP
#define HEADER_WSTDISPLAY_SURFACE_HPP

#include <memory>

#include <geom/rect.hpp>
#include <surf/software_surface.hpp>

#include "fwd.hpp"
#include "texture.hpp"

namespace wstdisplay {

/** A rectangular region of a Texture, the basic unit of drawing.
    Several surfaces can share a texture (atlas pages, sprite sheets),
    which is what allows drawing them in a single batch. */
class Surface final
{
public:
  /** @param uv    the region on the texture in [0,1] notation
      @param size  the size of the surface in pixels */
  static SurfacePtr create(TexturePtr texture, geom::frect const& uv, geom::fsize const& size);

  /** A surface covering the whole \a texture */
  static SurfacePtr create(TexturePtr texture);

  /** Upload \a image into a texture of its own */
  static SurfacePtr create(surf::SoftwareSurface const& image, TextureParams const& params = {});

  Surface(TexturePtr texture, geom::frect const& uv, geom::fsize const& size);

  /** A surface for the pixel rectangle \a rect of this surface,
      sharing the texture */
  SurfacePtr region(geom::frect const& rect) const;

  TexturePtr const& get_texture() const { return m_texture; }

  /** Texture coordinates of the surface rectangle */
  geom::frect const& get_uv() const { return m_uv; }

  geom::fsize get_size() const { return m_size; }
  float get_width() const { return m_size.width(); }
  float get_height() const { return m_size.height(); }

private:
  TexturePtr m_texture;
  geom::frect m_uv;
  geom::fsize m_size;
};

} // namespace wstdisplay

#endif

/* EOF */
