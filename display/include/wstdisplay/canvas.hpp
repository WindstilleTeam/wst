// Windstille Display Library
// Copyright (C) 2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HEADER_WSTDISPLAY_CANVAS_HPP
#define HEADER_WSTDISPLAY_CANVAS_HPP

#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

#include <glm/glm.hpp>
#include <geom/fwd.hpp>
#include <geom/rect.hpp>
#include <surf/color.hpp>

#include "blend.hpp"
#include "draw_params.hpp"
#include "fwd.hpp"
#include "vertex.hpp"

namespace wstdisplay {

/** The coordinate system vertices are given in */
enum class Space : uint8_t
{
  /** World coordinates, transformed by the View the canvas is
      rendered with */
  World,

  /** Pixel coordinates of the render target, (0,0) is top left,
      unaffected by the View, for HUDs and overlays */
  Screen,

  /** Normalized device coordinates, [-1,1] covers the target */
  Clip
};

enum class TextAlign : uint8_t { Left, Center, Right };

enum class CommandType : uint8_t
{
  /** Vertices and indices stored in the canvas */
  Geometry,

  /** A StaticBatch, drawn with a transform from get_matrices() */
  StaticBatch,

  /** A Mesh, drawn with a transform from get_matrices() */
  Mesh
};

/** A clip rect together with the space it was specified in */
struct ClipRect
{
  geom::frect rect;
  Space space;
};

/** Records drawing commands for later rendering with a Renderer.

    All draws are recorded as plain data: vertices go into one shared
    vertex array and each draw adds a small Command referencing them.
    Before rendering the commands are stably sorted by z, so draws
    with equal z keep their submission order, and neighboring
    commands with the same texture, material, blend mode and clip
    rect are merged into a single GPU draw call. Putting surfaces on
    shared atlas pages is what makes batching effective.

    Textures and materials are referenced by raw pointer and must stay
    alive until the canvas has been rendered or cleared.

    The canvas keeps a 2D transform stack that is applied to vertices
    when they are recorded, and a clip rect stack. */
class Canvas final
{
public:
  struct Command
  {
    float z;
    uint32_t first_vertex;
    uint32_t vertex_count;
    uint32_t first_index;
    uint32_t index_count;

    /** nullptr draws untextured (with a white texture) */
    Texture const* texture;

    /** nullptr uses the default program */
    Material const* material;

    /** The StaticBatch or Mesh for those command types */
    void const* object;

    /** Index into get_matrices() for StaticBatch and Mesh commands */
    uint32_t matrix;

    /** Index into get_clip_rects(), 0 is no clipping */
    uint16_t clip;

    Blend blend;
    Space space;
    CommandType type;
  };

  /** Maximum number of vertices a single command can hold, so that
      16-bit indices are enough (GLES 2.0) */
  static constexpr uint32_t max_command_vertices = 0xffff;

public:
  Canvas();

  /** Remove all commands, the transform and clip stack are reset */
  void clear();

  bool empty() const { return m_commands.empty(); }

  /*{ Transform */
  void push_transform();
  void pop_transform();
  void reset_transform();
  void translate(float x, float y);
  void scale(float x, float y);

  /** Rotate by \a degrees around the current origin */
  void rotate(float degrees);

  void set_transform(glm::mat3 const& transform);
  void mult_transform(glm::mat3 const& transform);
  glm::mat3 const& get_transform() const { return m_transform_stack.back(); }
  /*} */

  /*{ Clipping */
  /** Clip all following draws to \a rect, given in current canvas
      coordinates and intersected with the current clip rect. A
      rotated rect clips to its bounding box. */
  void push_clip(geom::frect const& rect);
  void pop_clip();
  /*} */

  /** Coordinate space for the following draws, World by default */
  void set_space(Space space) { m_space = space; }
  Space get_space() const { return m_space; }

  /*{ Surfaces */
  void draw(Surface const& surface, geom::fpoint const& pos, float z = 0.0f);
  void draw(Surface const& surface, DrawParams const& params, float z = 0.0f);

  /** Stretch the surface to \a dstrect */
  void draw(Surface const& surface, geom::frect const& dstrect,
            surf::Color const& color = {1.0f, 1.0f, 1.0f}, Blend blend = Blend::Alpha, float z = 0.0f);

  /** Draw the pixel rectangle \a srcrect of the surface into \a dstrect */
  void draw(Surface const& surface, geom::frect const& srcrect, geom::frect const& dstrect,
            surf::Color const& color = {1.0f, 1.0f, 1.0f}, Blend blend = Blend::Alpha, float z = 0.0f);

  /** Fill \a rect by repeating \a texture, \a offset shifts the
      pattern. Needs a power-of-two texture on GLES 2.0 without
      GL_OES_texture_npot. */
  void fill_pattern(Texture const& texture, geom::frect const& rect, geom::foffset const& offset = {},
                    surf::Color const& color = {1.0f, 1.0f, 1.0f}, float z = 0.0f);
  /*} */

  /*{ Shapes, lines are drawn as quads of the given width */
  void fill_screen(surf::Color const& color, float z = 0.0f);

  void fill_rect(geom::frect const& rect, surf::Color const& color, float z = 0.0f);
  void draw_rect(geom::frect const& rect, surf::Color const& color, float z = 0.0f, float width = 1.0f);

  void fill_rounded_rect(geom::frect const& rect, float radius, surf::Color const& color, float z = 0.0f);
  void draw_rounded_rect(geom::frect const& rect, float radius, surf::Color const& color,
                         float z = 0.0f, float width = 1.0f);

  void fill_quad(geom::fquad const& quad, surf::Color const& color, float z = 0.0f);
  void draw_quad(geom::fquad const& quad, surf::Color const& color, float z = 0.0f, float width = 1.0f);

  void fill_triangle(geom::fpoint const& p1, geom::fpoint const& p2, geom::fpoint const& p3,
                     surf::Color const& color, float z = 0.0f);

  void draw_line(geom::fpoint const& p1, geom::fpoint const& p2, surf::Color const& color,
                 float z = 0.0f, float width = 1.0f);

  /** Draw a connected line through \a points, closed if \a loop */
  void draw_polyline(std::span<geom::fpoint const> points, surf::Color const& color,
                     bool loop = false, float z = 0.0f, float width = 1.0f);

  void fill_circle(geom::fpoint const& center, float radius, surf::Color const& color,
                   float z = 0.0f, int segments = 32);
  void draw_circle(geom::fpoint const& center, float radius, surf::Color const& color,
                   float z = 0.0f, float width = 1.0f, int segments = 32);

  /** Angles in degrees, clockwise starting at the positive x axis */
  void fill_arc(geom::fpoint const& center, float radius, float start, float end,
                surf::Color const& color, float z = 0.0f, int segments = 32);
  void draw_arc(geom::fpoint const& center, float radius, float start, float end,
                surf::Color const& color, float z = 0.0f, float width = 1.0f, int segments = 32);

  /** Fill with a color per corner */
  void fill_gradient(geom::frect const& rect,
                     surf::Color const& top_left, surf::Color const& top_right,
                     surf::Color const& bottom_right, surf::Color const& bottom_left,
                     float z = 0.0f);
  void fill_vertical_gradient(geom::frect const& rect, surf::Color const& top,
                              surf::Color const& bottom, float z = 0.0f);
  void fill_horizontal_gradient(geom::frect const& rect, surf::Color const& left,
                                surf::Color const& right, float z = 0.0f);
  /*} */

  /*{ Text, \a pos is the left end of the baseline of the first line */
  void draw_text(Font const& font, geom::fpoint const& pos, std::string_view text,
                 surf::Color const& color = {1.0f, 1.0f, 1.0f}, TextAlign align = TextAlign::Left,
                 float z = 0.0f);
  /*} */

  /*{ Raw geometry, vertices are in current canvas coordinates */
  void draw_triangles(Texture const* texture, std::span<Vertex const> vertices,
                      std::span<uint16_t const> indices, Blend blend = Blend::Alpha,
                      float z = 0.0f, Material const* material = nullptr);

  /** Four vertices per quad, in order top-left, top-right,
      bottom-right, bottom-left */
  void draw_quads(Texture const* texture, std::span<Vertex const> vertices,
                  Blend blend = Blend::Alpha, float z = 0.0f, Material const* material = nullptr);

  /** Draw the prebuilt geometry of \a batch with the current
      transform, the batch must stay alive until rendering */
  void draw(StaticBatch const& batch, float z = 0.0f);

  /** Draw \a mesh with \a transform applied on top of the current
      canvas transform, the mesh must stay alive until rendering */
  void draw(Mesh const& mesh, glm::mat4 const& transform, float z = 0.0f);
  /*} */

  /*{ Recorded data */
  std::vector<Command> const& get_commands() const { return m_commands; }
  std::vector<Vertex> const& get_vertices() const { return m_vertices; }
  std::vector<uint16_t> const& get_indices() const { return m_indices; }

  /** Clip rects, entry 0 is unused */
  std::vector<ClipRect> const& get_clip_rects() const { return m_clip_rects; }

  /** Transforms of StaticBatch and Mesh commands */
  std::vector<glm::mat4> const& get_matrices() const { return m_matrices; }
  /*} */

private:
  /** Append a command, reusing the previous one if all state matches.
      Returns the vertices to fill, indices are given relative to the
      first returned vertex. */
  Vertex* add(Texture const* texture, Material const* material, Blend blend, float z,
              uint32_t vertex_count, std::span<uint16_t const> indices);
  Vertex* add_quads(Texture const* texture, Material const* material, Blend blend, float z,
                    uint32_t quad_count);

  void add_object(CommandType type, void const* object, glm::mat4 const& matrix, float z,
                  Material const* material);

  glm::vec2 apply(float x, float y) const;
  void transform_vertices(Vertex* vertices, size_t count) const;

  void add_polygon_fill(std::span<glm::vec2 const> points, PackedColor color, float z);
  void add_line_strip(std::span<glm::vec2 const> points, bool loop, PackedColor color, float z, float width);

private:
  std::vector<Command> m_commands;
  std::vector<Vertex> m_vertices;
  std::vector<uint16_t> m_indices;

  std::vector<glm::mat3> m_transform_stack;
  bool m_identity;

  std::vector<ClipRect> m_clip_rects;
  std::vector<glm::mat4> m_matrices;
  std::vector<uint16_t> m_clip_stack;

  Space m_space;

  /** scratch buffer for building outlines */
  std::vector<glm::vec2> m_points;

public:
  Canvas(Canvas const&) = delete;
  Canvas& operator=(Canvas const&) = delete;
};

} // namespace wstdisplay

#endif

/* EOF */
