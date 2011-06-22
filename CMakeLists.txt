###| Cmake qtermwidget |###

option(BUILD_DESIGNER_PLUGIN "Build Qt4 designer plugin" ON)

#| Setting required version
cmake_minimum_required( VERSION 2.4 )

#| Project
project( qtermwidget )

#| Library Versioning
#|	Libraries are not versioned by default
set ( qtermwidget_VERSION_MAJOR "0" )
set ( qtermwidget_VERSION_MINOR "4" )
set ( qtermwidget_VERSION_PATCH "0" )
set ( qtermwidget_VERSION 
	"${qtermwidget_VERSION_MAJOR}.${qtermwidget_VERSION_MINOR}.${qtermwidget_VERSION_PATCH}"
)

subdirs( 
	lib
	src
)

# a helper for macosx to build 32/64 bit universals
IF (WANT_UNIVERSAL)
    SET (CMAKE_OSX_ARCHITECTURES "i386;x86_64;")
    MESSAGE(STATUS "Archs builds: ${CMAKE_OSX_ARCHITECTURES}")
ENDIF (WANT_UNIVERSAL)


CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY
)
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")


# make dist custom target
SET(CPACK_PACKAGE_NAME "qtermwidget")
# TODO/FIXME: versioning from player subdir... I don't know why it's separated...
SET(CPACK_PACKAGE_VERSION ${qtermwidget_VERSION})
SET(CPACK_SOURCE_GENERATOR "TGZ;TBZ2")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
SET(CPACK_IGNORE_FILES "/CVS/;/\\\\.svn/;/\\\\.git/;\\\\.swp$;\\\\.#;/#;\\\\.tar.gz$;/CMakeFiles/;CMakeCache.txt;\\\\.qm$;/build/;\\\\.diff$;.DS_Store'")
SET(CPACK_SOURCE_IGNORE_FILES ${CPACK_IGNORE_FILES})
INCLUDE(CPack)
# simulate autotools' "make dist"
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

