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

#include <functional>
#include <iostream>

#include <boost/algorithm/string/join.hpp>
#include <gtkmm.h>
#include <libobozrenie/libobozrenie.hpp>

#include "helpers.hpp"
#include "models.hpp"
#include "application.hpp"

#include "config.hpp"
#include "data.c"

int
main(int argc, char* argv[])
{
  auto cout_ptr = std::shared_ptr<std::ostream>(&std::cout, [](void*) {});
  std::vector<std::string> ct_cat_vec = { Obozrenie::CORE_COMPONENT_STRING };
  auto log_core = [cout_ptr, ct_cat_vec](auto msg) { Obozrenie::log_message(*cout_ptr, ct_cat_vec, msg); };

  std::shared_ptr<ThreadPool> pool = nullptr;
#ifdef ENABLE_THREADPOOL
  pool = std::shared_ptr<ThreadPool>(new ThreadPool(std::thread::hardware_concurrency()));
#endif

  auto core = std::make_shared<Obozrenie::Core>();
  core->logger = [cout_ptr](auto cat, auto msg) { Obozrenie::log_message(*cout_ptr, cat, msg); };
  core->read_game_lists(Obozrenie::string_to_json(Obozrenie::get_string_from_resource(Glib::wrap(io_get_resource()), "/io/obozrenie/game_lists.json")));

  std::vector<std::string> game_names;
  for (const auto& kv : core->game_table->get_setting_map(Obozrenie::SettingGroup::SYSTEM, Obozrenie::name_setting))
  {
    game_names.push_back(kv.second.value<std::string>());
  }
  log_core(boost::join(game_names, ", "));

  auto g_app = Gtk::Application::create(argc, argv, application_id);
  std::map<Glib::ustring, Glib::RefPtr<Gdk::Pixbuf>> icons;
  for (const auto& id : core->game_table->get_game_list())
  {
    for (const auto& f : { "svg", "png" })
    {
      try
      {
        icons[Obozrenie::GTK::pixbuf_gameid(id)] = Gdk::Pixbuf::create_from_resource(Glib::ustring::compose("/io/obozrenie/game_icons/%1.%2", id, f).raw(), 24, 24, true);
      }
      catch (const Glib::Error& e)
      {
        continue;
      }
    }
  }

  auto logo = Gdk::Pixbuf::create_from_resource("/io/obozrenie/obozrenie.svg");
  auto logo_short = Gdk::Pixbuf::create_from_resource("/io/obozrenie/obozrenie-short.svg");

  std::string geoip_filename("/usr/share/GeoIP/GeoIP.dat");
  std::shared_ptr<Geoip::Geodata> geoip;
  try
  {
    geoip = std::make_shared<Geoip::Geodata>(geoip_filename);
    log_core(Glib::ustring::compose("Successfully opened GeoIP data file %1.", geoip_filename));
  }
  catch (...)
  {
    log_core(Glib::ustring::compose("Failed to load GeoIP data file %1. Geocoding has been disabled.", geoip_filename));
  }

  icons["logo"] = logo;
  icons["logo-short"] = logo_short;

  auto log_fn = [cout_ptr](auto msg) { return Obozrenie::log_message(*cout_ptr, std::vector<std::string>(), msg); };

  Obozrenie::GTK::Application app(pool, log_fn, g_app, core, *Gtk::Builder::create_from_resource("/io/obozrenie/obozrenie_gtk.ui").operator->(), icons);

  return app.start();
}
