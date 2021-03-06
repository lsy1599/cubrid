/*
 * Copyright (C) 2008 Search Solution Corporation. All rights reserved by Search Solution.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

#include "monitor_collect.hpp"

namespace cubmonitor
{
  timer::timer (void)
    : m_timept (clock_type::now ())
  {
    //
  }

  void
  timer::reset (void)
  {
    m_timept = clock_type::now ();
  }

  duration
  timer::time (void)
  {
    time_point start_pt = m_timept;
    m_timept = clock_type::now ();
    return m_timept - start_pt;
  }

  void
  build_name_vector (std::vector<std::string> &names, const char *basename, const char *prefix)
  {
    names.push_back (std::string (prefix) + basename);
  }

}  // namespace cubmonitor
