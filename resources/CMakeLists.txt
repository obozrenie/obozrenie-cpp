project(obresources)

set (GRESOURCE_CONFIG io.obozrenie.gresource.xml)
set (GRESOURCE_DATA data.c)

add_custom_command(OUTPUT ${GRESOURCE_DATA} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMMAND "glib-compile-resources" "--generate-source" "--target=${CMAKE_BINARY_DIR}/${GRESOURCE_DATA}" "${GRESOURCE_CONFIG}")

add_custom_target(gresources DEPENDS ${GRESOURCE_DATA})
