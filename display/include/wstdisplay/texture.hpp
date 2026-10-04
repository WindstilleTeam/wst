// Windstille Display Library
// Copyright (C) 2002-2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HEADER_WSTDISPLAY_TEXTURE_HPP
#define HEADER_WSTDISPLAY_TEXTURE_HPP

#include <cstdint>
#include <memory>

#include <geom/rect.hpp>
#include <geom/size.hpp>
#include <surf/software_surface.hpp>

#include "fwd.hpp"

namespace wstdisplay {

enum class TextureFilter : uint8_t { Nearest, Linear };
enum class TextureWrap : uint8_t { Clamp, Repeat };

struct TextureParams
{
  TextureFilter filter = TextureFilter::Linear;

  /** Horizontal and vertical wrapping, e.g. repeat only horizontally
      for parallax layers */
  TextureWrap wrap_x = TextureWrap::Clamp;
  TextureWrap wrap_y = TextureWrap::Clamp;
};

/** An RGBA texture on the GPU. Requires a current OpenGL context
    with the library initialized, see Renderer. */
class Texture final
{
public:
  /** Upload \a image, any pixel format is accepted and converted
      to RGB8 or RGBA8 if needed */
  static TexturePtr create(surf::SoftwareSurface const& image, TextureParams const& params = {});

  /** Create a transparent RGBA8 texture */
  static TexturePtr create(geom::isize const& size, TextureParams const& params = {});

private:
  Texture(geom::isize const& size, TextureParams const& params);

public:
  ~Texture();

  geom::isize get_size() const { return m_size; }
  int get_width() const { return m_size.width(); }
  int get_height() const { return m_size.height(); }

  TextureParams const& get_params() const { return m_params; }
  void set_filter(TextureFilter filter);
  void set_wrap(TextureWrap wrap_x, TextureWrap wrap_y);

  /** Upload \a image to the given position */
  void put(surf::SoftwareSurface const& image, geom::ipoint const& pos);

  /** Upload the section \a srcrect of \a image to the given position */
  void put(surf::SoftwareSurface const& image, geom::irect const& srcrect, geom::ipoint const& pos);

  /** Download the texture content as RGBA8, slow */
  surf::SoftwareSurface get_software_surface() const;

  /** The OpenGL texture name */
  uint32_t get_handle() const { return m_handle; }

private:
  void apply_params();

private:
  uint32_t m_handle;
  geom::isize m_size;
  TextureParams m_params;

public:
  Texture(Texture const&) = delete;
  Texture& operator=(Texture const&) = delete;
};

} // namespace wstdisplay

#endif

/* EOF */
