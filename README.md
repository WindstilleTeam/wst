wst - Windstille game framework
===============================

A modular C++20 game framework shared by Windstille, Pingus and
SuperTux Origins. One repository, one CMake project, a library per
module:

| Module      | Library      | Depends on                      | Contents                                              |
|-------------|--------------|---------------------------------|-------------------------------------------------------|
| `display/`  | `wstdisplay` | SDL2, freetype, surf, geom, babyxml | Rendering: GL 3.3 / GLES 2.0, Canvas, Compositor, fonts, window |
| `sprite/`   | `wstsprite`  | wstdisplay, prio                | Sprite data (SuperTux/Windstille/Pingus formats), animation, frame events |
| `input/`    | `wstinput`   | SDL2, prio                      | Controller abstraction and input bindings              |
| `gui/`      | `wstgui`     | wstdisplay, wstinput            | Widgets, menus, ScreenManager                          |
| `demos/`    | -            | all of the above                | Demo applications and GL regression tests              |

wstsound stays a separate library, nothing here depends on it, sound
is triggered by the games, e.g. from sprite frame events.

The history of the former wstdisplay, wstinput and wstgui
repositories is preserved in `display/`, `input/` and `gui/`, their
tags are prefixed with the module name (`wstdisplay-v0.3.0`, ...).


Building
--------

    nix build                      # library, demos, CPU tests, -Werror
    nix run .#demo-sprites         # or any other demo

or with CMake directly:

    cmake -B build -DBUILD_TESTS=ON -DWARNINGS=ON
    cmake --build build
    cd build && ctest              # everything
    ctest -LE gl                   # CPU only tests
    ctest -L gl                    # demos rendered with GL and GLES, needs a GPU or EGL

Options: `WST_DISPLAY`, `WST_INPUT`, `WST_GUI`, `WST_SPRITE`,
`WST_DEMOS` (all `ON`), `BUILD_TESTS`, `WARNINGS`, `WERROR`.

Consumers use the installed packages:

    find_package(wstdisplay REQUIRED)
    find_package(wstsprite REQUIRED)
    target_link_libraries(game PRIVATE wstdisplay::wstdisplay wstsprite::wstsprite)


Demos
-----

All demos take `--gl` / `--gles` to force the API, `--size WxH`,
`--no-vsync`, `--benchmark`, `--frames N --screenshot FILE` for
automated runs and `--hidden`. With `SDL_VIDEODRIVER=offscreen` they
render without a display (also on llvmpipe with
`LIBGL_ALWAYS_SOFTWARE=1`). F1 toggles the overlay with render
statistics, F12 takes a screenshot.

| Demo                       | Shows                                                         |
|----------------------------|---------------------------------------------------------------|
| `wst-demo-shapes`          | every Canvas primitive, transforms, clipping, z order, text   |
| `wst-demo-sprites`         | sprite benchmark, atlas vs. separate textures (`--count N`)   |
| `wst-demo-text`            | TTF fonts, border effect, BitmapFont, TextArea markup         |
| `wst-demo-lighting`        | Compositor lighting passes, camera zoom and rotation          |
| `wst-demo-tilemap`         | StaticBatch chunks vs. per frame recording, parallax layers   |
| `wst-demo-destructible`    | Pingus style terrain with partial texture updates             |
| `wst-demo-mesh`            | Mesh with depth test, custom shaders, render to framebuffer   |
| `wst-demo-gui`             | wstgui widgets and menus                                      |
| `wst-demo-sprite-viewer`   | the three .sprite formats, hitboxes and frame events, `--path DIR`, `--check` |

Measured on a Radeon RX 6700 at 1280x720, same workload:

| Sprites | old wstdisplay                    | wstdisplay now                    |
|---------|-----------------------------------|-----------------------------------|
| 5000    | 17.7 ms/frame, 5000 draw calls    | -                                 |
| 20000   | 70 ms/frame, 20000 draw calls     | 0.8 ms/frame, 2 draw calls        |
| 100000  | -                                 | 3.6 ms/frame, 7 draw calls        |


Rendering architecture
----------------------

    game
     │  records draws (CPU only, any time during the frame)
     ▼
    Canvas ── commands + one shared vertex array, transform/clip stack
     │  build_batches(): stable sort by z, merge neighbors with equal state
     ▼
    Renderer ── GL state cache, one streamed VBO/IBO per canvas,
     │          GL 3.3 core or GLES 2.0, framebuffers, materials
     ▼
    OpenGL context (OpenGLWindow, or a toolkit like Gtk::GLArea)

    Compositor ── several Canvases as passes (color, light, ...),
                  offscreen passes are combined with a blend mode
    SurfaceManager / TexturePacker ── images on shared atlas pages
    StaticBatch ── prebuilt geometry kept on the GPU
    Mesh, Material ── 3D meshes and custom shaders

A frame:

    renderer.begin_frame(window.get_drawable_size());
    renderer.clear(color);

    canvas.clear();
    canvas.draw(*surface, DrawParams().set_pos(pos).set_angle(45.0f), z);
    canvas.fill_rect(rect, color);
    canvas.draw_text(font, pos, "Hello");
    renderer.render(canvas, view);

    renderer.end_frame();
    window.swap_buffers();

### Batching rules

Draws are merged into one GL draw call when, after sorting by z, they
are next to each other and share texture, material, blend mode, clip
rect and coordinate space. In practice:

- Load images through `SurfaceManager`, small images end up on shared
  atlas pages, so different sprites share a texture.
- Untextured shapes and lines share the white texture and batch with
  each other, lines and outlines are drawn as quads.
- Equal z keeps submission order. Use z to group things, e.g. one z
  per layer, interleaving textures within a z breaks batches.
- Textures, materials, static batches and meshes are referenced by
  pointer from the canvas and must live until it is rendered.

### Coordinate spaces

`Space::World` is mapped through the `View` given to
`Renderer::render()`, `Space::Screen` is target pixels (HUDs),
`Space::Clip` is normalized device coordinates (`fill_screen()`). The
canvas transform stack is applied when a draw is recorded.

### Shaders

Shaders are written in GLSL ES 1.00 without a `#version` line and
translated to GLSL 3.30 by a preamble: use `attribute`, `varying`,
`texture2D()` and `gl_FragColor`. Built-in names are `a_position`,
`a_texcoord`, `a_color`, `a_normal`, `u_mvp`, `u_model` (meshes) and
`u_texture0` to `u_texture3`. A `Material` holds a program, extra
textures and uniform values, see `demos/demo_mesh.cpp`.

### GLES 2.0

GLES 2.0 is the baseline, GL 3.3 core is used where available and
falls back to GLES. Things to keep in mind on GLES 2.0 hardware:

- Repeating textures need power-of-two sizes unless
  `GL_OES_texture_npot` is present, the library falls back to clamp.
- 16-bit indices, the batch builder splits automatically.
- Framebuffers get 16 bit depth and 8 bit stencil.

`Caps` (`renderer.get_caps()`) reports what the context offers.


Sprites
-------

`wstsprite` loads all three .sprite formats into one model:
`SpriteData` holds actions, an action has frames with durations, an
origin (the point placed at the draw position), scale, mirroring,
loop count, an optional hitbox and frame events. Unknown keys are
kept in `Properties` for the game.

    SpriteDataPtr data = SpriteData::from_file("tux.sprite", surfaces);

    // in an ECS component: the shared data and a small POD state
    struct SpriteComponent { SpriteDataPtr data; AnimState anim; };

    // animation system, fixed time step
    events.clear();
    advance(c.anim, *c.data, dt, &events);
    for (FrameEvent const* e : events) {
      if (e->type == "sound") {
        sound.play(*e->properties.get_string("name"), e->properties.get_float("volume").value_or(1.0f));
      }
    }

    // render system
    draw(canvas, *c.data, c.anim, DrawParams().set_pos(pos), z);

Frame events are written per action, `at-step` is accepted for Pingus:

    (events
      (sound (frame 7) (name "splash") (volume 0.5))
      (particles (frame 7) (kind "pingu") (count 3)))

`wst-demo-sprite-viewer --check --path DIR` loads a whole data
directory and reports what failed.


Porting from the old wstdisplay
-------------------------------

| Old                                        | New                                                     |
|--------------------------------------------|---------------------------------------------------------|
| `GraphicsContext`, `OpenGLWindow::get_gc()`| `Renderer` (`window.get_renderer()`) plus a `Canvas`    |
| `DrawingContext`                           | `Canvas`                                                |
| `SceneContext` (color/light/highlight/control) | `Compositor(Compositor::lighting_passes(ambient))` |
| `GraphicContextState`                      | `View` (rotation in degrees everywhere now)             |
| `Surface::draw(gc, pos)`                   | `canvas.draw(surface, pos)`                             |
| `SurfaceDrawingParameters`                 | `DrawParams` (angle in degrees, `anchor` instead of rotating around the center) |
| blend `GLenum` pairs                       | `Blend::Alpha`, `Add`, `Multiply`, `Premultiplied`, `Opaque` |
| `VertexArrayDrawable`                      | `canvas.draw_triangles()` / `draw_quads()`              |
| `SceneGraph`, persistent `Drawable`s       | redraw every frame, or `StaticBatch` for static geometry|
| `ShaderDrawable`, `ShockwaveDrawable`      | `Material` on a draw, `Framebuffer` for post processing |
| `TTFFont::draw(gc, ...)`                   | `canvas.draw_text(font, ...)`, fonts are UTF-8 now      |
| `TextArea::draw(gc)`                       | `text_area.draw(canvas)`                                |
| `Texture::create(GL_TEXTURE_2D, size)`     | `Texture::create(size, TextureParams{...})`             |
| `Framebuffer::create_with_texture(...)`    | `Framebuffer::create(size, params, depth, stencil)`     |

Notes per game:

- **SuperTux Origins**: `Canvas`/`Painter`/`DrawingRequest` map almost
  one to one onto `wstdisplay::Canvas`, `int layer` becomes `z`. The
  lightmap becomes a Compositor pass. `Rectf` can move to
  `geom::frect`, geom has `from_center()`, `anchor_point()` and
  `with_left()` and friends for the setters. `Sprite::draw()`
  advancing the animation goes away, `AnimState` + `advance()` in an
  animation system replaces it.
- **Pingus**: the `Framebuffer`/`FramebufferSurface` interface and the
  SDL software renderer are replaced by `Renderer`, the ground map
  maps onto chunk textures with `Texture::put()` as in
  `wst-demo-destructible`. Integer coordinates can stay in Pingus or
  become floats. `AnimationClock` maps onto `AnimState`, animsets keep
  their game specific effects and can use frame events.
- **Windstille**: the game keeps `Sprite3D` and the armature code,
  frames become `Mesh` objects drawn with `canvas.draw(mesh, matrix)`.
  The editor creates a `Renderer` on the Gtk::GLArea context.


Known issues in dependencies
----------------------------

- surf: `surf::blit()` from a surface returned by `get_view()`
  corrupts the heap, wstdisplay copies rows itself to avoid it.
- surf: `surf-config.cmake` doesn't look up geom, consumers need
  `find_package(geom)` first.
- sexp-cpp: a comment at the end of a file without a trailing newline
  hung the lexer, fixed in sexp-cpp 67043e7, the flake input needs to
  be updated to pick it up.
- Windstille data: `images/explosion.sprite` and
  `images/hedgehog_die.sprite` are missing a closing parenthesis.
- Pingus data: some sprites reference images that don't exist, e.g.
  the editor `tb_*.png` icons.
