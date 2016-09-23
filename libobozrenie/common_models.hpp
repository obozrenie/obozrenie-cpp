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

#ifndef _COMMON_MODELS_HPP_
#define _COMMON_MODELS_HPP_

#include <experimental/optional>

#include <glibmm.h>
#include <json/json.h>

#include "exceptions.hpp"

namespace Obozrenie
{
const char* const CORE_COMPONENT_STRING = "Core";
const char* const BACKENDS_COMPONENT_STRING = "Backends";
const char* const UI_COMPONENT_STRING = "UI";

typedef Glib::ustring GameID;

template <typename T>
Glib::Variant<T>
make_variant(T v)
{
  return Glib::Variant<T>::create(v);
}

struct ConfigValue
{
  Glib::VariantType type;
  Glib::VariantBase data;
  Json::Value metadata;

  template <typename T>
  T value() const
  {
    return Glib::VariantBase::cast_dynamic<Glib::Variant<T>>(this->data).get();
  };

  ConfigValue() {}

  ConfigValue(Glib::VariantBase v)
  {
    data = v;
    type = v.get_type();
  }

  ConfigValue(Glib::VariantType t) { type = t; }
};

typedef std::map<Glib::ustring, ConfigValue> ConfStorage;

struct Player
{
  Glib::ustring name;
  std::map<Glib::ustring, Glib::ustring> info;
};

struct Server
{
  std::experimental::optional<Glib::ustring> name;
  std::experimental::optional<Glib::ustring> country;
  std::experimental::optional<Glib::ustring> game_mod;
  std::experimental::optional<Glib::ustring> game_type;
  std::experimental::optional<bool> need_pass;
  std::experimental::optional<bool> secure;
  std::experimental::optional<int> player_count;
  std::experimental::optional<int> player_limit;
  std::experimental::optional<int> spectator_count;
  std::experimental::optional<int> spectator_limit;
  std::experimental::optional<Glib::ustring> terrain;
  std::experimental::optional<int> ping;
  std::map<Glib::ustring, Glib::ustring> rules;
  std::list<Player> players;
};

typedef std::map<Glib::ustring, Server> ServerData;
typedef std::function<ServerData(GameID, ConfStorage)> QueryFunc;

struct Backend
{
  Glib::ustring name;
  Glib::ustring description;
  Glib::ustring version;
  QueryFunc f;
};
typedef std::function<Backend()> BackendInfoFunc;

template <typename T>
T
get_setting_from_storage(ConfStorage s, Glib::ustring id)
{
  try
  {
    return s.at(id).value<T>();
  }
  catch (std::out_of_range)
  {
    throw BackendError(Glib::ustring::compose("Missing setting in the provided ConfStorage : %1", id).c_str());
  }
  catch (std::bad_cast)
  {
    throw BackendError(Glib::ustring::compose("Invalid type of setting in ConfStorage : %1", id).c_str());
  }
}
}
#endif
