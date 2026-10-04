// Windstille Display Library
// Copyright (C) 2002-2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HEADER_WSTDISPLAY_FRAMEBUFFER_HPP
#define HEADER_WSTDISPLAY_FRAMEBUFFER_HPP

#include <cstdint>

#include <geom/size.hpp>

#include "fwd.hpp"
#include "texture.hpp"

namespace wstdisplay {

/** An offscreen render target backed by an RGBA texture */
class Framebuffer final
{
public:
  /** @param depth    also attach a depth buffer, needed for Mesh depth testing
      @param stencil  also attach a stencil buffer */
  static FramebufferPtr create(geom::isize const& size, TextureParams const& params = {},
                               bool depth = false, bool stencil = false);

private:
  Framebuffer(geom::isize const& size, TextureParams const& params, bool depth, bool stencil);
  void attach_renderbuffer(uint32_t& handle, uint32_t format, uint32_t attachment, geom::isize const& size);
  void destroy();

public:
  ~Framebuffer();

  geom::isize get_size() const { return m_texture->get_size(); }
  int get_width() const { return m_texture->get_width(); }
  int get_height() const { return m_texture->get_height(); }

  TexturePtr const& get_texture() const { return m_texture; }
  uint32_t get_handle() const { return m_handle; }

private:
  uint32_t m_handle;
  uint32_t m_depth_buffer;
  uint32_t m_stencil_buffer;
  TexturePtr m_texture;

public:
  Framebuffer(Framebuffer const&) = delete;
  Framebuffer& operator=(Framebuffer const&) = delete;
};

} // namespace wstdisplay

#endif

/* EOF */
