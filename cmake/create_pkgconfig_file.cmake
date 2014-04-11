#
# Write a pkg-config pc file for given "name" with "decription"
# Arguments:
#   name: a library name (withoud "lib" prefix and "so" suffixes
#   desc: a desription string
#   requires: required libraries
#   include_rel_dir: include directory, relative to includedir
#   version: package version
#
macro (create_pkgconfig_file name desc requires include_rel_dir version)
    set(_pkgfname "${CMAKE_CURRENT_BINARY_DIR}/${name}.pc")
    message(STATUS "${name}: writing pkgconfig file ${_pkgfname}")

    file(WRITE "${_pkgfname}"
            "prefix=${CMAKE_INSTALL_PREFIX}\n"
            "libdir=\${prefix}/${CMAKE_INSTALL_LIBDIR}\n"
            "includedir=\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}\n"
            "\n"
            "Name: ${name}\n"
            "Description: ${desc}\n"
            "Version: ${version}\n"
            "Requires: ${requires}\n"
            "Libs: -L\${libdir} -l${name}\n"
            "Cflags: -I\${includedir} -I\${includedir}/${include_rel_dir}\n"
            "\n"
        )

    install(FILES ${_pkgfname} DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endmacro()
