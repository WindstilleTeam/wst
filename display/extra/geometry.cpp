#include <SDL.h>

#include <iostream>
#include <random>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <geom/io.hpp>
#include <wstdisplay/graphics_context.hpp>
#include <wstdisplay/opengl_window.hpp>

using namespace wstdisplay;

int main()
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::ostringstream msg;
    msg << "Couldn't initialize SDL: " << SDL_GetError();
    throw std::runtime_error(msg.str());
  }
  atexit(SDL_Quit);

  std::random_device rd;
  std::mt19937 gen(rd());

  std::uniform_real_distribution<float> rand_x(0.0f, 1280.0f);
  std::uniform_real_distribution<float> rand_y(0.0f, 720.0f);
  std::uniform_real_distribution<float> rand_color(0.0f, 1.0f);

  OpenGLWindow window("Geometry Test", geom::isize(1280, 720), geom::isize(1280, 720),
                      false, 0);

  GraphicsContext gc;
  gc.set_aspect_size(geom::isize(1280, 720));

  struct Entity {
    geom::frect rect;
    surf::Color color;
  };

  std::deque<Entity> entities;

  bool quit = false;
  while (!quit) {
    // process input
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch(event.type)
      {
        case SDL_QUIT:
          // FIXME: This should be a bit more gentle, but will do for now
          std::cout << "Ctrl-c or Window-close pressed, game is going to quit" << std::endl;
          quit = true;
          break;

        case SDL_KEYDOWN:
          switch (event.key.keysym.sym)
          {
            case SDLK_ESCAPE:
              quit = true;
              break;
          }
          break;
      }
    }

    // update
    for (int i = 0; i < 10; ++i) {
      entities.emplace_back(geom::normalize(geom::frect(rand_x(gen), rand_y(gen), rand_x(gen), rand_y(gen))),
                            surf::Color(rand_color(gen), rand_color(gen), rand_color(gen), 1.0f));
    }

    while (entities.size() > 1000) {
      entities.pop_front();
    }

    // render
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gc.push_matrix();
    gc.mult_matrix(glm::ortho(0.0f,
                              static_cast<float>(1280),
                              static_cast<float>(720),
                              0.0f,
                              1000.0f,
                              -1000.0f));

    gc.fill_rect(geom::frect(0, 0, 1, 0.5f), surf::Color(1.0f, 1.0f, 1.0f));

    for (auto const& entity : entities) {
      gc.fill_rect(entity.rect, entity.color);
    }
    gc.pop_matrix();

    window.swap_buffers();
  }

  return 0;
}

/* EOF */
