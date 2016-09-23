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

#ifndef _EXCEPTION_LIST_HPP_
#define _EXCEPTION_LIST_HPP_

#include <exception>
#include <string>

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
    virtual const char* what() const throw() { return msg.data(); }                       \
  }
#endif

namespace Obozrenie
{
DEFINE_EXCEPTION(NullException, "unexpected null");
DEFINE_EXCEPTION(NoSuchGameError, "Specified game does not exist");
DEFINE_EXCEPTION(GameExistsError, "Specified game already exists");
DEFINE_EXCEPTION(NotFoundError, "Data not found");
DEFINE_EXCEPTION(FopenError, "fopen() failed!");
DEFINE_EXCEPTION(PopenError, "popen() failed!");
DEFINE_EXCEPTION(InvalidUTF8Error, "Broken UTF-8 data detected");
DEFINE_EXCEPTION(DataParseError, "Error parsing data");
DEFINE_EXCEPTION(InvalidConfStorageError, "Invalid settings storage");
DEFINE_EXCEPTION(InvalidSettingKeyError, "Invalid setting key");
DEFINE_EXCEPTION(SettingTypeMismatchError, "Setting type mismatch");
DEFINE_EXCEPTION(BackendError, "Backend error");
}
#endif
