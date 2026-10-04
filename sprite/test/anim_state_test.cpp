// wst - Windstille game framework
// Copyright (C) 2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include <wstsprite/anim_state.hpp>

using namespace wstsprite;

namespace {

// frames without surfaces, advance() never looks at them
Action make_action(std::string name, int frames, float duration, int loops = -1)
{
  Action action;
  action.name = std::move(name);
  action.loops = loops;
  for (int i = 0; i < frames; ++i) {
    action.frames.push_back(Frame{nullptr, duration});
  }
  return action;
}

} // namespace

TEST(AnimStateTest, loops_forever)
{
  SpriteData data;
  data.add_action(make_action("walk", 4, 0.1f));

  AnimState state;
  advance(state, data, 0.25f);
  EXPECT_EQ(state.frame, 2);
  advance(state, data, 0.2f);
  EXPECT_EQ(state.frame, 0);
  EXPECT_EQ(state.loops_done, 1);
  EXPECT_FALSE(state.finished);
}

TEST(AnimStateTest, plays_once)
{
  SpriteData data;
  data.add_action(make_action("die", 3, 0.1f, 1));

  AnimState state;
  advance(state, data, 1.0f);
  EXPECT_EQ(state.frame, 2);
  EXPECT_TRUE(state.finished);

  advance(state, data, 1.0f);
  EXPECT_EQ(state.frame, 2);
}

TEST(AnimStateTest, loop_frame)
{
  SpriteData data;
  Action action = make_action("idle", 4, 0.1f);
  action.loop_frame = 2;
  data.add_action(std::move(action));

  AnimState state;
  advance(state, data, 0.45f);
  // 0 1 2 3 -> 2
  EXPECT_EQ(state.frame, 2);
}

TEST(AnimStateTest, events_fire_when_frame_is_entered)
{
  SpriteData data;
  Action action = make_action("kill", 8, 0.1f, 1);
  action.events.push_back(FrameEvent{7, "sound", {}});
  action.events.push_back(FrameEvent{0, "start", {}});
  data.add_action(std::move(action));

  AnimState state;
  std::vector<FrameEvent const*> events;

  fire_current_events(state, data, events);
  ASSERT_EQ(events.size(), 1u);
  EXPECT_EQ(events[0]->type, "start");
  events.clear();

  advance(state, data, 0.65f, &events);
  EXPECT_TRUE(events.empty());

  advance(state, data, 0.1f, &events);
  ASSERT_EQ(events.size(), 1u);
  EXPECT_EQ(events[0]->type, "sound");
}

TEST(AnimStateTest, deterministic_with_fixed_steps)
{
  SpriteData data;
  data.add_action(make_action("a", 5, 0.033f));

  AnimState a;
  AnimState b;
  for (int i = 0; i < 1000; ++i) {
    advance(a, data, 1.0f / 60.0f);
  }
  for (int i = 0; i < 1000; ++i) {
    advance(b, data, 1.0f / 60.0f);
  }
  EXPECT_EQ(a.frame, b.frame);
  EXPECT_EQ(a.loops_done, b.loops_done);
  EXPECT_FLOAT_EQ(a.frame_time, b.frame_time);
}

TEST(AnimStateTest, speed_and_pause)
{
  SpriteData data;
  data.add_action(make_action("a", 10, 0.1f));

  AnimState state;
  state.speed = 2.0f;
  advance(state, data, 0.1f);
  EXPECT_EQ(state.frame, 2);

  state.speed = 0.0f;
  advance(state, data, 10.0f);
  EXPECT_EQ(state.frame, 2);
}

TEST(AnimStateTest, set_action)
{
  SpriteData data;
  data.add_action(make_action("walk-left", 4, 0.1f));
  data.add_action(make_action("walk-right", 4, 0.1f));

  AnimState state;
  advance(state, data, 0.25f);
  EXPECT_TRUE(set_action_continued(state, data, "walk-right"));
  EXPECT_EQ(state.action, 1);
  EXPECT_EQ(state.frame, 2);

  EXPECT_TRUE(set_action(state, data, "walk-left"));
  EXPECT_EQ(state.action, 0);
  EXPECT_EQ(state.frame, 0);

  EXPECT_FALSE(set_action(state, data, "jump"));
  EXPECT_EQ(state.action, 0);
}

TEST(PropertiesTest, typed_access)
{
  Properties props;
  props.set("volume", {"0.5"});
  props.set("offset", {"40", "90"});
  props.set("name", {"splash"});
  props.set("loop", {"false"});

  EXPECT_FLOAT_EQ(*props.get_float("volume"), 0.5f);
  EXPECT_EQ(*props.get_vec2("offset"), glm::vec2(40.0f, 90.0f));
  EXPECT_EQ(*props.get_string("name"), "splash");
  EXPECT_EQ(*props.get_bool("loop"), false);
  EXPECT_FALSE(props.get_float("name").has_value());
  EXPECT_FALSE(props.has("missing"));
}

/* EOF */
