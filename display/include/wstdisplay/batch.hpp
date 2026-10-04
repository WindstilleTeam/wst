// Windstille Display Library
// Copyright (C) 2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HEADER_WSTDISPLAY_BATCH_HPP
#define HEADER_WSTDISPLAY_BATCH_HPP

#include <cstdint>
#include <vector>

#include "canvas.hpp"
#include "vertex.hpp"

namespace wstdisplay {

/** A run of indices that can be drawn with a single draw call */
struct Batch
{
  Texture const* texture;
  Material const* material;

  /** StaticBatch or Mesh for those types, see Canvas::Command */
  void const* object;
  uint32_t matrix;

  /** Vertex offset the 16-bit indices of the batch are relative to */
  uint32_t base_vertex;

  uint32_t first_index;
  uint32_t index_count;

  uint16_t clip;
  Blend blend;
  Space space;
  CommandType type;
};

/** Vertices and indices of a whole canvas in draw order, ready for
    upload, split into batches. Pure CPU data, no GL involved. */
struct BatchList
{
  std::vector<Vertex> vertices = {};
  std::vector<uint16_t> indices = {};
  std::vector<Batch> batches = {};

  /** Scratch space for sorting */
  std::vector<uint32_t> order = {};

  void clear();
};

/** Sort the commands of \a canvas by z (stable) and merge neighboring
    commands with equal state into batches. Indices are rebased so
    that no batch references more than 65536 vertices. */
void build_batches(Canvas const& canvas, BatchList& out);

} // namespace wstdisplay

#endif

/* EOF */
