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

#ifndef _BACKEND_MINETEST_HPP_
#define _BACKEND_MINETEST_HPP_

#include "common_models.hpp"
#include "exceptions.hpp"

namespace Obozrenie
{
namespace Backends
{
namespace Minetest
{
ServerData query(GameID, ConfStorage)
{
  throw BackendError("backend stubbed");
};
Backend
get_information()
{
  return Backend{ .name = "Minetest", .description = "Minetest backend", .version = "0.0", .f = query };
};
}
}
}
#endif
