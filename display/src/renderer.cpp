// Windstille Display Library
// Copyright (C) 2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "renderer.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <stdexcept>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <logmich/log.hpp>
#include <surf/palette.hpp>

#include "batch.hpp"
#include "canvas.hpp"
#include "framebuffer.hpp"
#include "gl.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "shader_program.hpp"
#include "static_batch.hpp"
#include "texture.hpp"
#include "view.hpp"

namespace wstdisplay {

namespace {

char const default_vert_source[] = R"(
attribute vec2 a_position;
attribute vec2 a_texcoord;
attribute vec4 a_color;

uniform mat4 u_mvp;

varying vec2 v_texcoord;
varying vec4 v_color;

void main()
{
  v_texcoord = a_texcoord;
  v_color = a_color;
  gl_Position = u_mvp * vec4(a_position, 0.0, 1.0);
}
)";

char const mesh_vert_source[] = R"(
attribute vec3 a_position;
attribute vec2 a_texcoord;
attribute vec4 a_color;

uniform mat4 u_mvp;

varying vec2 v_texcoord;
varying vec4 v_color;

void main()
{
  v_texcoord = a_texcoord;
  v_color = a_color;
  gl_Position = u_mvp * vec4(a_position, 1.0);
}
)";

char const default_frag_source[] = R"(
uniform sampler2D u_texture0;

varying vec2 v_texcoord;
varying vec4 v_color;

void main()
{
  gl_FragColor = texture2D(u_texture0, v_texcoord) * v_color;
}
)";

constexpr GLuint invalid_handle = 0xffffffffu;

} // namespace

class Renderer::Impl
{
public:
  Caps caps;

  ShaderProgramPtr default_program;
  ShaderProgramPtr mesh_program;
  TexturePtr white_texture;

  GLuint vao = 0;
  GLuint vertex_buffer = 0;
  GLuint index_buffer = 0;

  BatchList batches;
  Canvas blit_canvas;

  RenderStats stats;

  GLint frame_framebuffer = 0;
  geom::isize frame_size = geom::isize(1, 1);
  Framebuffer* target = nullptr;

  /*{ state cache, invalidated at the start of every render() */
  GLuint cur_program = invalid_handle;
  std::array<GLuint, ShaderProgram::max_textures> cur_textures = {};
  int cur_active_unit = -1;
  int cur_blend = -1;
  int cur_scissor_enabled = -1;
  geom::irect cur_scissor = {};
  Material const* cur_material = nullptr;
  bool cur_material_valid = false;
  glm::mat4 cur_mvp = glm::mat4(0.0f);
  bool cur_mvp_valid = false;
  int64_t cur_attrib_base = -1;
  /*} */

public:
  explicit Impl(ProcLoader loader) :
    caps(detail::load_gl(loader)),
    default_program(),
    mesh_program(),
    white_texture(),
    batches(),
    blit_canvas(),
    stats()
  {
    if (!caps.is_gles()) {
      // a core profile requires a VAO to be bound for drawing
      glGenVertexArrays(1, &vao);
    }
    glGenBuffers(1, &vertex_buffer);
    glGenBuffers(1, &index_buffer);

    default_program = ShaderProgram::from_source(default_vert_source, default_frag_source);
    mesh_program = ShaderProgram::from_source(mesh_vert_source, default_frag_source);
    white_texture = Texture::create(
      surf::SoftwareSurface::create(surf::PixelFormat::RGBA8, geom::isize(1, 1), surf::palette::white));

    WST_CHECK_GL("initializing renderer");
  }

  Impl(Impl const&) = delete;
  Impl& operator=(Impl const&) = delete;

  ~Impl()
  {
    glDeleteBuffers(1, &vertex_buffer);
    glDeleteBuffers(1, &index_buffer);
    if (vao) {
      glDeleteVertexArrays(1, &vao);
    }
  }

  geom::isize target_size() const
  {
    return target ? target->get_size() : frame_size;
  }

  /** Target pixel coordinates to clip space. Framebuffers are
      rendered upside down so that row 0 of their texture is the top
      row, like any uploaded image. */
  glm::mat4 projection() const
  {
    geom::isize const size = target_size();
    float const w = static_cast<float>(size.width());
    float const h = static_cast<float>(size.height());
    if (target) {
      return glm::ortho(0.0f, w, 0.0f, h, -1.0f, 1.0f);
    } else {
      return glm::ortho(0.0f, w, h, 0.0f, -1.0f, 1.0f);
    }
  }

  void invalidate_cache()
  {
    cur_program = invalid_handle;
    cur_textures.fill(invalid_handle);
    cur_active_unit = -1;
    cur_blend = -1;
    cur_scissor_enabled = -1;
    cur_material_valid = false;
    cur_mvp_valid = false;
    cur_attrib_base = -1;
  }

  void use_program(ShaderProgram const& program)
  {
    if (cur_program != program.get_handle()) {
      glUseProgram(program.get_handle());
      cur_program = program.get_handle();
      cur_mvp_valid = false;
      cur_material_valid = false;
      stats.program_binds += 1;
    }
  }

  void set_mvp(ShaderProgram const& program, glm::mat4 const& mvp)
  {
    if (!cur_mvp_valid || cur_mvp != mvp) {
      if (program.get_mvp_location() != -1) {
        glUniformMatrix4fv(program.get_mvp_location(), 1, GL_FALSE, glm::value_ptr(mvp));
      }
      cur_mvp = mvp;
      cur_mvp_valid = true;
    }
  }

  void bind_texture(int unit, GLuint handle)
  {
    if (cur_textures[static_cast<size_t>(unit)] != handle) {
      if (cur_active_unit != unit) {
        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(unit));
        cur_active_unit = unit;
      }
      glBindTexture(GL_TEXTURE_2D, handle);
      cur_textures[static_cast<size_t>(unit)] = handle;
      stats.texture_binds += 1;
    }
  }

  void set_blend(Blend blend)
  {
    int const b = static_cast<int>(blend);
    if (cur_blend == b) {
      return;
    }

    if (blend == Blend::Opaque) {
      glDisable(GL_BLEND);
    } else {
      if (cur_blend == -1 || cur_blend == static_cast<int>(Blend::Opaque)) {
        glEnable(GL_BLEND);
      }

      switch (blend)
      {
        case Blend::Alpha:
          glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
          break;

        case Blend::Premultiplied:
          glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
          break;

        case Blend::Add:
          glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE);
          break;

        case Blend::Multiply:
          glBlendFuncSeparate(GL_DST_COLOR, GL_ZERO, GL_ZERO, GL_ONE);
          break;

        case Blend::Opaque:
          break;
      }
    }
    cur_blend = b;
  }

  void set_scissor(std::optional<geom::irect> const& rect)
  {
    if (!rect) {
      if (cur_scissor_enabled != 0) {
        glDisable(GL_SCISSOR_TEST);
        cur_scissor_enabled = 0;
      }
    } else {
      if (cur_scissor_enabled != 1) {
        glEnable(GL_SCISSOR_TEST);
        cur_scissor_enabled = 1;
      }
      if (cur_scissor != *rect) {
        glScissor(rect->left(), rect->top(), rect->width(), rect->height());
        cur_scissor = *rect;
      }
    }
  }

  void apply_material(Material const* material)
  {
    if (cur_material_valid && cur_material == material) {
      return;
    }

    if (material) {
      for (int unit = 1; unit < ShaderProgram::max_textures; ++unit) {
        TexturePtr const& texture = material->get_texture(unit);
        if (texture) {
          bind_texture(unit, texture->get_handle());
        }
      }

      for (auto const& uniform : material->get_uniforms()) {
        float const* d = uniform.data.data();
        switch (uniform.type)
        {
          case Material::UniformType::Int: glUniform1i(uniform.location, static_cast<GLint>(d[0])); break;
          case Material::UniformType::Float: glUniform1f(uniform.location, d[0]); break;
          case Material::UniformType::Vec2: glUniform2fv(uniform.location, 1, d); break;
          case Material::UniformType::Vec3: glUniform3fv(uniform.location, 1, d); break;
          case Material::UniformType::Vec4: glUniform4fv(uniform.location, 1, d); break;
          case Material::UniformType::Mat4: glUniformMatrix4fv(uniform.location, 1, GL_FALSE, d); break;
        }
      }
    }

    cur_material = material;
    cur_material_valid = true;
  }

  void set_vertex_layout(int64_t base_vertex)
  {
    if (cur_attrib_base == base_vertex) {
      return;
    }

    auto const offset = [base_vertex](size_t member) {
      return reinterpret_cast<void const*>(static_cast<uintptr_t>(base_vertex) * sizeof(Vertex) + member);
    };

    glVertexAttribPointer(static_cast<GLuint>(Attrib::Position), 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), offset(offsetof(Vertex, x)));
    glVertexAttribPointer(static_cast<GLuint>(Attrib::Texcoord), 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), offset(offsetof(Vertex, u)));
    glVertexAttribPointer(static_cast<GLuint>(Attrib::Color), 4, GL_UNSIGNED_BYTE, GL_TRUE,
                          sizeof(Vertex), offset(offsetof(Vertex, color)));
    cur_attrib_base = base_vertex;
  }

  void enable_2d_attribs()
  {
    glEnableVertexAttribArray(static_cast<GLuint>(Attrib::Position));
    glEnableVertexAttribArray(static_cast<GLuint>(Attrib::Texcoord));
    glEnableVertexAttribArray(static_cast<GLuint>(Attrib::Color));
    glDisableVertexAttribArray(static_cast<GLuint>(Attrib::Normal));
  }

  /** Convert a clip rect to a scissor rect in GL window coordinates */
  std::optional<geom::irect> scissor_for(ClipRect const& clip, glm::mat4 const& mvp) const
  {
    geom::isize const size = target_size();
    geom::frect const& r = clip.rect;
    glm::vec4 const corners[] = {
      mvp * glm::vec4(r.left(), r.top(), 0.0f, 1.0f),
      mvp * glm::vec4(r.right(), r.top(), 0.0f, 1.0f),
      mvp * glm::vec4(r.right(), r.bottom(), 0.0f, 1.0f),
      mvp * glm::vec4(r.left(), r.bottom(), 0.0f, 1.0f)
    };

    float x0 = corners[0].x;
    float x1 = corners[0].x;
    float y0 = corners[0].y;
    float y1 = corners[0].y;
    for (auto const& c : corners) {
      x0 = std::min(x0, c.x);
      x1 = std::max(x1, c.x);
      y0 = std::min(y0, c.y);
      y1 = std::max(y1, c.y);
    }

    float const w = static_cast<float>(size.width());
    float const h = static_cast<float>(size.height());
    int const left = static_cast<int>(std::round((x0 + 1.0f) / 2.0f * w));
    int const right = static_cast<int>(std::round((x1 + 1.0f) / 2.0f * w));
    int const bottom = static_cast<int>(std::round((y0 + 1.0f) / 2.0f * h));
    int const top = static_cast<int>(std::round((y1 + 1.0f) / 2.0f * h));

    // irect(left, top, right, bottom) used as (x, y, x+w, y+h) in GL window coordinates
    return geom::irect(left, bottom, std::max(left, right), std::max(bottom, top));
  }

  void render(Canvas const& canvas, glm::mat4 const& world_matrix, glm::mat4 const& screen_matrix)
  {
    if (canvas.empty()) {
      return;
    }

    build_batches(canvas, batches);

    stats.canvases += 1;
    stats.commands += static_cast<int>(canvas.get_commands().size());

    invalidate_cache();

    if (vao) {
      glBindVertexArray(vao);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    if (!batches.vertices.empty()) {
      glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(batches.vertices.size() * sizeof(Vertex)),
                   batches.vertices.data(), GL_STREAM_DRAW);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(batches.indices.size() * sizeof(uint16_t)),
                   batches.indices.data(), GL_STREAM_DRAW);
    }
    enable_2d_attribs();

    glm::mat4 const proj = projection();
    std::array<glm::mat4, 3> const space_mvp = {
      proj * world_matrix,  // World
      proj * screen_matrix, // Screen
      glm::mat4(1.0f)      // Clip
    };

    auto const& clips = canvas.get_clip_rects();

    for (Batch const& batch : batches.batches)
    {
      glm::mat4 const& mvp = space_mvp[static_cast<size_t>(batch.space)];

      set_scissor(batch.clip == 0 ? std::nullopt :
                  scissor_for(clips[batch.clip], space_mvp[static_cast<size_t>(clips[batch.clip].space)]));

      if (batch.type == CommandType::StaticBatch) {
        draw_static_batch(*static_cast<StaticBatch const*>(batch.object),
                          mvp * canvas.get_matrices()[batch.matrix]);
        continue;
      } else if (batch.type == CommandType::Mesh) {
        draw_mesh(*static_cast<Mesh const*>(batch.object),
                  mvp, canvas.get_matrices()[batch.matrix]);
        // meshes use their own buffers, restore the canvas ones
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        enable_2d_attribs();
        cur_attrib_base = -1;
        continue;
      }

      ShaderProgram const& program = batch.material ? *batch.material->get_program() : *default_program;
      use_program(program);
      set_mvp(program, mvp);
      apply_material(batch.material);
      bind_texture(0, batch.texture ? batch.texture->get_handle() : white_texture->get_handle());
      set_blend(batch.blend);
      set_vertex_layout(batch.base_vertex);

      glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(batch.index_count), GL_UNSIGNED_SHORT,
                     reinterpret_cast<void const*>(static_cast<uintptr_t>(batch.first_index) * sizeof(uint16_t)));

      stats.batches += 1;
      stats.draw_calls += 1;
      stats.triangles += static_cast<int>(batch.index_count / 3);
    }

    stats.vertices += static_cast<int>(batches.vertices.size());

    set_scissor(std::nullopt);
    WST_CHECK_GL("rendering canvas");
  }

  void draw_static_batch(StaticBatch const& sbatch, glm::mat4 const& mvp)
  {
    sbatch.upload();

    glBindBuffer(GL_ARRAY_BUFFER, sbatch.get_vertex_buffer());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sbatch.get_index_buffer());
    cur_attrib_base = -1;

    for (Batch const& batch : sbatch.get_batches().batches) {
      if (batch.type != CommandType::Geometry) {
        continue;
      }

      ShaderProgram const& program = batch.material ? *batch.material->get_program() : *default_program;
      use_program(program);
      set_mvp(program, mvp);
      apply_material(batch.material);
      bind_texture(0, batch.texture ? batch.texture->get_handle() : white_texture->get_handle());
      set_blend(batch.blend);
      set_vertex_layout(batch.base_vertex);

      glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(batch.index_count), GL_UNSIGNED_SHORT,
                     reinterpret_cast<void const*>(static_cast<uintptr_t>(batch.first_index) * sizeof(uint16_t)));

      stats.batches += 1;
      stats.draw_calls += 1;
      stats.triangles += static_cast<int>(batch.index_count / 3);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    cur_attrib_base = -1;
  }

  void draw_mesh(Mesh const& mesh, glm::mat4 const& mvp, glm::mat4 const& model)
  {
    mesh.upload();

    ShaderProgram const& program = mesh.get_material() ? *mesh.get_material()->get_program() : *mesh_program;
    use_program(program);
    set_mvp(program, mvp * model);
    apply_material(mesh.get_material().get());
    bind_texture(0, mesh.get_texture() ? mesh.get_texture()->get_handle() : white_texture->get_handle());
    set_blend(mesh.get_blend());

    int const model_location = program.get_uniform_location("u_model");
    if (model_location != -1) {
      glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model));
    }

    if (mesh.get_depth_test()) {
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_LEQUAL);
    }

    mesh.draw();

    if (mesh.get_depth_test()) {
      glDisable(GL_DEPTH_TEST);
    }

    stats.draw_calls += 1;
    stats.triangles += mesh.get_triangle_count();
    cur_attrib_base = -1;
  }

  void bind_target()
  {
    geom::isize const size = target_size();
    glBindFramebuffer(GL_FRAMEBUFFER, target ? target->get_handle() : static_cast<GLuint>(frame_framebuffer));
    glViewport(0, 0, size.width(), size.height());
    stats.target_switches += 1;
  }
};

Renderer::Renderer(ProcLoader loader) :
  m_impl(std::make_unique<Impl>(loader))
{
}

Renderer::~Renderer()
{
}

Caps const&
Renderer::get_caps() const
{
  return m_impl->caps;
}

void
Renderer::begin_frame(geom::isize const& size)
{
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_impl->frame_framebuffer);
  m_impl->frame_size = size;
  m_impl->target = nullptr;
  m_impl->stats = RenderStats();
  m_impl->invalidate_cache();

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glDisable(GL_SCISSOR_TEST);

  m_impl->bind_target();
}

void
Renderer::end_frame()
{
  m_impl->target = nullptr;
  m_impl->bind_target();

  glUseProgram(0);
  glDisable(GL_SCISSOR_TEST);
  glDisable(GL_BLEND);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
  if (m_impl->vao) {
    glBindVertexArray(0);
  }
  m_impl->invalidate_cache();
  WST_CHECK_GL("end of frame");
}

void
Renderer::set_target(Framebuffer* framebuffer)
{
  if (m_impl->target != framebuffer) {
    m_impl->target = framebuffer;
    m_impl->bind_target();
  }
}

Framebuffer*
Renderer::get_target() const
{
  return m_impl->target;
}

geom::isize
Renderer::get_target_size() const
{
  return m_impl->target_size();
}

void
Renderer::clear(surf::Color const& color)
{
  glDisable(GL_SCISSOR_TEST);
  m_impl->cur_scissor_enabled = 0;
  glClearColor(color.r, color.g, color.b, color.a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void
Renderer::render(Canvas const& canvas, View const& view)
{
  m_impl->render(canvas, view.get_matrix(), glm::mat4(1.0f));
}

void
Renderer::render(Canvas const& canvas, glm::mat4 const& world_matrix)
{
  m_impl->render(canvas, world_matrix, glm::mat4(1.0f));
}

void
Renderer::render(Canvas const& canvas, glm::mat4 const& world_matrix, glm::mat4 const& screen_matrix)
{
  m_impl->render(canvas, world_matrix, screen_matrix);
}

void
Renderer::render(Canvas const& canvas)
{
  m_impl->render(canvas, glm::mat4(1.0f), glm::mat4(1.0f));
}

void
Renderer::blit(Texture const& texture, Blend blend, surf::Color const& color, bool flip_vertical)
{
  float const v0 = flip_vertical ? 1.0f : 0.0f;
  float const v1 = flip_vertical ? 0.0f : 1.0f;
  PackedColor const pc = pack_color(color);

  // clip space is y-up, texture row 0 is the top
  Vertex const vertices[] = {
    {-1.0f,  1.0f, 0.0f, v0, pc},
    { 1.0f,  1.0f, 1.0f, v0, pc},
    { 1.0f, -1.0f, 1.0f, v1, pc},
    {-1.0f, -1.0f, 0.0f, v1, pc},
  };

  Canvas& canvas = m_impl->blit_canvas;
  canvas.clear();
  canvas.set_space(Space::Clip);
  canvas.draw_quads(&texture, vertices, blend);
  m_impl->render(canvas, glm::mat4(1.0f), glm::mat4(1.0f));
  canvas.clear();
}

surf::SoftwareSurface
Renderer::read_pixels() const
{
  return read_pixels(geom::irect(m_impl->target_size()));
}

surf::SoftwareSurface
Renderer::read_pixels(geom::irect const& rect) const
{
  geom::isize const size = m_impl->target_size();
  std::vector<uint8_t> pixels(static_cast<size_t>(rect.width()) * static_cast<size_t>(rect.height()) * 4);

  // the default framebuffer has row 0 at the bottom, framebuffers at the top
  int const y = m_impl->target ? rect.top() : size.height() - rect.bottom();

  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(rect.left(), y, rect.width(), rect.height(), GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
  WST_CHECK_GL("reading pixels");

  surf::SoftwareSurface surface = surf::SoftwareSurface::create(surf::PixelFormat::RGBA8, rect.size());
  size_t const row_size = static_cast<size_t>(rect.width()) * 4;
  for (int row = 0; row < rect.height(); ++row) {
    int const src_row = m_impl->target ? row : rect.height() - 1 - row;
    std::memcpy(surface.get_row_data(row), pixels.data() + row_size * static_cast<size_t>(src_row), row_size);
  }
  return surface;
}

RenderStats const&
Renderer::get_stats() const
{
  return m_impl->stats;
}

ShaderProgramPtr const&
Renderer::get_default_program() const
{
  return m_impl->default_program;
}

TexturePtr const&
Renderer::get_white_texture() const
{
  return m_impl->white_texture;
}

} // namespace wstdisplay

/* EOF */
