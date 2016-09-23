# This file is part of Obozrenie.

# https://github.com/skybon/obozrenie
# Copyright (C) 2016 Artem Vorotnikov
#
# Obozrenie is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License
# as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# Obozrenie is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Obozrenie.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.5)

project("obozrenie")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )

set(PROJNAME ${PROJECT_NAME})

set(LIBNAME libobozrenie)

set(PROJECT "Obozrenie")
set(VER "0.1")
set(APPLICATION_ID "io.obozrenie")

find_package (Boost REQUIRED)
find_package (PkgConfig REQUIRED)

pkg_check_modules (GIOMM REQUIRED giomm-2.4)
pkg_check_modules (GLIBMM REQUIRED glibmm-2.4)
pkg_check_modules (LIBXMLMM REQUIRED libxml++-2.6)
pkg_check_modules (GTKMM REQUIRED gtkmm-3.0)
pkg_check_modules (GEOIP REQUIRED geoip)
pkg_check_modules (JSONCPP REQUIRED jsoncpp)

configure_file( config.hpp.in ${CMAKE_BINARY_DIR}/config.hpp )
include_directories(${CMAKE_BINARY_DIR})

add_subdirectory(resources)

add_subdirectory(${LIBNAME})

add_subdirectory(gtk)
