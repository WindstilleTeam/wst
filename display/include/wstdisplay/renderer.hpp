// SPDX-FileCopyrightText: 2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HEADER_WSTDISPLAY_RENDERER_HPP
#define HEADER_WSTDISPLAY_RENDERER_HPP

#include <memory>

#include <glm/glm.hpp>
#include <geom/rect.hpp>
#include <geom/size.hpp>
#include <surf/color.hpp>
#include <surf/software_surface.hpp>

#include "blend.hpp"
#include "caps.hpp"
#include "fwd.hpp"

namespace wstdisplay {

struct RenderStats
{
  int canvases = 0;
  int commands = 0;
  int batches = 0;
  int draw_calls = 0;
  int vertices = 0;
  int triangles = 0;
  int texture_binds = 0;
  int program_binds = 0;
  int target_switches = 0;
};

/** Executes Canvas contents on the current OpenGL context.

    The Renderer works on whatever context is current when it is
    created, so it can be used with OpenGLWindow as well as with a
    context owned by a toolkit (e.g. a Gtk::GLArea). It loads the GL
    function pointers, detects GL 3.3 core vs. GLES 2.0 and keeps a
    cache of the GL state it touches.

    A frame looks like:

      renderer.begin_frame(window_size);
      renderer.clear(color);
      renderer.render(canvas, view);
      renderer.end_frame();
      swap buffers */
class Renderer final
{
public:
  using ProcLoader = void* (*)(char const* name);

public:
  /** Initialize on the current context, \a loader resolves GL
      function names, e.g. SDL_GL_GetProcAddress */
  explicit Renderer(ProcLoader loader);
  ~Renderer();

  Caps const& get_caps() const;

  /** Start a frame on the framebuffer that is currently bound (the
      window, or a toolkit-provided one) with the given size. Resets
      the state cache and the stats. */
  void begin_frame(geom::isize const& size);
  void end_frame();

  /** Render to \a framebuffer, nullptr for the frame's target */
  void set_target(Framebuffer* framebuffer);
  Framebuffer* get_target() const;
  geom::isize get_target_size() const;

  void clear(surf::Color const& color);

  /** Render \a canvas, World space is mapped through \a view */
  void render(Canvas const& canvas, View const& view);

  /** Render \a canvas, \a world_matrix maps World space to target
      pixel coordinates */
  void render(Canvas const& canvas, glm::mat4 const& world_matrix);

  /** Render \a canvas with World space equal to Screen space */
  void render(Canvas const& canvas);

  /** Render \a canvas, \a screen_matrix maps Screen space to target
      pixel coordinates, for rendering into scaled down targets */
  void render(Canvas const& canvas, glm::mat4 const& world_matrix, glm::mat4 const& screen_matrix);

  /** Draw \a texture over the whole target, used for compositing */
  void blit(Texture const& texture, Blend blend, surf::Color const& color = {1.0f, 1.0f, 1.0f},
            bool flip_vertical = false);

  /** Read back the pixels of the current target as RGBA8, top row first */
  surf::SoftwareSurface read_pixels() const;
  surf::SoftwareSurface read_pixels(geom::irect const& rect) const;

  RenderStats const& get_stats() const;

  ShaderProgramPtr const& get_default_program() const;
  TexturePtr const& get_white_texture() const;

private:
  class Impl;
  std::unique_ptr<Impl> m_impl;

public:
  Renderer(Renderer const&) = delete;
  Renderer& operator=(Renderer const&) = delete;
};

} // namespace wstdisplay

#endif

/* EOF */
