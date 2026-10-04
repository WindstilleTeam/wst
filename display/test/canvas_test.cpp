// SPDX-FileCopyrightText: 2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include <wstdisplay/canvas.hpp>
#include <wstdisplay/surface.hpp>

using namespace wstdisplay;

TEST(CanvasTest, transform_is_applied_on_record)
{
  Canvas canvas;
  canvas.push_transform();
  canvas.translate(10, 20);
  canvas.scale(2, 2);
  canvas.fill_rect(geom::frect(0, 0, 1, 1), surf::Color(1, 1, 1));
  canvas.pop_transform();
  canvas.fill_rect(geom::frect(0, 0, 1, 1), surf::Color(1, 1, 1));

  auto const& v = canvas.get_vertices();
  ASSERT_EQ(v.size(), 8u);
  EXPECT_FLOAT_EQ(v[0].x, 10.0f);
  EXPECT_FLOAT_EQ(v[0].y, 20.0f);
  EXPECT_FLOAT_EQ(v[2].x, 12.0f);
  EXPECT_FLOAT_EQ(v[2].y, 22.0f);
  EXPECT_FLOAT_EQ(v[4].x, 0.0f);
  EXPECT_FLOAT_EQ(v[6].x, 1.0f);
}

TEST(CanvasTest, rotate_is_clockwise_on_screen)
{
  Canvas canvas;
  canvas.rotate(90.0f);
  canvas.fill_rect(geom::frect(0, 0, 1, 1), surf::Color(1, 1, 1));

  // (1, 0) rotated by 90 degrees with y pointing down ends up at (0, 1)
  auto const& v = canvas.get_vertices();
  EXPECT_NEAR(v[1].x, 0.0f, 1e-6f);
  EXPECT_NEAR(v[1].y, 1.0f, 1e-6f);
}

TEST(CanvasTest, pop_transform_underflow_throws)
{
  Canvas canvas;
  EXPECT_THROW(canvas.pop_transform(), std::logic_error);
  EXPECT_THROW(canvas.pop_clip(), std::logic_error);
}

TEST(CanvasTest, clip_rects_nest)
{
  Canvas canvas;
  canvas.push_clip(geom::frect(0, 0, 100, 100));
  canvas.push_clip(geom::frect(50, 50, 200, 200));
  canvas.fill_rect(geom::frect(0, 0, 1, 1), surf::Color(1, 1, 1));
  canvas.pop_clip();
  canvas.pop_clip();

  auto const& clips = canvas.get_clip_rects();
  ASSERT_EQ(clips.size(), 3u);
  EXPECT_EQ(clips[2].rect, geom::frect(50, 50, 100, 100));
  EXPECT_EQ(canvas.get_commands().back().clip, 2);
}

TEST(CanvasTest, clip_rects_follow_transform)
{
  Canvas canvas;
  canvas.translate(10, 10);
  canvas.push_clip(geom::frect(0, 0, 5, 5));
  EXPECT_EQ(canvas.get_clip_rects()[1].rect, geom::frect(10, 10, 15, 15));
}

TEST(CanvasTest, shapes_merge_into_one_command)
{
  Canvas canvas;
  canvas.fill_rect(geom::frect(0, 0, 1, 1), surf::Color(1, 0, 0));
  canvas.fill_circle(geom::fpoint(5, 5), 2.0f, surf::Color(0, 1, 0));
  canvas.draw_line(geom::fpoint(0, 0), geom::fpoint(5, 5), surf::Color(0, 0, 1));
  canvas.draw_rounded_rect(geom::frect(0, 0, 10, 10), 3.0f, surf::Color(1, 1, 1));
  EXPECT_EQ(canvas.get_commands().size(), 1u);
}

TEST(CanvasTest, different_z_doesnt_merge)
{
  Canvas canvas;
  canvas.fill_rect(geom::frect(0, 0, 1, 1), surf::Color(1, 0, 0), 0.0f);
  canvas.fill_rect(geom::frect(0, 0, 1, 1), surf::Color(1, 0, 0), 1.0f);
  EXPECT_EQ(canvas.get_commands().size(), 2u);
}

TEST(CanvasTest, null_surface_draws_nothing)
{
  Canvas canvas;
  canvas.draw(Surface(), geom::fpoint(0, 0));
  canvas.draw(Surface(), DrawParams());
  EXPECT_TRUE(canvas.empty());
}

TEST(CanvasTest, surface_region)
{
  Surface const surface(TextureId{0, 1}, geom::frect(0.5f, 0.0f, 1.0f, 0.5f), geom::fsize(32, 32));
  Surface const part = surface.region(geom::frect(16, 0, 32, 16));
  EXPECT_EQ(part.get_texture(), surface.get_texture());
  EXPECT_EQ(part.get_size(), geom::fsize(16, 16));
  EXPECT_EQ(part.get_uv(), geom::frect(0.75f, 0.0f, 1.0f, 0.25f));
}

TEST(CanvasTest, fill_screen_uses_clip_space)
{
  Canvas canvas;
  canvas.translate(100, 100);
  canvas.fill_screen(surf::Color(1, 1, 1));
  ASSERT_EQ(canvas.get_commands().size(), 1u);
  EXPECT_EQ(canvas.get_commands()[0].space, Space::Clip);
  EXPECT_FLOAT_EQ(canvas.get_vertices()[0].x, -1.0f);
  EXPECT_EQ(canvas.get_space(), Space::World);
}

TEST(CanvasTest, clear_resets_state)
{
  Canvas canvas;
  canvas.translate(5, 5);
  canvas.push_clip(geom::frect(0, 0, 1, 1));
  canvas.fill_rect(geom::frect(0, 0, 1, 1), surf::Color(1, 1, 1));
  canvas.clear();

  EXPECT_TRUE(canvas.empty());
  EXPECT_EQ(canvas.get_transform(), glm::mat3(1.0f));
  EXPECT_EQ(canvas.get_clip_rects().size(), 1u);
}

TEST(CanvasTest, packed_color_byte_order)
{
  PackedColor const c = pack_color(surf::Color::from_rgba8888(1, 2, 3, 4));
  auto const* bytes = reinterpret_cast<uint8_t const*>(&c);
  EXPECT_EQ(bytes[0], 1);
  EXPECT_EQ(bytes[1], 2);
  EXPECT_EQ(bytes[2], 3);
  EXPECT_EQ(bytes[3], 4);
  EXPECT_EQ(unpack_color(c), surf::Color::from_rgba8888(1, 2, 3, 4));
}

/* EOF */
