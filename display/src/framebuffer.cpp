// Windstille Display Library
// Copyright (C) 2002-2026 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "framebuffer.hpp"

#include <stdexcept>
#include <string>

#include "gl.hpp"

namespace wstdisplay {

namespace {

char const* framebuffer_status_string(GLenum status)
{
  switch (status)
  {
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
    case GL_FRAMEBUFFER_UNSUPPORTED: return "GL_FRAMEBUFFER_UNSUPPORTED";
    default: return "unknown framebuffer status";
  }
}

} // namespace

FramebufferPtr
Framebuffer::create(geom::isize const& size, TextureParams const& params, bool stencil)
{
  return FramebufferPtr(new Framebuffer(size, params, stencil));
}

Framebuffer::Framebuffer(geom::isize const& size, TextureParams const& params, bool stencil) :
  m_handle(0),
  m_stencil_buffer(0),
  m_texture(Texture::create(size, params))
{
  GLint old_framebuffer = 0;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_framebuffer);

  glGenFramebuffers(1, &m_handle);
  glBindFramebuffer(GL_FRAMEBUFFER, m_handle);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         m_texture->get_handle(), 0);

  if (stencil) {
    // GL_STENCIL_INDEX8 is the only stencil format GLES 2.0 guarantees
    glGenRenderbuffers(1, &m_stencil_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_stencil_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, size.width(), size.height());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_stencil_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
  }

  GLenum const status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(old_framebuffer));

  if (status != GL_FRAMEBUFFER_COMPLETE) {
    glDeleteFramebuffers(1, &m_handle);
    if (m_stencil_buffer) {
      glDeleteRenderbuffers(1, &m_stencil_buffer);
    }
    throw std::runtime_error(std::string("Framebuffer: incomplete: ") + framebuffer_status_string(status));
  }

  WST_CHECK_GL("creating framebuffer");
}

Framebuffer::~Framebuffer()
{
  glDeleteFramebuffers(1, &m_handle);
  if (m_stencil_buffer) {
    glDeleteRenderbuffers(1, &m_stencil_buffer);
  }
}

} // namespace wstdisplay

/* EOF */
