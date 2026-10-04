// Windstille Display Library
// Copyright (C) 2002-2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "compositor.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include <glm/gtc/matrix_transform.hpp>

#include "framebuffer.hpp"
#include "renderer.hpp"
#include "view.hpp"

namespace wstdisplay {

std::vector<PassDesc>
Compositor::lighting_passes(surf::Color const& ambient)
{
  return {
    PassDesc{.name = "color"},
    PassDesc{.name = "light", .offscreen = true, .resolution = 0.25f,
             .clear_color = ambient, .composite_blend = Blend::Multiply},
    PassDesc{.name = "highlight"},
    PassDesc{.name = "overlay", .screen_space = true}
  };
}

Compositor::Compositor(std::vector<PassDesc> passes) :
  m_passes()
{
  for (auto& desc : passes) {
    m_passes.push_back(Pass{std::move(desc), std::make_unique<Canvas>(), {}});
  }
}

Compositor::~Compositor()
{
}

size_t
Compositor::get_pass_index(std::string_view name) const
{
  auto it = std::find_if(m_passes.begin(), m_passes.end(),
                         [name](Pass const& pass) { return pass.desc.name == name; });
  if (it == m_passes.end()) {
    throw std::out_of_range("Compositor: unknown pass: " + std::string(name));
  }
  return static_cast<size_t>(it - m_passes.begin());
}

PassDesc&
Compositor::get_pass(size_t index)
{
  return m_passes.at(index).desc;
}

PassDesc&
Compositor::get_pass(std::string_view name)
{
  return get_pass(get_pass_index(name));
}

Canvas&
Compositor::get_canvas(size_t index)
{
  return *m_passes.at(index).canvas;
}

Canvas&
Compositor::get_canvas(std::string_view name)
{
  return get_canvas(get_pass_index(name));
}

void
Compositor::set_enabled(std::string_view name, bool enabled)
{
  get_pass(name).enabled = enabled;
}

void
Compositor::render(Renderer& renderer, View const& view)
{
  Framebuffer* const main_target = renderer.get_target();
  geom::isize const screen_size = renderer.get_target_size();

  for (auto& pass : m_passes)
  {
    if (!pass.desc.enabled) {
      pass.canvas->clear();
      continue;
    }

    glm::mat4 const world_matrix = pass.desc.screen_space ? glm::mat4(1.0f) : view.get_matrix();

    if (!pass.desc.offscreen)
    {
      renderer.render(*pass.canvas, world_matrix, glm::mat4(1.0f));
    }
    else
    {
      float const res = pass.desc.resolution;
      geom::isize const size(std::max(1, static_cast<int>(std::ceil(static_cast<float>(screen_size.width()) * res))),
                             std::max(1, static_cast<int>(std::ceil(static_cast<float>(screen_size.height()) * res))));

      if (!pass.framebuffer || pass.framebuffer->get_size() != size) {
        pass.framebuffer = Framebuffer::create(size);
      }

      // map screen pixels onto the smaller buffer
      glm::mat4 const scale = glm::scale(glm::mat4(1.0f),
                                         glm::vec3(static_cast<float>(size.width()) / static_cast<float>(screen_size.width()),
                                                   static_cast<float>(size.height()) / static_cast<float>(screen_size.height()),
                                                   1.0f));

      renderer.set_target(pass.framebuffer.get());
      renderer.clear(pass.desc.clear_color);
      renderer.render(*pass.canvas, scale * world_matrix, scale);
      renderer.set_target(main_target);

      renderer.blit(*pass.framebuffer->get_texture(), pass.desc.composite_blend);
    }

    pass.canvas->clear();
  }
}

void
Compositor::clear()
{
  for (auto& pass : m_passes) {
    pass.canvas->clear();
  }
}

} // namespace wstdisplay

/* EOF */
