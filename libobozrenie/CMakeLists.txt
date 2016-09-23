# This file is part of Obozrenie.

# https://github.com/skybon/obozrenie-cpp
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

project(libob)

set(
    ${LIBNAME}_PUBLIC_HEADERS

    libobozrenie.hpp
    geoip.hpp
    core.hpp
    exceptions.hpp
    backend_minetest.hpp
    backend_qstat.hpp
    util.hpp
    xmlpp_util.hpp
    ThreadPool.hpp
)

set(
    ${LIBNAME}_SOURCES

    geoip.cpp
    core.cpp
    backend_qstat.cpp
    util.cpp
)

include_directories (
    ${GIOMM_INCLUDE_DIRS}
    ${GLIBMM_INCLUDE_DIRS}
    ${LIBXMLMM_INCLUDE_DIRS}
)

add_library(
    ${LIBNAME}

    ${${LIBNAME}_PUBLIC_HEADERS}
    ${${LIBNAME}_SOURCES}
)
set_property(TARGET ${LIBNAME} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${LIBNAME} PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(${LIBNAME} pthread stdc++fs)
