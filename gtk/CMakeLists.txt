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

project (obgtk)


set (${PROJECT_NAME}_SOURCES

    main.cpp
    helpers.cpp
    widgets.cpp
    application.cpp
)

set (${PROJECT_NAME}_HEADERS
    helpers.hpp
    models.hpp
    widgets.hpp
    application.hpp
)

include_directories (${GIOMM_INCLUDE_DIRS})
include_directories (${GTKMM_INCLUDE_DIRS})
include_directories (${GDKMM_INCLUDE_DIRS})
include_directories (${GIOMM_INCLUDE_DIRS})
include_directories (${GEOIP_INCLUDE_DIRS})
include_directories (${LIBXMLMM_INCLUDE_DIRS})
include_directories( ${CMAKE_SOURCE_DIR} )

link_directories (${GTKMM_LIBRARY_DIRS})

add_executable (obozrenie_gtk ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS})
set_property(TARGET obozrenie_gtk PROPERTY CXX_STANDARD 14)
set_property(TARGET obozrenie_gtk PROPERTY CXX_STANDARD_REQUIRED ON)

add_dependencies(obozrenie_gtk gresources)
target_link_libraries (obozrenie_gtk ${LIBNAME} ${GTKMM_LIBRARIES} ${LIBXMLMM_LIBRARIES} ${GEOIP_LIBRARIES} ${JSONCPP_LIBRARIES} stdc++fs)
