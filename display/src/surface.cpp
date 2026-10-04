// Windstille Display Library
// Copyright (C) 2002-2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "surface.hpp"

namespace wstdisplay {

SurfacePtr
Surface::create(TexturePtr texture, geom::frect const& uv, geom::fsize const& size)
{
  return std::make_shared<Surface>(std::move(texture), uv, size);
}

SurfacePtr
Surface::create(TexturePtr texture)
{
  geom::fsize const size(texture->get_size());
  return std::make_shared<Surface>(std::move(texture), geom::frect(0.0f, 0.0f, 1.0f, 1.0f), size);
}

SurfacePtr
Surface::create(surf::SoftwareSurface const& image, TextureParams const& params)
{
  return create(Texture::create(image, params));
}

Surface::Surface(TexturePtr texture, geom::frect const& uv, geom::fsize const& size) :
  m_texture(std::move(texture)),
  m_uv(uv),
  m_size(size)
{
}

SurfacePtr
Surface::region(geom::frect const& rect) const
{
  float const sx = m_uv.width() / m_size.width();
  float const sy = m_uv.height() / m_size.height();
  geom::frect const uv(m_uv.left() + rect.left() * sx,
                       m_uv.top() + rect.top() * sy,
                       m_uv.left() + rect.right() * sx,
                       m_uv.top() + rect.bottom() * sy);
  return create(m_texture, uv, rect.size());
}

} // namespace wstdisplay

/* EOF */
