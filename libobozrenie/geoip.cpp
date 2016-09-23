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

#include "geoip.hpp"

#include <mutex>

namespace Geoip
{
Geodata::Geodata(std::string f)
{
  GeoIP* v = GeoIP_open(f.c_str(), GEOIP_MEMORY_CACHE);
  if (v == nullptr)
  {
    throw DataNotLoaded(f);
  }

  this->data = v;
  this->_filename = f;
}

Geodata::~Geodata()
{
  GeoIP_delete(this->data);
}

std::string
Geodata::country_code_by_addr(std::string addr) const
{
  std::lock_guard<std::mutex> lock(this->m);
  return GeoIP_country_code_by_addr(this->data, addr.c_str());
}

std::string
Geodata::country_code_by_name(std::string name) const
{
  std::lock_guard<std::mutex> lock(this->m);
  return GeoIP_country_code_by_name(this->data, name.c_str());
}
}
