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

#ifndef _GEOIP_HPP_
#define _GEOIP_HPP_

#include <mutex>
#include <string>

#include <GeoIP.h>

#ifndef DEFINE_EXCEPTION
#define DEFINE_EXCEPTION(ClassName, Message)                                              \
  class ClassName : public std::exception                                                 \
  {                                                                                       \
  private:                                                                                \
    std::string msg;                                                                      \
                                                                                          \
  public:                                                                                 \
    ClassName() { msg = std::string(Message); }                                           \
    ClassName(std::string arg) { msg = std::string(Message) + std::string(" : ") + arg; } \
    virtual const char* what() const throw() { return msg.c_str(); }                      \
  }

#endif

namespace Geoip
{

DEFINE_EXCEPTION(DataNotLoaded, "could not load data");
DEFINE_EXCEPTION(InvalidAddress, "invalid address specified");

class Geodata
{
private:
  GeoIP* data;
  std::string _filename;
  mutable std::mutex m;

public:
  std::string filename() { return this->_filename; }
  std::string country_code_by_addr(std::string) const;
  std::string country_code_by_name(std::string) const;

  Geodata(std::string);
  Geodata(const Geodata&) = delete;
  ~Geodata();
};
}

#endif
