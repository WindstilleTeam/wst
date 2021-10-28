// Windstille Display Library
// Copyright (C) 2002-2020 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
// License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#ifndef HEADER_WINDSTILLE_DISPLAY_OPENGL_WINDOW_HPP
#define HEADER_WINDSTILLE_DISPLAY_OPENGL_WINDOW_HPP

#include <filesystem>
#include <memory>

#include <SDL.h>

#include <geom/size.hpp>
#include <surf/fwd.hpp>

namespace wstdisplay {

class GraphicsContext;
class OpenGLWindowImpl;

class OpenGLWindow final
{
public:
  OpenGLWindow();
  OpenGLWindow(const std::string& title,
               const geom::isize& size, const geom::isize& aspect,
               bool fullscreen = false, int anti_aliasing = 0);
  ~OpenGLWindow();

  void set_title(std::string const& title);
  void set_icon(std::filesystem::path const& filename);
  void set_size(geom::isize const& size);
  void set_aspect(geom::isize const& aspect);
  void set_fullscreen(bool fullscreen);
  void set_resizable(bool resizable);
  void set_anti_aliasing(int anti_aliasing);
  void set_gamma(float r, float g, float b);

  void show();

  geom::isize get_size() const;

  GraphicsContext& get_gc() const;

  void swap_buffers();

  surf::SoftwareSurface screenshot() const;

private:
  std::string m_title;
  SDL_Window* m_window;
  SDL_GLContext m_gl_context;
  geom::isize m_size;
  geom::isize m_aspect;
  bool m_fullscreen;
  bool m_resizable;
  int m_anti_aliasing;

  std::unique_ptr<GraphicsContext> m_gc;

private:
  OpenGLWindow(const OpenGLWindow&);
  OpenGLWindow& operator=(const OpenGLWindow&);
};

} // namespace wstdisplay

#endif

/* EOF */
