#include <SDL.h>

#include <iostream>
#include <random>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <geom/quad.hpp>
#include <geom/io.hpp>
#include <surf/save.hpp>

#include <wstdisplay/graphics_context.hpp>
#include <wstdisplay/opengl_window.hpp>
#include <wstdisplay/surface_drawing_parameters.hpp>
#include <wstdisplay/surface_manager.hpp>
#include <wstdisplay/font/ttf_font_manager.hpp>
#include <wstdisplay/font/ttf_font.hpp>
#include <wstsystem/system.hpp>

using namespace wstdisplay;

namespace {

void run()
{
  wstsys::System system;

  std::unique_ptr<OpenGLWindow> window = system.create_window("Geometry Test",
                                                              geom::isize(1280, 720));
  window->set_aspect(geom::isize(1280, 720));
  window->set_mode(OpenGLWindow::Mode::Window);
  window->set_resizable(true);
  window->set_anti_aliasing(0);
  //window->show();

  GraphicsContext gc;
  gc.set_aspect_size(geom::isize(1280, 720));

  SurfaceManager surface_manager;
  TTFFontManager font_manager;

  std::unique_ptr<TTFFont> font = font_manager.create_font("extra/Vera.ttf", 32);
  SurfacePtr surface = surface_manager.get("extra/tux.png");

  FramebufferPtr fb = Framebuffer::create(geom::isize(1280, 720));

  gc.push_framebuffer(fb);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  gc.pop_framebuffer();

  // vsync
  // SDL_GL_SetSwapInterval(0);

  std::random_device rd;
  std::mt19937 gen(rd());

  std::uniform_real_distribution<float> rand_x(0.0f, 1280.0f);
  std::uniform_real_distribution<float> rand_y(0.0f, 720.0f);
  std::uniform_real_distribution<float> rand_color(0.0f, 1.0f);
  std::uniform_real_distribution<float> rand_radius(0.0f, 256.0f);
  std::uniform_real_distribution<float> rand_rounded_radius(0.0f, 64.0f);
  std::uniform_real_distribution<float> rand_angle(0.0f, 360.0f);
  std::uniform_real_distribution<float> rand_scale(0.01f, 5.0f);

  int mode = 0;

  bool quit = false;

  system.sig_keyboard_event.connect([&](SDL_KeyboardEvent const& key){
    if (key.state == SDL_PRESSED)
    {
      switch (key.keysym.sym)
      {
        case SDLK_SPACE:
          mode += 1;
          break;

        case SDLK_ESCAPE:
          quit = true;
          break;

        case SDLK_f:
          if (window->get_mode() == OpenGLWindow::Mode::Window) {
            if (key.keysym.mod & KMOD_SHIFT) {
              window->set_mode(OpenGLWindow::Mode::Fullscreen);
            } else {
              window->set_mode(OpenGLWindow::Mode::FullscreenDesktop);
            }
          } else {
            window->set_mode(OpenGLWindow::Mode::Window);
          }
          break;

        case SDLK_s:
          {
            surf::SoftwareSurface screenshot = window->screenshot();
            std::filesystem::path outfile = "/tmp/out.jpg";
            std::cout << "Saving screenshot to " << outfile << std::endl;
            surf::save(screenshot, outfile);
          }
          break;
      }
    }
  });

  window->sig_resized.connect([&](geom::isize const& size){
    log_fatal("on_resize: {}", size);
    rand_x = std::uniform_real_distribution<float>(0.0f, static_cast<float>(size.width()));
    rand_y = std::uniform_real_distribution<float>(0.0f, static_cast<float>(size.height()));
    gc.set_aspect_size(size);
    fb = Framebuffer::create(size);

    // FIXME: workaround, as framebuffer are full of crap
    gc.push_framebuffer(fb);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gc.pop_framebuffer();
  });

  while (!quit)
  {
    // render
    { // render to framebuffer
      gc.push_framebuffer(fb);
      gc.push_matrix();
      gc.mult_matrix(glm::ortho(0.0f,
                                static_cast<float>(gc.size().width()),
                                static_cast<float>(gc.size().height()),
                                0.0f,
                                1000.0f,
                                -1000.0f));

      switch (mode % 11) {
        case 0:
          gc.fill_rect(geom::normalize(geom::frect(rand_x(gen), rand_y(gen), rand_x(gen), rand_y(gen))),
                       surf::Color(rand_color(gen), rand_color(gen), rand_color(gen), 1.0f));
          break;

        case 1:
          gc.draw_rect(geom::normalize(geom::frect(rand_x(gen), rand_y(gen), rand_x(gen), rand_y(gen))),
                       surf::Color(rand_color(gen), rand_color(gen), rand_color(gen), 1.0f));
          break;

        case 2:
          gc.fill_circle(geom::fpoint(rand_x(gen), rand_y(gen)),
                         rand_radius(gen),
                         surf::Color(rand_color(gen), rand_color(gen), rand_color(gen), 1.0f),
                         32);
          break;

        case 3:
          gc.draw_circle(geom::fpoint(rand_x(gen), rand_y(gen)),
                         rand_radius(gen),
                         surf::Color(rand_color(gen), rand_color(gen), rand_color(gen), 1.0f),
                         32);
          break;

        case 4:
          gc.fill_quad(geom::fquad(geom::fpoint(rand_x(gen), rand_y(gen)),
                                   geom::fpoint(rand_x(gen), rand_y(gen)),
                                   geom::fpoint(rand_x(gen), rand_y(gen)),
                                   geom::fpoint(rand_x(gen), rand_y(gen))),
                       surf::Color(rand_color(gen), rand_color(gen), rand_color(gen), 1.0f));
          break;

        case 5:
          gc.fill_rounded_rect(geom::normalize(geom::frect(rand_x(gen), rand_y(gen), rand_x(gen), rand_y(gen))),
                               rand_rounded_radius(gen),
                               surf::Color(rand_color(gen), rand_color(gen), rand_color(gen), 1.0f));
          break;

        case 6:
          gc.draw_rounded_rect(geom::normalize(geom::frect(rand_x(gen), rand_y(gen), rand_x(gen), rand_y(gen))),
                               rand_rounded_radius(gen),
                               surf::Color(rand_color(gen), rand_color(gen), rand_color(gen), 1.0f));
          break;

        case 7:
          gc.draw_grid(geom::fpoint(rand_x(gen), rand_y(gen)), geom::fsize(rand_radius(gen), rand_radius(gen)),
                       surf::Color(rand_color(gen), rand_color(gen), rand_color(gen), 1.0f));
          break;

        case 8:
          surface->draw(gc, geom::fpoint(rand_x(gen), rand_y(gen)));
          break;

        case 9:
          surface->draw(gc, SurfaceDrawingParameters()
                        .set_pos(geom::fpoint(rand_x(gen), rand_y(gen)))
                        .set_angle(rand_angle(gen))
                        .set_scale(rand_scale(gen)));
          break;

        case 10:
          font->draw(gc, geom::fpoint(rand_x(gen), rand_y(gen)).as_vec(), "Hello World",
                     surf::Color(rand_color(gen), rand_color(gen), rand_color(gen), 1.0f));
          break;
      }

      gc.pop_matrix();
      gc.pop_framebuffer();
    }

    { // render framebuffer to screen
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      fb->blit(fb->get_size(), fb->get_size(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    window->swap_buffers();
    system.update();
  }
}

} // namespace

int main()
{
  try {
    run();
  } catch (std::exception const& err) {
    std::cerr << "error: " << err.what() << std::endl;
  }
}

/* EOF */
