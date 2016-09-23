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

#include "util.hpp"

#include "exceptions.hpp"

#include <cstdio>
#include <ctime>
#include <experimental/filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <future>

#include <boost/algorithm/string/join.hpp>
#include <giomm.h>
#include <glibmm.h>
#include <json/json.h>

namespace Obozrenie
{
Glib::VariantBase
json_to_variant(Glib::VariantType t, Json::Value v)
{
  if (t.equal(Glib::VARIANT_TYPE_STRING))
  {
    return Glib::Variant<std::string>::create(v.asString());
  }
  else if (t.equal(Glib::VARIANT_TYPE_INT32))
  {
    return Glib::Variant<int>::create(v.asInt());
  }
  else if (t.equal(Glib::VARIANT_TYPE_DOUBLE))
  {
    return Glib::Variant<double>::create(v.asDouble());
  }
  else if (t.equal(Glib::VARIANT_TYPE_BOOL))
  {
    return Glib::Variant<bool>::create(v.asBool());
  }
  else if (t.equal(Glib::VARIANT_TYPE_STRING_ARRAY))
  {
    std::vector<std::string> vec;
    for (auto v2 : v)
    {
      auto str = v2.asString();
      vec.push_back(str);
    }

    return Glib::Variant<std::vector<std::string>>::create(vec);
  }
  else
  {
    throw DataParseError("Variant type not supported : " + t.get_string());
  }
}

std::string
get_string_from_resource(Glib::RefPtr<Gio::Resource> r, std::string path)
{
  auto b = r->lookup_data(path);
  auto s = b->get_size();
  return std::string((const char*)b->get_data(s));
}

std::string
exec(std::vector<std::string> argv, std::chrono::milliseconds wait_timeout)
{
  auto fut = std::async([argv]() {
    std::string data;
    int exit_status = 0;
    try
    {
      Glib::spawn_sync(std::experimental::filesystem::current_path().string(), argv, Glib::SpawnFlags::SPAWN_DEFAULT, Glib::SlotSpawnChildSetup(), &data, nullptr, &exit_status);
    }
    catch (const Glib::SpawnError& e)
    {
      throw PopenError(Glib::ustring::compose("%1 - %2", e.what(), boost::join(argv, " ")).c_str());
    }

    if (exit_status)
    {
      throw PopenError(Glib::ustring::compose("(%1) %2 - %3", exit_status, strerror(exit_status), boost::join(argv, " ")).c_str());
    }

    return data;
  });

  if (wait_timeout != std::chrono::milliseconds(0))
  {
    if (fut.wait_for(wait_timeout) == std::future_status::timeout)
    {
      throw PopenError(Glib::ustring::compose("Timed out - %1", boost::join(argv, " ")).c_str());
    }
  }

  return fut.get();
}

void
map_json_object(Json::Value m, JSONCallbackMap b, std::function<void(std::string)> cb_unknown_key)
{
  for (auto k : m.getMemberNames())
  {
    JSONCallback f;
    try
    {
      f = b.at(k);
    }
    catch (const std::out_of_range& e)
    {
      if (cb_unknown_key)
      {
        cb_unknown_key(k);
      }
      continue;
    }
    f(k, m[k]);
  }
}

std::string
json_to_string(Json::Value v, bool condensed)
{
  Json::StreamWriterBuilder builder;
  if (condensed)
  {
    builder.settings_["indentation"] = "";
  }
  return Json::writeString(builder, v);
}

void
json_to_file(Json::Value v, std::string filename, bool condensed)
{
  std::ofstream f;
  if (!f)
  {
    throw FopenError(("Failed to open file: " + filename).data());
  }

  f << json_to_string(v, condensed);
}

Json::Value
string_to_json(std::string data)
{
  Json::CharReaderBuilder builder;
  std::istringstream s(data);

  Json::Value m;
  std::string err;
  auto ok = Json::parseFromStream(builder, s, &m, &err);

  if (!ok)
  {
    throw DataParseError(err.data());
  }

  return m;
}

void
log_message(std::ostream& os, std::vector<std::string> ct, std::string msg)
{
  std::vector<std::string> msgvec{ boost::join(ct, " / "), msg };

  std::time_t t = std::time(nullptr);
  std::tm tm = *std::localtime(&t);
  std::ostringstream oss;
  oss << std::put_time(&tm, "%F %T");
  os << Glib::ustring::compose("%1 | %2\n", oss.str(), boost::join(msgvec, " | "));
}
}
