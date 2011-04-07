/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2011 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "AutoQNH.hpp"
#include "NMEA/Info.hpp"
#include "NMEA/Derived.hpp"
#include "Components.hpp"
#include "Engine/Waypoint/Waypoints.hpp"

void
AutoQNH(const NMEA_INFO &basic, DERIVED_INFO &calculated)
{
  #define QNH_TIME 10

  static unsigned countdown_autoqnh = QNH_TIME;

  if (!calculated.flight.OnGround // must be on ground
      || !countdown_autoqnh    // only do it once
      || !basic.gps.real // never in replay mode / simulator
      || !basic.LocationAvailable // Reject if no valid GPS fix
      || !basic.PressureAltitudeAvailable // Reject if no pressure altitude
      || basic.QNHAvailable // Reject if QNH already known
    ) {
    if (countdown_autoqnh<= QNH_TIME) {
      countdown_autoqnh= QNH_TIME; // restart if havent performed
    }
    return;
  }

  if (countdown_autoqnh<= QNH_TIME)
    countdown_autoqnh--;

  if (!countdown_autoqnh) {
    const Waypoint *next_wp;
    next_wp = way_points.lookup_location(basic.Location, fixed(1000));

    if (next_wp && next_wp->is_airport()) {
      calculated.pressure.set_QNH(basic.pressure.FindQNHFromPressureAltitude(basic.PressureAltitude, next_wp->Altitude));
      calculated.pressure_available.update(basic.Time);
    } else if (calculated.TerrainValid) {
      calculated.pressure.set_QNH(basic.pressure.FindQNHFromPressureAltitude(basic.PressureAltitude, calculated.TerrainAlt));
      calculated.pressure_available.update(basic.Time);
    } else
      return;

    countdown_autoqnh = UINT_MAX; // disable after performing once
  }
}
