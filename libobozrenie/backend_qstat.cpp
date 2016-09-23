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

#include "backend_qstat.hpp"

#include "common_models.hpp"
#include "exceptions.hpp"
#include "util.hpp"
#include "xmlpp_util.hpp"

#include <experimental/filesystem>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include <boost/algorithm/string/join.hpp>
#include <glibmm.h>

namespace Obozrenie
{
namespace Backends
{
namespace QStat
{
const auto COLOR_CODE_PATTERN = "[\\^](.)";

namespace filesystem = std::experimental::filesystem;

std::vector<std::string>
make_qstat_cmd(Glib::ustring master_type, std::map<std::string, std::string> rules, std::vector<std::string> master_server_uri)
{
  std::vector<std::string> rulevec;
  for (auto r : rules)
  {
    rulevec.push_back(r.first + "=" + r.second);
  }

  std::string rulestring;
  if (!rulevec.empty())
  {
    rulestring = "," + boost::join(rulevec, ",");
  }

  std::vector<std::string> cmd;
  cmd.push_back("-xml");
  cmd.push_back("-utf8");
  cmd.push_back("-maxsim");
  cmd.push_back("9999");
  cmd.push_back("-R");
  cmd.push_back("-P");
  cmd.push_back(Glib::ustring("-") + Glib::ustring(master_type).lowercase() + rulestring);
  cmd.push_back(boost::join(master_server_uri, " "));

  return cmd;
}

Player parse_player_entry(const xmlpp::Node& data_node) {
    Player e;

    xmlpp::util::CallbackMap cb_data;
    cb_data["name"] = [&e](const auto& v) { e.name = xmlpp::util::get_string(v); };
    cb_data["score"] = [&e](const auto& v) { e.info["score"] = xmlpp::util::get_string(v); };
    cb_data["ping"] = [&e](const auto& v) { e.info["ping"] = xmlpp::util::get_string(v); };
    xmlpp::util::map_node(data_node, cb_data);

    return e;
}

std::pair<Glib::ustring, Server>
parse_server_entry(const xmlpp::Node& m, std::string server_type)
{
  auto parsed_type = xmlpp::util::get_string(m, "@type");
  if (parsed_type != server_type)
  {
    throw InvalidServerType(parsed_type);
  }

  Glib::ustring host;
  Server data;

  xmlpp::util::CallbackMap cb_data;
  cb_data["hostname"] = [&host](const auto& v) { host = xmlpp::util::get_string(v); };
  cb_data["name"] = [&data](const auto& v) { data.name = Glib::Regex::create(COLOR_CODE_PATTERN)->replace(xmlpp::util::get_string(v), 0, Glib::ustring(), static_cast<Glib::RegexMatchFlags>(0)); };
  cb_data["gametype"] = [&data](const auto& v) { data.game_type = xmlpp::util::get_string(v); };
  cb_data["map"] = [&data](const auto& v) { data.terrain = xmlpp::util::get_string(v); };
  cb_data["numplayers"] = [&data](const auto& v) { data.player_count = xmlpp::util::get_number<int>(v); };
  cb_data["maxplayers"] = [&data](const auto& v) { data.player_limit = xmlpp::util::get_number<int>(v); };
  cb_data["numspectators"] = [&data](const auto& v) { data.spectator_count = xmlpp::util::get_number<int>(v); };
  cb_data["maxspectators"] = [&data](const auto& v) { data.spectator_limit = xmlpp::util::get_number<int>(v); };
  cb_data["ping"] = [&data](const auto& v) { data.ping = xmlpp::util::get_number<int>(v); };
  cb_data["rules"] = [&data](const auto& v) {
    for (auto rule_node : v.find(".//rule"))
    {
      auto k = xmlpp::util::get_string(*rule_node, "@name");
      auto v = xmlpp::util::get_string(*rule_node);

      data.rules[k] = v;
    }
  };
  cb_data["players"] = [&data](const auto& v) {
    for (auto player_node : v.find(".//player")) {
      auto e = parse_player_entry(*player_node);

      if (!e.name.empty())
      {
        data.players.push_back(e);
      }
    }
  };
  xmlpp::util::map_node(m, cb_data);

  for (auto kv : data.rules)
  {
    if (std::set<Glib::ustring>{ "punkbuster", "sv_punkbuster", "secure" }.count(kv.first))
    {
      data.secure = (kv.second == "0" ? false : true);
    }
    if (std::set<Glib::ustring>{ "g_needpass", "needpass", "si_usepass", "pswrd", "password" }.count(kv.first))
    {
      data.need_pass = (kv.second == "0" ? false : true);
    }
  }

  if (host.empty())
  {
    throw DataParseError("Empty host.");
  }

  return std::make_pair(host, data);
}

ServerData
parse_xml(Glib::ustring xml_data, std::string server_type)
{
  xmlpp::util::EasyDocument doc;
  doc.parse(xml_data);

  ServerData data;

  xmlpp::util::CallbackMap cb_data;
  cb_data["server"] = [&data, server_type](const auto& v) {
    try
    {
      data.emplace(parse_server_entry(v, server_type));
    }
    catch (...)
    {
    }
  };
  xmlpp::util::map_node(doc(), cb_data);

  return data;
}

ServerData
query(GameID id, ConfStorage settings)
{
  auto qstat_path = get_setting_from_storage<std::string>(settings, "qstat_path");
  auto master_type = get_setting_from_storage<std::string>(settings, "qstat_master_type");
  auto server_type = get_setting_from_storage<std::string>(settings, "qstat_server_type");
  auto master_server_uri = get_setting_from_storage<std::vector<std::string>>(settings, "master_server_uri");

  std::map<std::string, std::string> rules;
  try
  {
    auto gt = get_setting_from_storage<std::string>(settings, "qstat_game_type");
    rules["gametype"] = gt;
  }
  catch (...)
  {
  }

  auto cmd = make_qstat_cmd(master_type, rules, master_server_uri);
  cmd.insert(std::begin(cmd), qstat_path);

  auto data = Obozrenie::exec(cmd);

  return parse_xml(data, server_type);
}

Backend
get_information()
{
  return Backend{.name = QSTAT_COMPONENT_STRING, .description = "QStat backend", .version = "1.0", .f = query };
}
}
}
}
