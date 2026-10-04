// Windstille Display Library
// Copyright (C) 2002-2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HEADER_WSTDISPLAY_FWD_HPP
#define HEADER_WSTDISPLAY_FWD_HPP

#include <memory>

namespace wstdisplay {

class BitmapFont;
class BorderFontEffect;
class Canvas;
class Compositor;
class Font;
class FontEffect;
class Framebuffer;
class Material;
class Mesh;
class NoFontEffect;
class OpenGLWindow;
class Renderer;
class ShaderProgram;
class StaticBatch;
class Surface;
class SurfaceManager;
class TTFFont;
class TTFFontManager;
class TextArea;
class Texture;
class TextureManager;
class TexturePacker;
class View;

struct Caps;
struct DrawParams;
struct RenderStats;
struct Vertex;

using FramebufferPtr = std::shared_ptr<Framebuffer>;
using MaterialPtr = std::shared_ptr<Material>;
using MeshPtr = std::shared_ptr<Mesh>;
using ShaderProgramPtr = std::shared_ptr<ShaderProgram>;
using SurfacePtr = std::shared_ptr<Surface>;
using TexturePtr = std::shared_ptr<Texture>;

} // namespace wstdisplay

#endif

/* EOF */
