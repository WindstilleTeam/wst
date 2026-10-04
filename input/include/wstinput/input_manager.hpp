// SPDX-FileCopyrightText: 2005-2020 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HEADER_WINDSTILLE_INPUT_INPUT_MANAGER_HPP
#define HEADER_WINDSTILLE_INPUT_INPUT_MANAGER_HPP

#include <SDL.h>
#include <filesystem>
#include <memory>

#include <prio/fwd.hpp>

#include "controller.hpp"
#include "controller_description.hpp"
#include "input_bindings.hpp"

namespace wstinput {

class InputManagerSDLImpl;

class InputManagerSDL
{
public:
  InputManagerSDL(ControllerDescription const& controller_description);
  ~InputManagerSDL();

  void load(std::filesystem::path const& filename);

  void update(float delta);

  void clear();

  std::string keyid_to_string(SDL_Scancode id) const;
  SDL_Scancode string_to_keyid(const std::string& str) const;

  ControllerDescription const& get_controller_description() const { return m_controller_description; }
  Controller const& get_controller() const { return m_controller; }

  InputBindings& bindings() { return m_bindings; }

  void on_event(const SDL_Event& event);

  /** Ensure that the joystick device \a device is open */
  void ensure_open_joystick(int device);

  void start_text_input();
  void stop_text_input();
  bool is_text_input_active() const;

private:
  ControllerDescription m_controller_description;
  Controller m_controller;
  InputBindings m_bindings;
  std::vector<SDL_Joystick*> m_joysticks;
  std::map<std::string, SDL_Scancode> m_keyidmapping;

private:
  InputManagerSDL (const InputManagerSDL&);
  InputManagerSDL& operator= (const InputManagerSDL&);
};

} // namespace wstinput

#endif

/* EOF */
