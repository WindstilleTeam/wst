// SPDX-FileCopyrightText: 2007 Ingo Ruhnke <grumbel@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <surf/software_surface.hpp>

#include "font/no_font_effect.hpp"

namespace wstdisplay {

void
NoFontEffect::blit(surf::SoftwareSurface& target, const FT_Bitmap& brush, int x_pos, int y_pos) const
{
  int start_x = std::max(0, -x_pos);
  int start_y = std::max(0, -y_pos);

  int end_x = std::min(static_cast<int>(brush.width), target.get_width()  - x_pos);
  int end_y = std::min(static_cast<int>(brush.rows), target.get_height() - y_pos);

  uint8_t* target_buf = static_cast<uint8_t*>(target.get_data());

  int target_pitch = target.get_pitch();

  for (int y = start_y; y < end_y; ++y)
    for (int x = start_x; x < end_x; ++x)
    {
      int target_pos = (y + y_pos) * target_pitch + 4*(x + x_pos);
      int brush_pos  = y * brush.pitch + x;

      target_buf[target_pos + 0] = 255;
      target_buf[target_pos + 1] = 255;
      target_buf[target_pos + 2] = 255;
      target_buf[target_pos + 3] = brush.buffer[brush_pos];
    }
}

} // namespace wstdisplay

/* EOF */
