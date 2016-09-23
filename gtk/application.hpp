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

#ifndef _APPLICATION_HPP_
#define _APPLICATION_HPP_

#include <functional>

#include <gtkmm.h>

#include <libobozrenie/libobozrenie.hpp>

#include "helpers.hpp"
#include "models.hpp"
#include "widgets.hpp"

namespace Obozrenie
{
namespace GTK
{

enum class GameBrowserPages
{
  SERVERS,
  WELCOME,
  LOADING,
  ERROR
};

struct ThemedIcons
{
  Glib::ustring working;
  Glib::ustring error;
  Glib::ustring ready;
  Glib::ustring need_pass;
  Glib::ustring secure;
  Glib::ustring unknown;
};

struct Application : sigc::trackable
{
private:
  std::function<void(std::string)> log_fn;

  bool first_selection;
  GameID connect_game;
  Glib::ustring connect_host;
  Glib::ustring connect_pass;

  sigc::signal<void> server_connect_info_changed;
  sigc::connection selection_signal_connection;

  std::shared_ptr<ThreadPool> pool;
  Glib::RefPtr<Gtk::Application> app;
  std::shared_ptr<Obozrenie::Core> core;
  std::map<Glib::ustring, Glib::RefPtr<Gdk::Pixbuf>> pixbufs;
  ThemedIcons themed_icons;

  GameListModelColumns game_list_columns;
  ServerListModelColumns server_list_columns;

  PlayerListModelColumns player_list_columns;
  RuleListModelColumns rule_list_columns;

  Glib::RefPtr<Gtk::ListStore> game_list;
  Glib::RefPtr<Gtk::ListStore> server_list;

  Glib::RefPtr<Gtk::ListStore> player_list;
  Glib::RefPtr<Gtk::ListStore> rule_list;

  Glib::RefPtr<Gtk::ApplicationWindow> main_window;

  Gtk::TreeView* game_browser_view;
  Gtk::TreeView* server_browser_view;
  Gtk::Notebook* server_browser_pager;
  Gtk::Label* error_message;

  Gtk::ComboBox* server_connect_game_combobox;
  Gtk::Entry* server_connect_host_entry;
  Gtk::Entry* server_connect_pass_entry;

  Gtk::Button* game_preferences_button;
  Gtk::Button* refresh_button;
  Gtk::ToggleButton* filters_button;

  Gtk::Revealer* filters_revealer;

  Gtk::Button* server_info_button;
  Gtk::Button* server_connect_button;

  std::function<void(std::function<void()>)> async_cb;

  void show_about_dialog();

  void on_status_changed_cb(GameID, Obozrenie::QueryStatus);
  void on_refresh_button_clicked_cb();
  void on_game_browser_view_selection_changed_cb();
  void on_server_browser_view_selection_changed_cb();
  void on_game_preferences_button_clicked_cb();
  void show_server_info(Glib::ustring, Glib::ustring);

  void do_refresh(GameID);
  void populate_server_list(GameID);
  void present_servers(GameID);

  void connect_signals();

public:
  int start();

  Application(std::shared_ptr<ThreadPool>, std::function<void(std::string)>, Glib::RefPtr<Gtk::Application>, std::shared_ptr<Obozrenie::Core>, Gtk::Builder&, std::map<Glib::ustring, Glib::RefPtr<Gdk::Pixbuf>>);
  virtual ~Application();
};
}
}

#endif
