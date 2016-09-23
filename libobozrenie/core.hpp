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

#ifndef _CORE_HPP_
#define _CORE_HPP_

#include <functional>
#include <iostream>
#include <list>
#include <mutex>
#include <vector>

#include <boost/signals2.hpp>
#include <json/json.h>
#include <glibmm.h>

#include "common_models.hpp"
#include "geoip.hpp"
#include "ThreadPool.hpp"

namespace Obozrenie
{
typedef Glib::ustring BackendID;
BackendInfoFunc get_backend_data(BackendID id);

typedef std::function<GameID()> QueryCallback;

enum class SettingGroup
{
  USER,
  SYSTEM,
  BACKEND
};

enum class QueryStatus
{
  EMPTY,
  READY,
  WORKING,
  ERROR
};

typedef std::map<SettingGroup, ConfStorage> GameSettings;
typedef std::function<bool(std::pair<Glib::ustring, Server>)> ServerCompareFunc;

struct GameEntry
{
  QueryStatus status;

  std::map<SettingGroup, ConfStorage> settings;
  ServerData servers;

  BackendInfoFunc backend_info_func;

  GameEntry() { status = QueryStatus::EMPTY; }
};

const char* const name_setting = "name";

class GameTable
{
private:
  mutable std::mutex m;
  std::map<Glib::ustring, GameEntry> data;

  bool game_exists(GameID);
  void modify_game_entry(GameID, std::function<void(GameEntry&)>);
  void modify_game_entry(GameID, std::function<void(const GameEntry&)>) const;

public:
  boost::signals2::signal<void(GameID)> changed;
  boost::signals2::signal<void(GameID, QueryStatus, QueryStatus)> status_changed;
  boost::signals2::signal<void(GameID)> settings_changed;
  boost::signals2::signal<void(GameID, ServerData)> servers_changed;

  void create_game_entry(GameID);
  void remove_game_entry(GameID);
  std::vector<GameID> get_game_list() const;

  void set_backend(GameID, BackendInfoFunc);
  BackendInfoFunc get_backend(GameID) const;

  void set_query_status(GameID, QueryStatus);
  QueryStatus get_query_status(GameID) const;

  std::vector<Glib::ustring> get_setting_keys(GameID, SettingGroup) const;
  void create_setting(GameID, Glib::VariantType, SettingGroup, Glib::ustring);

  void set_setting_metadata(GameID, SettingGroup, Glib::ustring, Json::Value);
  template <typename T>
  void set_setting(GameID id, SettingGroup g, Glib::ustring k, T v, bool upsert = false)
  {
    set_setting_value(id, g, k, Glib::Variant<T>::create(v), upsert);
  }
  void set_setting_value(GameID, SettingGroup, Glib::ustring, Glib::VariantBase, bool = false);
  ConfigValue get_setting(GameID, SettingGroup, Glib::ustring) const;
  std::map<GameID, ConfigValue> get_setting_map(SettingGroup, Glib::ustring) const;

  ConfStorage get_settings(GameID, SettingGroup) const;
  void remove_setting(GameID, SettingGroup, Glib::ustring);

  void insert_servers(GameID, ServerData, bool = false);
  ServerData get_servers(GameID, ServerCompareFunc = nullptr) const;
  Server get_server_info_by_host(GameID, Glib::ustring) const;
  ServerData remove_servers(GameID, ServerCompareFunc = nullptr);
};

class Core
{
private:
  mutable std::mutex m;
  std::shared_ptr<ThreadPool> pool;
  std::shared_ptr<Geoip::Geodata> geocoder;
  std::function<void(std::function<void()>)> async_cb;
  std::map<std::string, BackendInfoFunc> backend_map;

public:
  std::function<void(std::vector<std::string>, std::string)> logger;
  std::shared_ptr<GameTable> game_table;
  boost::signals2::signal<void(GameID)> refresh_started;
  boost::signals2::signal<void(GameID)> refresh_complete;
  void refresh_servers(GameID, bool = true, std::function<void(const std::exception&)> = nullptr, boost::signals2::signal<void()>* = nullptr );
  void read_game_lists(Json::Value);

  Core(std::shared_ptr<ThreadPool> p = nullptr)
  {
    if (!p)
    {
      this->async_cb = [](std::function<void()> fn) { std::thread(fn).detach(); };
    }
    else
    {
      this->async_cb = [this](std::function<void()> fn) { this->pool->enqueue(fn); };
    }
  }
  Core(const Core&) = delete;
};
}

#endif
