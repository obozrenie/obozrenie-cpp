// This file is part of Obozrenie.

// https://github.com/skybon/obozrenie
// Copyright (C) 2016 Artem Vorotnikov
//
// Obozrenie is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// Obozrenie is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Obozrenie.  If not, see <http://www.gnu.org/licenses/>.

std::string make_quake_launch_string(GameID id, Server data, ConfStorage settings) {
    std::string v;

    return v;
}

Launcher get_launcher(std::string id) {
  if (id == "quake") {
    return {.name = "Quake launcher", .description = "Launcher used for Quake and Source games.", .func = make_quake_launch_string}  
  } else {
    throw NotFoundError("No such launcher.");
  }
}
