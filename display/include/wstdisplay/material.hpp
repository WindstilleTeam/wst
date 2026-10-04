// SPDX-FileCopyrightText: 2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HEADER_WSTDISPLAY_MATERIAL_HPP
#define HEADER_WSTDISPLAY_MATERIAL_HPP

#include <array>
#include <string_view>
#include <vector>

#include <glm/glm.hpp>

#include "fwd.hpp"
#include "shader_program.hpp"

namespace wstdisplay {

/** A custom shader together with its extra textures and uniform
    values. Drawing with a material replaces the default program,
    texture unit 0 is still the texture of the drawn surface.

    Draws sharing the same Material object can be batched together,
    so share materials instead of creating one per draw. */
class Material final
{
public:
  enum class UniformType : uint8_t { Int, Float, Vec2, Vec3, Vec4, Mat4 };

  struct Uniform
  {
    int location;
    UniformType type;
    std::array<float, 16> data;
  };

public:
  explicit Material(ShaderProgramPtr program);

  ShaderProgramPtr const& get_program() const { return m_program; }

  /** Set the texture for \a unit in [1, ShaderProgram::max_textures) */
  void set_texture(int unit, TexturePtr texture);
  TexturePtr const& get_texture(int unit) const { return m_textures[static_cast<size_t>(unit)]; }

  /** Set the uniform \a name, unknown names are ignored */
  void set_uniform(std::string_view name, int value);
  void set_uniform(std::string_view name, float value);
  void set_uniform(std::string_view name, glm::vec2 const& value);
  void set_uniform(std::string_view name, glm::vec3 const& value);
  void set_uniform(std::string_view name, glm::vec4 const& value);
  void set_uniform(std::string_view name, glm::mat4 const& value);

  std::vector<Uniform> const& get_uniforms() const { return m_uniforms; }

private:
  void set(std::string_view name, UniformType type, float const* data, size_t count);

private:
  ShaderProgramPtr m_program;
  std::array<TexturePtr, ShaderProgram::max_textures> m_textures;
  std::vector<Uniform> m_uniforms;
};

} // namespace wstdisplay

#endif

/* EOF */
