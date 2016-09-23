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

#include "helpers.hpp"
#include "widgets.hpp"

namespace Obozrenie
{
namespace GTK
{
void show_server_info_dialog(Gtk::Window& w, Glib::ustring game_v, Glib::ustring host_v, Server data, std::function<void()> cb_launch) {
    PlayerListModelColumns player_list_columns;
    RuleListModelColumns rule_list_columns;

    // Set models
    auto game = Gtk::manage(new Gtk::Label(game_v)); game->set_halign(Gtk::Align::ALIGN_START);
    auto host = Gtk::manage(new Gtk::Label(host_v)); host->set_halign(Gtk::Align::ALIGN_START);
    auto name = Gtk::manage(new Gtk::Label); name->set_halign(Gtk::Align::ALIGN_START);
    auto terrain = Gtk::manage(new Gtk::Label); terrain->set_halign(Gtk::Align::ALIGN_START);
    auto ping = Gtk::manage(new Gtk::Label); ping->set_halign(Gtk::Align::ALIGN_START);
    auto players = Gtk::manage(new Gtk::Label); players->set_halign(Gtk::Align::ALIGN_START);

    set_label_from_optional(name, data.name);
    set_label_from_optional(terrain, data.terrain);
    set_label_from_optional(ping, data.ping);

    try {
        players->set_text(Glib::ustring::compose("%1 / %2", data.player_count.value(), data.player_limit.value()));
    } catch (const std::experimental::bad_optional_access&) {}

    auto player_list = Gtk::ListStore::create(player_list_columns);
    for (auto v : data.players) {
        auto& row = *player_list->append();
        row[player_list_columns.name] = v.name;
        try { row[player_list_columns.ping] = std::stoi(v.info.at("ping")); } catch (...) {}
        try { row[player_list_columns.score] = std::stoi(v.info.at("score")); } catch (...) {}
    }
    
    auto rule_list = Gtk::ListStore::create(rule_list_columns);
    for (auto v : data.rules) {
        auto& row = *rule_list->append();
        row[rule_list_columns.key] = v.first;
        row[rule_list_columns.value] = v.second;
    }

    // Create presentation
    auto server_info_data_grid = Gtk::manage(new Gtk::Grid);
    server_info_data_grid->set_row_spacing(5);
    server_info_data_grid->set_column_spacing(5);

{
    auto i = -1;

    auto name_label = Gtk::manage(new Gtk::Label); name_label->set_markup("<b>Name:</b>"); name_label->set_halign(Gtk::Align::ALIGN_END);
    server_info_data_grid->attach(*name_label, 0, i++, 1, 1);
    server_info_data_grid->attach_next_to(*name, *name_label, Gtk::PositionType::POS_RIGHT, 1, 1);

    auto host_label = Gtk::manage(new Gtk::Label); host_label->set_markup("<b>Host:</b>"); host_label->set_halign(Gtk::Align::ALIGN_END);
    server_info_data_grid->attach(*host_label, 0, i++, 1, 1);
    server_info_data_grid->attach_next_to(*host, *host_label, Gtk::PositionType::POS_RIGHT, 1, 1);

    auto game_label = Gtk::manage(new Gtk::Label); game_label->set_markup("<b>Game:</b>"); game_label->set_halign(Gtk::Align::ALIGN_END);
    server_info_data_grid->attach(*game_label, 0, i++, 1, 1);
    server_info_data_grid->attach_next_to(*game, *game_label, Gtk::PositionType::POS_RIGHT, 1, 1);

    auto terrain_label = Gtk::manage(new Gtk::Label); terrain_label->set_markup("<b>Terrain:</b>"); terrain_label->set_halign(Gtk::Align::ALIGN_END);
    server_info_data_grid->attach(*terrain_label, 0, i++, 1, 1);
    server_info_data_grid->attach_next_to(*terrain, *terrain_label, Gtk::PositionType::POS_RIGHT, 1, 1);

    auto players_label = Gtk::manage(new Gtk::Label); players_label->set_markup("<b>Players:</b>"); players_label->set_halign(Gtk::Align::ALIGN_END);
    server_info_data_grid->attach(*players_label, 0, i++, 1, 1);
    server_info_data_grid->attach_next_to(*players, *players_label, Gtk::PositionType::POS_RIGHT, 1, 1);

    auto ping_label = Gtk::manage(new Gtk::Label); ping_label->set_markup("<b>Ping:</b>"); ping_label->set_halign(Gtk::Align::ALIGN_END);
    server_info_data_grid->attach(*ping_label, 0, i++, 1, 1);
    server_info_data_grid->attach_next_to(*ping, *ping_label, Gtk::PositionType::POS_RIGHT, 1, 1);
}

    auto player_list_view = Gtk::manage(new Gtk::TreeView(player_list));
{
    auto col = Gtk::manage(new Gtk::TreeViewColumn("Name"));
    col->pack_start(player_list_columns.name);
    player_list_view->append_column(*col);
}
{
    auto col = Gtk::manage(new Gtk::TreeViewColumn("Score"));
    col->pack_start(player_list_columns.score);
    player_list_view->append_column(*col);
}
{
    auto col = Gtk::manage(new Gtk::TreeViewColumn("Ping"));
    col->pack_start(player_list_columns.ping);
    player_list_view->append_column(*col);
}
    auto player_list_sw = Gtk::manage(new Gtk::ScrolledWindow); player_list_sw->set_hexpand(true); player_list_sw->set_vexpand(true);
    player_list_sw->add(*player_list_view);

    auto server_info_grid = Gtk::manage(new Gtk::Grid); server_info_grid->set_row_spacing(10);
{
    auto i = -1;
    server_info_grid->attach(*server_info_data_grid, 0, i++, 1, 1);
    server_info_grid->attach(*player_list_sw, 0, i++, 1, 1);
}

    // Server rules
    auto rule_list_view = Gtk::manage(new Gtk::TreeView(rule_list));

{
    auto col = Gtk::manage(new Gtk::TreeViewColumn("Key"));
    col->pack_start(rule_list_columns.key);
    rule_list_view->append_column(*col);
}
{
    auto col = Gtk::manage(new Gtk::TreeViewColumn("Value"));
    col->pack_start(rule_list_columns.value);
    rule_list_view->append_column(*col);
}

    auto server_rules_sw = Gtk::manage(new Gtk::ScrolledWindow);
    server_rules_sw->add(*rule_list_view);

    // The stack
    auto stack = Gtk::manage(new Gtk::Stack); stack->property_margin().set_value(10);
    stack->add(*server_info_grid, "info", "Information");
    stack->add(*server_rules_sw, "rules", "Rules");

    auto stack_switcher = Gtk::manage(new Gtk::StackSwitcher); stack_switcher->set_stack(*stack); stack_switcher->set_halign(Gtk::Align::ALIGN_CENTER); 

    // The dialog
    Gtk::Dialog dialog_window("Server info", w);

    auto content = dialog_window.get_content_area();
    content->pack_start(*stack_switcher, Gtk::PackOptions::PACK_SHRINK);
    content->pack_start(*stack);

    auto connect_button = dialog_window.add_button("Connect", 0);
    auto close_button = dialog_window.add_button("Close", 0);
    connect_button->signal_clicked().connect([&dialog_window, cb_launch]() { try { cb_launch(); } catch (const std::bad_function_call&) {} });

    dialog_window.show_all();
    dialog_window.run();
}
}
}
