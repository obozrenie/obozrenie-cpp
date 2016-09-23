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

#include "core.hpp"

#include "backend_qstat.hpp"
#include "backend_minetest.hpp"
#include "exceptions.hpp"
#include "util.hpp"

namespace Obozrenie
{
BackendInfoFunc
get_backend_data(BackendID id)
{
  std::map<BackendID, BackendInfoFunc> m;
  m["qstat"] = Obozrenie::Backends::QStat::get_information;
  m["minetest"] = Obozrenie::Backends::Minetest::get_information;

  try
  {
    return m.at(id);
  }
  catch (const std::out_of_range& e)
  {
    throw BackendError("Backend not found : " + id.raw());
  }
}

bool
GameTable::game_exists(GameID id)
{
  return this->data.find(id) != this->data.end();
}

void
GameTable::modify_game_entry(GameID id, std::function<void(GameEntry&)> cb)
{
  std::lock_guard<std::mutex> lg(this->m);

  try
  {
    cb(this->data.at(id));
  }
  catch (const std::out_of_range& e)
  {
    throw NoSuchGameError(id);
  }
}

void
GameTable::modify_game_entry(GameID id, std::function<void(const GameEntry&)> cb) const
{
  std::lock_guard<std::mutex> lg(this->m);

  try
  {
    cb(this->data.at(id));
  }
  catch (const std::out_of_range& e)
  {
    throw NoSuchGameError(id);
  }
}

void
GameTable::create_game_entry(GameID id)
{
  std::lock_guard<std::mutex> lg(this->m);

  if (this->game_exists(id))
  {
    throw GameExistsError(id);
  }
  else
  {
    this->data[id] = GameEntry();
  }

  this->changed(id);
}

void
GameTable::remove_game_entry(GameID id)
{
  std::lock_guard<std::mutex> lg(this->m);

  if (this->data.erase(id) == 0)
  {
    throw NoSuchGameError(id);
  }

  this->changed(id);
}

std::vector<GameID>
GameTable::get_game_list() const
{
  std::vector<GameID> v;

  std::lock_guard<std::mutex> lg(this->m);

  for (const auto& kv : this->data)
  {
    v.push_back(kv.first);
  }

  return v;
}

void
GameTable::set_backend(GameID id, BackendInfoFunc v)
{
  this->modify_game_entry(id, [v](GameEntry& e) { e.backend_info_func = v; });
}

BackendInfoFunc
GameTable::get_backend(GameID id) const
{
  BackendInfoFunc v;
  this->modify_game_entry(id, [&v](const GameEntry& e) { v = e.backend_info_func; });
  return v;
}

std::vector<Glib::ustring>
GameTable::get_setting_keys(GameID id, SettingGroup g) const
{
  std::vector<Glib::ustring> v;
  this->modify_game_entry(id, [g, &v](const GameEntry& e) {
    for (const auto& kv : e.settings.at(g))
    {
      v.push_back(kv.first);
    }
  });

  return v;
}

void
GameTable::create_setting(GameID id, Glib::VariantType t, SettingGroup g, Glib::ustring k)
{
  this->modify_game_entry(id, [this, id, t, g, k](GameEntry& e) {
    e.settings[g][k] = ConfigValue(t);

    this->changed(id);
    this->settings_changed(id);
  });
}

void GameTable::set_setting_metadata(GameID id, SettingGroup g, Glib::ustring k, Json::Value v) {
    this->modify_game_entry(id, [this, id, g, k, v](GameEntry& e) {
        try {
            auto& s = e.settings.at(g);
            try {
                s.at(k).metadata = v;
            } catch (const std::out_of_range& e) { throw InvalidSettingKeyError(e.what()); }
        } catch (const std::out_of_range& e) { throw InvalidConfStorageError(e.what()); }
    });
}

void GameTable::set_setting_value(GameID id, SettingGroup g, Glib::ustring k, Glib::VariantBase v, bool upsert) {
    this->modify_game_entry(id, [g, k, v, upsert](GameEntry& e) {
        try {
            auto s = e.settings.at(g);
            try { s.at(k); } catch (const std::out_of_range& e) { if (!upsert) throw InvalidSettingKeyError(); }
        } catch (const std::out_of_range& e) { throw InvalidConfStorageError(); }

        try { e.settings[g][k].data = v; } catch (const Json::LogicError& e) { throw SettingTypeMismatchError(e.what()); }
    });
}

ConfigValue GameTable::get_setting(GameID id, SettingGroup g, Glib::ustring k) const {
    ConfigValue v;
    this->modify_game_entry(id, [g, k, &v](const GameEntry& e) {
        try {
            auto s = e.settings.at(g);
            try { v = s.at(k); } catch (const std::out_of_range& e) { throw InvalidSettingKeyError(e.what()); }
        } catch (const std::out_of_range& e) { throw InvalidConfStorageError(e.what()); }
    });
    return v;
}

std::map<GameID, ConfigValue> GameTable::get_setting_map(SettingGroup g, Glib::ustring k) const {
    std::map<GameID, ConfigValue> m;

    for (auto e : this->data) { try { m[e.first] = e.second.settings.at(g).at(k); } catch (...) {} }

    return m;
}

ConfStorage GameTable::get_settings(GameID id, SettingGroup g) const {
    ConfStorage v;
    this->modify_game_entry(id, [&v, g](const GameEntry& e) { try { v = e.settings.at(g); } catch (const std::out_of_range& e) { throw InvalidConfStorageError(e.what()); } });

    return v;
}

void GameTable::remove_setting(GameID id, SettingGroup g, Glib::ustring k) {
  this->modify_game_entry(id, [this, id, g, k](GameEntry& e) {
    try { if (e.settings.at(g).erase(k) == 0) throw InvalidSettingKeyError(); } catch (const std::out_of_range& e) { throw InvalidConfStorageError(); }
    this->changed(id);
    this->settings_changed(id);
  });
}

void GameTable::set_query_status(GameID id, QueryStatus v)
{
  this->modify_game_entry(id, [this, id, v](GameEntry& e) {
    auto old_status = e.status;
    e.status = v;

    this->changed(id);
    this->status_changed(id, v, old_status);
  });
}

QueryStatus GameTable::get_query_status(GameID id) const {
  QueryStatus v;

  this->modify_game_entry(id, [&v](const GameEntry& e) { v = e.status; });

  return v;
}

void GameTable::insert_servers(GameID id, ServerData v, bool replace) {
  this->modify_game_entry(id, [this, id, v, replace](GameEntry& e) {
    if (replace) { e.servers = v; } else { std::for_each(std::begin(v), std::end(v), [&e](std::pair<Glib::ustring, Server> kv) { e.servers[kv.first] = kv.second; }); }
    this->changed(id);
    this->servers_changed(id, e.servers);
  });
}

ServerData
GameTable::get_servers(GameID id, ServerCompareFunc f) const
{
  ServerData matched;

  this->modify_game_entry(id, [&matched, f](const GameEntry& e) {
    if (!f)
    {
      matched = e.servers;
    }
    else
    {
      for (const auto& kv : e.servers)
      {
        if (f(kv))
        {
          matched[kv.first] = kv.second;
        }
      }
    }
  });

  return matched;
}

Server
GameTable::get_server_info_by_host(GameID id, Glib::ustring k) const
{
  Server v;

  this->modify_game_entry(id, [&v, k](const GameEntry& e) {
    try
    {
      v = e.servers.at(k);
    }
    catch (const std::out_of_range&)
    {
      throw NotFoundError(Glib::ustring::compose("No data for host %1 found", k));
    }
  });

  return v;
}

ServerData
GameTable::remove_servers(GameID id, ServerCompareFunc f)
{
  ServerData deleted;

  this->modify_game_entry(id, [this, id, &deleted, f](GameEntry& e) {
    if (!f)
    {
      deleted = e.servers;
      e.servers = ServerData();
    }
    else
    {
      ServerData nonmatch;
      for (const auto& kv : e.servers)
      {
        if (f(kv))
        {
          deleted[kv.first] = kv.second;
        }
        else
        {
          nonmatch[kv.first] = kv.second;
        }
      }
      e.servers = nonmatch;
    }
    this->changed(id);
    this->servers_changed(id, e.servers);
  });

  return deleted;
}

void
Core::read_game_lists(Json::Value m)
{
  std::lock_guard<std::mutex> lock(this->m);

  auto gt = std::make_shared<GameTable>();

  for (const auto& game_id : m.getMemberNames())
  {
    gt->create_game_entry(game_id);

    JSONCallbackMap b;
    b[name_setting] = [this, game_id, &gt](std::string, Json::Value v) {
      if (v.isString())
      {
        gt->create_setting(game_id, Glib::VARIANT_TYPE_STRING, SettingGroup::SYSTEM, name_setting);
        gt->set_setting_value(game_id, SettingGroup::SYSTEM, name_setting, make_variant(v.asString()));
      }
    };
    b["backend"] = [this, game_id, &gt](std::string, Json::Value v) {
      if (v.isString())
      {
        gt->set_backend(game_id, get_backend_data(v.asString()));
      }
    };
    b["settings"] = [this, game_id, &gt](std::string, Json::Value v) {
      for (auto k : v.getMemberNames())
      {
        auto entry_data = v[k];
        auto typestring = entry_data.get("type", Json::Value(Json::nullValue));
        if (typestring.isString())
        {
          auto t = typestring.asString();
          if (g_variant_type_string_is_valid(t.c_str()))
          {
            Glib::VariantType vtype(t);
            gt->create_setting(game_id, vtype, SettingGroup::USER, k);
            gt->set_setting_metadata(game_id, SettingGroup::USER, k, v[k]);
            auto defaultnode = entry_data.get("default", Json::Value(Json::nullValue));
            if (!defaultnode.isNull())
            {
              gt->set_setting_value(game_id, SettingGroup::USER, k, json_to_variant(vtype, defaultnode));
            }
          }
        }
      }
    };
    map_json_object(m[game_id], b);
  }
  this->game_table = gt;
}

void
Core::refresh_servers(GameID id, bool is_async, std::function<void(const std::exception&)> error_handler, boost::signals2::signal<void()>* cancellable)
{
  {
    std::lock_guard<std::mutex> lock(this->m);

    auto status = this->game_table->get_query_status(id);
    if (status == QueryStatus::WORKING)
    {
      return;
    }

    auto new_status = QueryStatus::WORKING;
    this->game_table->set_query_status(id, new_status);
  }

  auto fn = [this, id, error_handler, &cancellable]() {
    ServerData data;
    try
    {
      auto b = this->game_table->get_backend(id)();
      this->logger(std::vector<std::string>{ CORE_COMPONENT_STRING, BACKENDS_COMPONENT_STRING, b.name }, "Refreshing servers for " + id);

      if (!b.f)
      {
        throw BackendError("no query function");
      }

      bool is_cancelled = false;
      if (cancellable)
      {
        cancellable->connect([&is_cancelled]() {
          is_cancelled = true;

          throw PopenError("Cancelled");
        });
      }

      auto f = std::async(b.f, id, this->game_table->get_settings(id, SettingGroup::USER));
      auto recvdata = f.get();

      for (const auto& kv : recvdata)
      {
        try
        {
          auto host = kv.first;
          auto info = kv.second;

          if (this->geocoder)
          {
            info.country = this->geocoder->country_code_by_name(host);
          }
          data[host] = info;
        }
        catch (const std::experimental::bad_optional_access& e)
        {
        }
      }
      this->logger(std::vector<std::string>{ CORE_COMPONENT_STRING, BACKENDS_COMPONENT_STRING, b.name }, "Parsed servers for " + id);
    }
    catch (const std::exception& e)
    {
      this->logger(std::vector<std::string>{ CORE_COMPONENT_STRING }, Glib::ustring::compose("Error refreshing servers for %1: %2", id, e.what()));
      if (error_handler)
      {
        error_handler(e);
      }
      this->game_table->set_query_status(id, QueryStatus::ERROR);
      throw;
    }
    this->game_table->insert_servers(id, data, true);
    this->logger(std::vector<std::string>{ CORE_COMPONENT_STRING }, "Loaded servers into game table for " + id);
    this->game_table->set_query_status(id, QueryStatus::READY);
    return;
  };

  if (is_async)
  {
    this->async_cb(fn);
  }
  else
  {
    fn();
  }
}
}
