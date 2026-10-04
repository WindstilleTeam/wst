// Windstille Display Library
// Copyright (C) 2005-2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "font/ttf_font.hpp"

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <logmich/log.hpp>
#include <surf/palette.hpp>
#include <surf/software_surface.hpp>

#include "blitter.hpp"
#include "caps.hpp"
#include "font/font_effect.hpp"
#include "font/ttf_font_manager.hpp"
#include "rect_packer.hpp"
#include "texture.hpp"

namespace wstdisplay {

namespace {

struct GlyphPage
{
  TexturePtr texture;
  RectPacker packer;
};

} // namespace

class TTFFontImpl
{
public:
  std::vector<char> buffer;
  FT_Face face = nullptr;
  int size = 0;
  std::unique_ptr<FontEffect> effect;
  float line_height = 0.0f;

  std::vector<GlyphPage> pages;
  geom::isize page_size;
  mutable std::unordered_map<char32_t, Glyph> glyphs;
  mutable std::unordered_map<char32_t, bool> missing;

  TTFFontImpl() :
    buffer(),
    effect(),
    pages(),
    page_size(512, 512),
    glyphs(),
    missing()
  {}

  TTFFontImpl(TTFFontImpl const&) = delete;
  TTFFontImpl& operator=(TTFFontImpl const&) = delete;

  ~TTFFontImpl()
  {
    if (face) {
      FT_Done_Face(face);
    }
  }

  Glyph const* load_glyph(char32_t codepoint)
  {
    FT_UInt const index = FT_Get_Char_Index(face, codepoint);
    if (index == 0 && codepoint != 0) {
      return nullptr;
    }

    if (FT_Load_Glyph(face, index, FT_LOAD_RENDER)) {
      log_warn("TTFFont: couldn't render U+{:04X}", static_cast<uint32_t>(codepoint));
      return nullptr;
    }

    FT_GlyphSlot slot = face->glyph;
    float const advance = static_cast<float>(slot->advance.x >> 6);

    int const bitmap_w = static_cast<int>(slot->bitmap.width);
    int const bitmap_h = static_cast<int>(slot->bitmap.rows);
    if (bitmap_w == 0 || bitmap_h == 0) {
      // whitespace
      return &(glyphs[codepoint] = Glyph{nullptr, {}, {}, advance});
    }


    int const glyph_w = effect->get_glyph_width(bitmap_w);
    int const glyph_h = effect->get_glyph_height(bitmap_h);

    // one pixel of padding around each glyph, filled by generate_border()
    surf::SoftwareSurface image = surf::SoftwareSurface::create(
      surf::PixelFormat::RGBA8, geom::isize(glyph_w + 2, glyph_h + 2), surf::palette::transparent);
    effect->blit(image, slot->bitmap, 1, 1);
    generate_border(image, 1, 1, glyph_w, glyph_h);

    auto [page, rect] = allocate(image.get_size());
    page->texture->put(image, rect.topleft());

    float const tw = static_cast<float>(page->texture->get_width());
    float const th = static_cast<float>(page->texture->get_height());
    geom::frect const uv(static_cast<float>(rect.left() + 1) / tw,
                         static_cast<float>(rect.top() + 1) / th,
                         static_cast<float>(rect.left() + 1 + glyph_w) / tw,
                         static_cast<float>(rect.top() + 1 + glyph_h) / th);

    float const x = static_cast<float>(effect->get_x_offset(slot->bitmap_left));
    float const y = static_cast<float>(effect->get_y_offset(-slot->bitmap_top));
    geom::frect const glyph_rect(x, y, x + static_cast<float>(glyph_w), y + static_cast<float>(glyph_h));

    return &(glyphs[codepoint] = Glyph{page->texture.get(), uv, glyph_rect, advance});
  }

  std::pair<GlyphPage*, geom::irect> allocate(geom::isize const& glyph_size)
  {
    for (auto& page : pages) {
      if (auto rect = page.packer.allocate(glyph_size)) {
        return {&page, *rect};
      }
    }

    pages.push_back(GlyphPage{Texture::create(page_size), RectPacker(page_size)});
    auto rect = pages.back().packer.allocate(glyph_size);
    if (!rect) {
      throw std::runtime_error("TTFFont: glyph too large for atlas page");
    }
    return {&pages.back(), *rect};
  }
};

TTFFont::TTFFont(TTFFontManager& mgr, std::filesystem::path const& filename, int size, FontEffect const& effect) :
  impl(std::make_unique<TTFFontImpl>())
{
  if (size <= 0) {
    throw std::invalid_argument("TTFFont: size must be positive");
  }

  impl->size = size;
  impl->effect = effect.clone();

  std::ifstream fin(filename, std::ios::binary);
  if (!fin) {
    throw std::runtime_error("TTFFont: couldn't open " + filename.string());
  }
  impl->buffer.assign(std::istreambuf_iterator<char>(fin), std::istreambuf_iterator<char>());

  if (FT_New_Memory_Face(mgr.get_handle(),
                         reinterpret_cast<FT_Byte const*>(impl->buffer.data()),
                         static_cast<FT_Long>(impl->buffer.size()),
                         0, &impl->face))
  {
    throw std::runtime_error("TTFFont: couldn't load font: " + filename.string());
  }

  FT_Set_Pixel_Sizes(impl->face, static_cast<FT_UInt>(size), static_cast<FT_UInt>(size));
  FT_Select_Charmap(impl->face, FT_ENCODING_UNICODE);

  impl->line_height = static_cast<float>(impl->face->size->metrics.height >> 6) +
    static_cast<float>(impl->effect->get_height(size) - size);

  // a page that fits a reasonable set of glyphs, larger fonts get larger pages
  int const page = std::clamp(size * 16, 256, 2048);
  impl->page_size = geom::isize(page, page);

  for (char32_t cp = 32; cp < 256; ++cp) {
    get_glyph(cp);
  }
}

TTFFont::~TTFFont()
{
}

Glyph const*
TTFFont::get_glyph(char32_t codepoint) const
{
  auto it = impl->glyphs.find(codepoint);
  if (it != impl->glyphs.end()) {
    return &it->second;
  }

  if (impl->missing.contains(codepoint)) {
    return nullptr;
  }

  Glyph const* glyph = impl->load_glyph(codepoint);
  if (!glyph) {
    impl->missing[codepoint] = true;
  }
  return glyph;
}

float
TTFFont::get_height() const
{
  return static_cast<float>(impl->effect->get_height(impl->size));
}

float
TTFFont::get_line_height() const
{
  return impl->line_height;
}

int
TTFFont::get_pixel_size() const
{
  return impl->size;
}

} // namespace wstdisplay

/* EOF */
