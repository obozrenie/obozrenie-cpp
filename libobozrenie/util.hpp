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

#ifndef _UTIL_HPP_
#define _UTIL_HPP_

#include <functional>
#include <iostream>
#include <map>

#include <glibmm.h>
#include <giomm.h>
#include <json/json.h>
#include <libxml++/libxml++.h>

namespace Obozrenie
{
std::string get_string_from_resource(Glib::RefPtr<Gio::Resource> r, std::string path);
Glib::VariantBase json_to_variant(Glib::VariantType t, Json::Value v);

typedef std::function<void(std::string, Json::Value)> JSONCallback;
typedef std::map<std::string, JSONCallback> JSONCallbackMap;

std::string exec(std::vector<std::string>, std::chrono::milliseconds = std::chrono::milliseconds(0));

void map_json_object(Json::Value, JSONCallbackMap, std::function<void(std::string)> = nullptr);

std::string json_to_string(Json::Value, bool = false);

void json_to_file(Json::Value, std::string, bool = false);

Json::Value string_to_json(std::string);

void log_message(std::ostream&, std::vector<std::string>, std::string);
}
#endif
