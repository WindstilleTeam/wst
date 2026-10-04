// Windstille Display Library
// Copyright (C) 2002-2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "surface_manager.hpp"

#include <algorithm>

#include <logmich/log.hpp>

#include "gl.hpp"
#include "surface.hpp"

namespace wstdisplay {

namespace {

SurfaceManager::Options clamp_options(SurfaceManager::Options options)
{
  int const max_size = detail::current_caps().max_texture_size;
  options.page_size = geom::isize(std::min(options.page_size.width(), max_size),
                                  std::min(options.page_size.height(), max_size));
  options.max_atlas_image_size = std::min(options.max_atlas_image_size,
                                          std::min(options.page_size.width(), options.page_size.height()) - 2);
  return options;
}

} // namespace

SurfaceManager::SurfaceManager() :
  SurfaceManager(Options())
{
}

SurfaceManager::SurfaceManager(Options const& options) :
  m_options(clamp_options(options)),
  m_packer(m_options.page_size, m_options.params),
  m_surfaces(),
  m_fallback()
{
}

SurfaceManager::~SurfaceManager()
{
}

SurfacePtr
SurfaceManager::get(std::filesystem::path const& filename)
{
  auto it = m_surfaces.find(filename);
  if (it != m_surfaces.end()) {
    return it->second;
  }

  SurfacePtr surface;
  try {
    surface = create(surf::SoftwareSurface::from_file(filename));
  } catch (std::exception const& err) {
    if (!m_fallback) {
      throw;
    }
    log_error("SurfaceManager: {}", err.what());
    surface = m_fallback;
  }
  m_surfaces[filename] = surface;
  return surface;
}

SurfacePtr
SurfaceManager::create(surf::SoftwareSurface const& image)
{
  if (image.get_width() <= m_options.max_atlas_image_size &&
      image.get_height() <= m_options.max_atlas_image_size) {
    return m_packer.upload(image);
  } else {
    return Surface::create(image, m_options.params);
  }
}

std::vector<SurfacePtr>
SurfaceManager::load_grid(std::filesystem::path const& filename, geom::isize const& size)
{
  surf::SoftwareSurface const image = surf::SoftwareSurface::from_file(filename);

  std::vector<SurfacePtr> surfaces;
  for (int y = 0; y + size.height() <= image.get_height(); y += size.height()) {
    for (int x = 0; x + size.width() <= image.get_width(); x += size.width()) {
      surfaces.push_back(create(image.get_view(geom::irect(geom::ipoint(x, y), size))));
    }
  }
  return surfaces;
}

void
SurfaceManager::cleanup()
{
  std::erase_if(m_surfaces, [](auto const& item) { return item.second.use_count() <= 1; });
}

} // namespace wstdisplay

/* EOF */
