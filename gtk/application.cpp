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

#include "application.hpp"

#include <iostream>

#include "config.hpp"

namespace Obozrenie
{
namespace GTK
{

const auto error_message_setting = "gtk_error_label";

void
Application::show_about_dialog()
{
  Gtk::AboutDialog v;

  v.set_program_name(project_name);
  v.set_version(version);
  v.set_comments("Simple and easy to use game server browser.");
  v.set_copyright("Copyright (c) 2015-2016 Artem Vorotnikov");
  v.set_logo(this->pixbufs.at("logo-short"));
  v.set_license_type(Gtk::License::LICENSE_GPL_3_0);

  v.set_transient_for(*this->main_window.operator->());

  v.run();
}

void
Application::on_game_preferences_button_clicked_cb()
{
}

void
Application::on_server_browser_view_selection_changed_cb()
{
  auto iter = this->server_browser_view->get_selection()->get_selected();

  if (iter)
  {
    auto& row = *iter;
    this->server_connect_game_combobox->set_active_id(row[this->server_list_columns.game_id]);
    this->server_connect_host_entry->set_text(row[this->server_list_columns.host]);
  }
}

void
Application::present_servers(GameID id)
{
  this->selection_signal_connection.block();
  this->populate_server_list(id);
  this->selection_signal_connection.unblock();
  this->server_browser_pager->set_current_page(int(GameBrowserPages::SERVERS));
}

void
Application::on_status_changed_cb(GameID id, Obozrenie::QueryStatus new_status)
{
  auto selected = (*this->game_browser_view->get_selection()->get_selected())[this->game_list_columns.id];

  auto game_row = *search_model(this->game_list, this->game_list_columns.id, id);
  auto status_icon_cell = game_row[this->game_list_columns.status_icon];

  switch (new_status)
  {
  case QueryStatus::EMPTY:
    status_icon_cell = nullptr;
    if (id == selected)
    {
      this->refresh_button->set_sensitive(true);
      this->server_browser_pager->set_current_page(int(GameBrowserPages::WELCOME));
    }
  case QueryStatus::ERROR:
    status_icon_cell = this->themed_icons.error;
    if (id == selected)
    {
      this->refresh_button->set_sensitive(true);
      this->server_browser_pager->set_current_page(int(GameBrowserPages::ERROR));
      this->error_message->set_text(this->core->game_table->get_setting(id, Obozrenie::SettingGroup::SYSTEM, error_message_setting).value<std::string>());
    }
    break;
  case QueryStatus::WORKING:
    status_icon_cell = this->themed_icons.working;
    if (id == selected)
    {
      this->refresh_button->set_sensitive(false);
      this->server_browser_pager->set_current_page(int(GameBrowserPages::LOADING));
    }
    break;
  case QueryStatus::READY:
    status_icon_cell = this->themed_icons.ready;
    if (id == selected)
    {
      this->refresh_button->set_sensitive(true);
      this->present_servers(id);
    }
    break;
  }
}

void
Application::on_game_browser_view_selection_changed_cb()
{
  auto id = Glib::ustring((*this->game_browser_view->get_selection()->get_selected())[this->game_list_columns.id]);

  if (id.empty())
  {
    this->refresh_button->set_sensitive(false);
  }
  else
  {
    auto qs = this->core->game_table->get_query_status(id);
    if (qs == QueryStatus::EMPTY)
    {
      this->do_refresh(id);
    }
    else
    {
      this->on_status_changed_cb(id, qs);
    }
  }
}

void
Application::on_refresh_button_clicked_cb()
{
  auto id = (*this->game_browser_view->get_selection()->get_selected())[this->game_list_columns.id];
  if (this->core->game_table->get_query_status(id) != QueryStatus::WORKING)
  {
    this->do_refresh(id);
  }
}

void
Application::show_server_info(Glib::ustring game, Glib::ustring host) {
    show_server_info_dialog(*this->main_window.operator->(),
                            this->core->game_table->get_setting(game, Obozrenie::SettingGroup::SYSTEM, Obozrenie::name_setting).value<std::string>(),
                            host,
                            this->core->game_table->get_server_info_by_host(game, host),
                            [this, game]() { this->log_fn(Glib::ustring::compose("Launching %1\n", game)); });
}

void
Application::do_refresh(GameID id)
{
  this->async_cb([this, id]() {
    try
    {
      this->core->refresh_servers(
        id, false, [this, id](const std::exception& e) { this->core->game_table->set_setting<std::string>(id, Obozrenie::SettingGroup::SYSTEM, error_message_setting, e.what(), true); });
    }
    catch (...)
    {
      return;
    }
    this->core->game_table->set_setting<std::string>(id, Obozrenie::SettingGroup::SYSTEM, error_message_setting, "", true);
  });
}

void
Application::populate_server_list(GameID id)
{
  this->server_list->clear();

  auto data = this->core->game_table->get_servers(id);

  for (auto kv : data)
  {
    auto host = kv.first;
    auto v = kv.second;
    auto row = *this->server_list->append();
    row[this->server_list_columns.game_id] = id;
    row[this->server_list_columns.game_icon] = this->pixbufs.at(pixbuf_gameid(id));
    row[this->server_list_columns.host] = host;
    row[this->server_list_columns.name] = v.name.value_or("(unnamed server)");
    row[this->server_list_columns.need_pass] = v.need_pass.value_or(false);
    if (v.need_pass.value_or(false))
    {
      row[this->server_list_columns.lock_icon] = this->themed_icons.need_pass;
    }
    row[this->server_list_columns.secure] = v.secure.value_or(false);
    if (v.secure.value_or(false))
    {
      row[this->server_list_columns.secure_icon] = this->themed_icons.secure;
    }
    row[this->server_list_columns.player_count] = v.player_count.value_or(0);
    row[this->server_list_columns.player_limit] = v.player_limit.value_or(0);
    row[this->server_list_columns.ping] = v.ping.value_or(9999);
    row[this->server_list_columns.game_type] = v.game_type.value_or("");
    row[this->server_list_columns.game_mod] = v.game_mod.value_or("");
    row[this->server_list_columns.terrain] = v.terrain.value_or("");
  }
}

void
Application::connect_signals()
{
  this->core->game_table->servers_changed.connect([this](GameID, ServerData) {
    Glib::signal_idle().connect([this]() {
      this->server_connect_info_changed();
      return false;
    });
  });

  this->core->game_table->status_changed.connect([this](GameID id, QueryStatus ns, QueryStatus os) {
    Glib::signal_idle().connect([this, id, ns, os]() {
      this->on_status_changed_cb(id, ns);
      return false;
    });
  });

  this->refresh_button->signal_clicked().connect([this]() { this->async_cb(sigc::mem_fun(*this, &Application::on_refresh_button_clicked_cb)); });
  this->game_browser_view->get_selection()->signal_changed().connect([this]() {
    if (this->first_selection)
    {
      this->refresh_button->set_sensitive(false);
      this->game_browser_view->get_selection()->unselect_all();
      this->first_selection = false;
    }
    else
    {
      this->on_game_browser_view_selection_changed_cb();
    }
  });
  this->selection_signal_connection = this->server_browser_view->get_selection()->signal_changed().connect([this]() { this->on_server_browser_view_selection_changed_cb(); });
  this->game_preferences_button->signal_clicked().connect([this]() { this->on_game_preferences_button_clicked_cb(); });

  this->server_connect_info_changed.connect([this]() {

    auto id = this->connect_game;
    auto host = this->connect_host;

    if (!id.empty())
    {
      this->game_preferences_button->set_sensitive(true);
      try
      {
        if (host.empty())
        {
          this->server_connect_button->set_sensitive(false);
          throw NotFoundError();
        }
        this->server_connect_button->set_sensitive(true);

        this->core->game_table->get_server_info_by_host(id, host);

        this->server_info_button->set_sensitive(true);
      }
      catch (const NotFoundError&)
      {
        this->server_info_button->set_sensitive(false);
      }
    }
    else
    {
      this->game_preferences_button->set_sensitive(false);
      this->server_info_button->set_sensitive(false);
      this->server_connect_button->set_sensitive(false);
    }
  });

  this->server_info_button->signal_clicked().connect([this]() { this->show_server_info(this->connect_game, this->connect_host); });

  this->server_connect_game_combobox->signal_changed().connect([this]() {
    this->connect_game = this->server_connect_game_combobox->get_active_id();
    this->server_connect_info_changed();
  });

  this->server_connect_host_entry->signal_changed().connect([this]() {
    this->connect_host = this->server_connect_host_entry->get_text();
    this->server_connect_info_changed();
  });
  this->server_connect_pass_entry->signal_changed().connect([this]() { this->connect_pass = this->server_connect_pass_entry->get_text(); });

  this->filters_button->signal_clicked().connect([this]() { this->filters_revealer->set_reveal_child(this->filters_button->get_active()); });

  this->app->signal_startup().connect([this]() {
    this->app->add_action("about")->signal_activate().connect([this](const auto&) { this->show_about_dialog(); });
    this->app->add_action("quit")->signal_activate().connect([this](const auto&) { this->app->quit(); });

    auto m = Gio::Menu::create();
    m->insert(0, "About", "app.about");
    m->insert(1, "Quit", "app.quit");

    this->app->set_app_menu(m);
  });
}

int
Application::start() {
  return this->app->run(*this->main_window.operator->());
}

Application::Application(std::shared_ptr<ThreadPool> p,
                         std::function<void(std::string)> logger,
                         Glib::RefPtr<Gtk::Application> a,
                         std::shared_ptr<Obozrenie::Core> c,
                         Gtk::Builder& b,
                         std::map<Glib::ustring, Glib::RefPtr<Gdk::Pixbuf>> l) {
  this->pool = p;

  if (!this->pool)
  {
    this->async_cb = [](std::function<void()> fn) { std::thread(fn).detach(); };
  }
  else
  {
    this->async_cb = [this](std::function<void()> fn) { this->pool->enqueue(fn); };
  }
  this->log_fn = logger;
  this->app = a;
  this->core = c;
  this->pixbufs = l;

  this->main_window = get_object<Gtk::ApplicationWindow>(b, "main_window");

  auto gl = Gtk::ListStore::create(this->game_list_columns);

  for (auto kv : this->core->game_table->get_setting_map(Obozrenie::SettingGroup::SYSTEM, Obozrenie::name_setting))
  {
    auto name = kv.second.value<std::string>();
    auto it = gl->append();
    (*it)[this->game_list_columns.id] = kv.first;
    (*it)[this->game_list_columns.game_name] = name;
    try
    {
      (*it)[this->game_list_columns.game_icon] = this->pixbufs.at(pixbuf_gameid(kv.first));
    }
    catch (const std::out_of_range& e)
    {
    }
  }
  this->game_list = gl;
  this->first_selection = true;
  this->server_list = Gtk::ListStore::create(this->server_list_columns);

  this->error_message = &get_widget<Gtk::Label>(b, "error_message");

  this->game_preferences_button = &get_widget<Gtk::Button>(b, "game_preferences_button");
  this->refresh_button = &get_widget<Gtk::Button>(b, "refresh_button");
  this->filters_button = &get_widget<Gtk::ToggleButton>(b, "filters_button");

  this->filters_revealer = &get_widget<Gtk::Revealer>(b, "filters_revealer");

  this->server_info_button = &get_widget<Gtk::Button>(b, "server_info_button");
  this->server_connect_button = &get_widget<Gtk::Button>(b, "server_connect_button");

  this->game_browser_view = &get_widget<Gtk::TreeView>(b, "game_browser_view");
  this->game_browser_view->set_model(this->game_list);
  auto gl_col = Gtk::manage(new Gtk::TreeViewColumn("Available games"));
  gl_col->pack_start(this->game_list_columns.game_icon, false);
  gl_col->pack_start(this->game_list_columns.game_name);

  Gtk::CellRendererPixbuf si;
  gl_col->pack_start(si);
  gl_col->add_attribute(si.property_icon_name(), this->game_list_columns.status_icon);
  this->game_browser_view->append_column(*gl_col);

  auto& sbv = get_widget<Gtk::TreeView>(b, "server_browser_view");
  sbv.set_model(this->server_list);
  this->server_browser_view = &sbv;

  auto name_col = Gtk::manage(new Gtk::TreeViewColumn("Name"));
  name_col->pack_start(this->server_list_columns.game_icon, false);

  Gtk::CellRendererPixbuf lock_icon;
  name_col->pack_start(lock_icon);
  name_col->add_attribute(lock_icon.property_icon_name(), this->server_list_columns.lock_icon);

  Gtk::CellRendererPixbuf secure_icon;
  name_col->pack_start(secure_icon);
  name_col->add_attribute(secure_icon.property_icon_name(), this->server_list_columns.secure_icon);

  name_col->pack_start(this->server_list_columns.name);

  auto host_col = Gtk::manage(new Gtk::TreeViewColumn("Host"));
  host_col->pack_start(this->server_list_columns.country_icon);
  host_col->pack_start(this->server_list_columns.host);

  auto ping_col = Gtk::manage(new Gtk::TreeViewColumn("Ping"));
  ping_col->pack_start(this->server_list_columns.ping);

  auto players_col = Gtk::manage(new Gtk::TreeViewColumn("Players"));
  players_col->pack_start(this->server_list_columns.player_count);
  Gtk::CellRendererText player_sep;
  player_sep.property_text() = "/";
  players_col->pack_start(player_sep);
  players_col->pack_start(this->server_list_columns.player_limit);

  auto mod_col = Gtk::manage(new Gtk::TreeViewColumn("Mod"));
  mod_col->pack_start(this->server_list_columns.game_mod);

  auto terrain_col = Gtk::manage(new Gtk::TreeViewColumn("Terrain"));
  terrain_col->pack_start(this->server_list_columns.terrain);

  this->server_browser_view->append_column(*name_col);
  this->server_browser_view->append_column(*host_col);
  this->server_browser_view->append_column(*ping_col);
  this->server_browser_view->append_column(*players_col);
  this->server_browser_view->append_column(*mod_col);
  this->server_browser_view->append_column(*terrain_col);

  this->server_connect_game_combobox = &get_widget<Gtk::ComboBox>(b, "server_connect_game_combobox");
  this->server_connect_game_combobox->set_model(this->game_list);
  this->server_connect_game_combobox->set_id_column(this->game_list_columns.id.index());
  this->server_connect_game_combobox->pack_start(this->game_list_columns.game_icon, false);
  this->server_connect_game_combobox->pack_start(this->game_list_columns.game_name);

  this->server_connect_host_entry = &get_widget<Gtk::Entry>(b, "server_connect_host_entry");
  this->server_connect_pass_entry = &get_widget<Gtk::Entry>(b, "server_connect_pass_entry");

  this->themed_icons = ThemedIcons{.working = "emblem-synchronizing-symbolic",
    .error = "dialog-error-symbolic",
    .ready = "emblem-ok-symbolic",
    .need_pass = "network-wireless-encrypted-symbolic",
    .secure = "security-high-symbolic",
    .unknown = "dialog-question-symbolic" };

  this->server_browser_pager = &get_widget<Gtk::Notebook>(b, "server_browser_pager");
  this->server_browser_pager->set_current_page(this->server_browser_pager->page_num(get_widget<Gtk::Label>(b, "welcome_label")));

  this->connect_signals();
}

Application::~Application() {}
}
}
