// SPDX-FileCopyrightText: 2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HEADER_WSTSPRITE_SPRITE_DATA_HPP
#define HEADER_WSTSPRITE_SPRITE_DATA_HPP

#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <glm/glm.hpp>
#include <geom/point.hpp>
#include <geom/rect.hpp>
#include <wstdisplay/fwd.hpp>

namespace prio {
class ReaderMapping;
} // namespace prio

namespace wstsprite {

/** Values of keys the library doesn't interpret, kept as strings so
    games can read their own extensions (hitboxes, sounds, ...) */
class Properties
{
public:
  bool has(std::string_view key) const;

  std::optional<std::string> get_string(std::string_view key) const;
  std::optional<float> get_float(std::string_view key) const;
  std::optional<int> get_int(std::string_view key) const;
  std::optional<bool> get_bool(std::string_view key) const;
  std::optional<glm::vec2> get_vec2(std::string_view key) const;

  /** All values of \a key, e.g. the four numbers of a hitbox */
  std::vector<std::string> const* get(std::string_view key) const;

  void set(std::string key, std::vector<std::string> values);

  std::map<std::string, std::vector<std::string>, std::less<>> const& get_all() const { return m_values; }

private:
  std::map<std::string, std::vector<std::string>, std::less<>> m_values;
};

/** Something a game wants to happen when an animation reaches a frame,
    e.g. (sound (frame 7) (name "splash") (volume 0.5)). The type is
    the element name, the library only stores and reports events, the
    game decides what they do. */
struct FrameEvent
{
  int frame = 0;
  std::string type = {};
  Properties properties = {};
};

struct Frame
{
  wstdisplay::SurfacePtr surface = {};

  /** How long the frame is shown, in seconds */
  float duration = 0.1f;
};

struct Action
{
  std::string name = {};
  std::vector<Frame> frames = {};

  /** The point of the frame, in frame pixels, that is placed at the
      draw position and that rotation and scaling are relative to */
  geom::fpoint origin = {};

  float scale = 1.0f;

  /** Draw the frames mirrored, used for "mirror-action" */
  bool hflip = false;

  /** Number of times the animation plays, -1 loops forever */
  int loops = -1;

  /** Frame the animation restarts at when looping */
  int loop_frame = 0;

  /** Collision box in frame pixels, SuperTux "hitbox" */
  std::optional<geom::frect> hitbox = {};

  std::vector<FrameEvent> events = {};

  /** Keys not interpreted by the library */
  Properties properties = {};

  float get_total_duration() const;
};

class SpriteData;
using SpriteDataPtr = std::shared_ptr<SpriteData const>;

/** The animations of a sprite, loaded from one of the .sprite formats
    of SuperTux, Windstille or Pingus, detected by the root element:

    (supertux-sprite (actions (action (name ..) (fps ..) (images ..) ...) ...))
    (sprite (actions (action (name ..) (speed ..) (images ..) ...) ...))
    (pingus-sprite (image ..) (array ..) (speed ..) (origin ..) ...)

    All formats accept an (events ...) list per action, see FrameEvent. */
class SpriteData final
{
public:
  /** Load \a filename, images are loaded through \a surfaces */
  static SpriteDataPtr from_file(std::filesystem::path const& filename, wstdisplay::SurfaceManager& surfaces);

  /** Load the sprite element \a name with \a mapping, images are
      relative to \a directory. Use this for sprites embedded in other
      files, e.g. Pingus animation sets. */
  static SpriteDataPtr from_mapping(std::string_view name, prio::ReaderMapping const& mapping,
                                    std::filesystem::path const& directory,
                                    wstdisplay::SurfaceManager& surfaces);

  /** A single image as a sprite with one action "default" */
  static SpriteDataPtr from_surface(wstdisplay::SurfacePtr surface);

public:
  SpriteData();

  std::vector<Action> const& get_actions() const { return m_actions; }

  /** Index of the action \a name, -1 if there is none */
  int find_action(std::string_view name) const;

  Action const& get_action(int index) const { return m_actions[static_cast<size_t>(index)]; }

  void add_action(Action action) { m_actions.push_back(std::move(action)); }

private:
  std::vector<Action> m_actions;
};

} // namespace wstsprite

#endif

/* EOF */
