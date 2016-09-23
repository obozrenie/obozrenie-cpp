// This file is part of Obozrenie.

// https://github.com/skybon/obozrenie-cpp
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

#ifndef _HELPERS_HPP_
#define _HELPERS_HPP_

#include <gtkmm.h>

#include <libobozrenie/libobozrenie.hpp>

#include "gtk_exception_list.hpp"

namespace Obozrenie
{
namespace GTK
{
Glib::ustring pixbuf_gameid(GameID id);

template <typename T>
T& get_widget(Gtk::Builder& b, Glib::ustring k) {
  T* v;

  b.get_widget(k, v);

  if (v) { throw NoSuchWidgetError(k); }

  return *v;
}

template <typename T>
Glib::RefPtr<T> get_object(Gtk::Builder& b, Glib::ustring k) {
  auto temp = b.get_object(k);

  if (!temp) { throw NoSuchWidgetError(k); }

  return Glib::RefPtr<T>::cast_dynamic(temp);
}

template <typename T>
void set_label_from_optional(Gtk::Label* label, std::experimental::optional<T> data) {
    if (label) { try { label->set_text(Glib::ustring::format(data.value())); } catch (const std::experimental::bad_optional_access&) {} }
}
}
}
#endif
