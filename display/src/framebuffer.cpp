// SPDX-FileCopyrightText: 2002-2026 Ingo Ruhnke <grumbel@gmail.com>
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
Framebuffer::create(geom::isize const& size, TextureParams const& params, bool depth, bool stencil)
{
  return FramebufferPtr(new Framebuffer(size, params, depth, stencil));
}

void
Framebuffer::attach_renderbuffer(uint32_t& handle, uint32_t format, uint32_t attachment, geom::isize const& size)
{
  glGenRenderbuffers(1, &handle);
  glBindRenderbuffer(GL_RENDERBUFFER, handle);
  glRenderbufferStorage(GL_RENDERBUFFER, format, size.width(), size.height());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, handle);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

Framebuffer::Framebuffer(geom::isize const& size, TextureParams const& params, bool depth, bool stencil) :
  m_handle(0),
  m_depth_buffer(0),
  m_stencil_buffer(0),
  m_texture(Texture::create(size, params))
{
  GLint old_framebuffer = 0;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_framebuffer);

  glGenFramebuffers(1, &m_handle);
  glBindFramebuffer(GL_FRAMEBUFFER, m_handle);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         m_texture->get_handle(), 0);

  bool const gles = detail::current_caps().is_gles();
  constexpr GLenum depth24_stencil8 = 0x88F0; // GL_DEPTH24_STENCIL8 and GL_DEPTH24_STENCIL8_OES

  if (depth && stencil && !gles) {
    attach_renderbuffer(m_depth_buffer, depth24_stencil8, GL_DEPTH_STENCIL_ATTACHMENT, size);
  } else {
    // GLES 2.0 only guarantees 16 bit depth and separate 8 bit stencil
    if (depth) {
      attach_renderbuffer(m_depth_buffer, gles ? GL_DEPTH_COMPONENT16 : GL_DEPTH_COMPONENT24,
                          GL_DEPTH_ATTACHMENT, size);
    }
    if (stencil) {
      attach_renderbuffer(m_stencil_buffer, GL_STENCIL_INDEX8, GL_STENCIL_ATTACHMENT, size);
    }
  }

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

  if (status != GL_FRAMEBUFFER_COMPLETE && gles && depth && stencil) {
    // some GLES drivers only support depth and stencil packed together
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
    glDeleteRenderbuffers(1, &m_depth_buffer);
    glDeleteRenderbuffers(1, &m_stencil_buffer);
    m_stencil_buffer = 0;
    attach_renderbuffer(m_depth_buffer, depth24_stencil8, GL_DEPTH_ATTACHMENT, size);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depth_buffer);
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(old_framebuffer));

  if (status != GL_FRAMEBUFFER_COMPLETE) {
    destroy();
    throw std::runtime_error(std::string("Framebuffer: incomplete: ") + framebuffer_status_string(status));
  }

  WST_CHECK_GL("creating framebuffer");
}

Framebuffer::~Framebuffer()
{
  destroy();
}

void
Framebuffer::destroy()
{
  if (m_handle) {
    glDeleteFramebuffers(1, &m_handle);
    m_handle = 0;
  }
  if (m_depth_buffer) {
    glDeleteRenderbuffers(1, &m_depth_buffer);
    m_depth_buffer = 0;
  }
  if (m_stencil_buffer) {
    glDeleteRenderbuffers(1, &m_stencil_buffer);
    m_stencil_buffer = 0;
  }
}

} // namespace wstdisplay

/* EOF */
