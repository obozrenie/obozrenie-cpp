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

#ifndef _MODELS_HPP_
#define _MODELS_HPP_

#include <gtkmm.h>

#include <libobozrenie/libobozrenie.hpp>

namespace Obozrenie
{
namespace GTK
{
class ServerListModelColumns : public Gtk::TreeModelColumnRecord
{
public:
  Gtk::TreeModelColumn<Glib::ustring> host;
  Gtk::TreeModelColumn<bool> need_pass;
  Gtk::TreeModelColumn<int> player_count;
  Gtk::TreeModelColumn<int> player_limit;
  Gtk::TreeModelColumn<int> ping;
  Gtk::TreeModelColumn<bool> secure;
  Gtk::TreeModelColumn<Glib::ustring> country;
  Gtk::TreeModelColumn<Glib::ustring> name;
  Gtk::TreeModelColumn<Glib::ustring> game_id;
  Gtk::TreeModelColumn<Glib::ustring> game_mod;
  Gtk::TreeModelColumn<Glib::ustring> game_type;
  Gtk::TreeModelColumn<Glib::ustring> terrain;
  Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> game_icon;
  Gtk::TreeModelColumn<Glib::ustring> lock_icon;
  Gtk::TreeModelColumn<Glib::ustring> secure_icon;
  Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> country_icon;
  Gtk::TreeModelColumn<bool> full;
  Gtk::TreeModelColumn<bool> empty;

  ServerListModelColumns()
  {
    add(host);
    add(need_pass);
    add(player_count);
    add(player_limit);
    add(ping);
    add(secure);
    add(country);
    add(name);
    add(game_id);
    add(game_mod);
    add(game_type);
    add(terrain);
    add(game_icon);
    add(lock_icon);
    add(secure_icon);
    add(country_icon);
    add(full);
    add(empty);
  }
};

class GameListModelColumns : public Gtk::TreeModelColumnRecord
{
public:
  Gtk::TreeModelColumn<Glib::ustring> id;
  Gtk::TreeModelColumn<Glib::ustring> game_name;
  Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> game_icon;
  Gtk::TreeModelColumn<Glib::ustring> status_icon;

  GameListModelColumns()
  {
    add(id);
    add(game_name);
    add(game_icon);
    add(status_icon);
  }
};

class PlayerListModelColumns : public Gtk::TreeModelColumnRecord {
public:
    Gtk::TreeModelColumn<Glib::ustring> name;
    Gtk::TreeModelColumn<int> score;
    Gtk::TreeModelColumn<int> ping;

    PlayerListModelColumns() {
        this->add(name);
        this->add(score);
        this->add(ping);
    }
};

class RuleListModelColumns : public Gtk::TreeModelColumnRecord {
public:
    Gtk::TreeModelColumn<Glib::ustring> key;
    Gtk::TreeModelColumn<Glib::ustring> value;

    RuleListModelColumns() {
        this->add(key);
        this->add(value);
    }
};

void append_server_entry(Glib::RefPtr<Gtk::ListStore>, ServerListModelColumns, GameID, Server, std::map<Glib::ustring, Glib::RefPtr<Gdk::Pixbuf>>);

template <typename T>
Gtk::TreeIter
search_model(Glib::RefPtr<Gtk::ListStore> model, Gtk::TreeModelColumn<T> column, T v)
{
  for (auto iter : model->children())
  {
    if ((*iter)[column] == v)
    {
      return iter;
    }
  }
  throw Obozrenie::NotFoundError("model search failed");
}
}
}
#endif
