option(BUILD_DESIGNER_PLUGIN "Build Qt4 designer plugin" ON)
option(USE_QT5 	             "Build using Qt5. Default OFF." OFF)
option(BUILD_TEST            "Build test application. Default OFF." OFF)

cmake_minimum_required( VERSION 2.8 )
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake )

project( qtermwidget )

# Lets start to follow Qt library versioning. 4.x for Qt4, 5.x for Qt5
if (USE_QT5)
set(QTERMWIDGET_LIBRARY_NAME qtermwidget5)
else (USE_QT5)
set(QTERMWIDGET_LIBRARY_NAME qtermwidget4)
endif (USE_QT5)
# just change version for releases
set ( QTERMWIDGET_VERSION_MAJOR "0" )
set ( QTERMWIDGET_VERSION_MINOR "5" )
set ( QTERMWIDGET_VERSION_PATCH "1" )

set ( QTERMWIDGET_VERSION "${QTERMWIDGET_VERSION_MAJOR}.${QTERMWIDGET_VERSION_MINOR}.${QTERMWIDGET_VERSION_PATCH}" )


include(CheckFunctionExists)
include(GNUInstallDirs)

include_directories( ${CMAKE_SOURCE_DIR}/lib
                     ${CMAKE_BINARY_DIR}/lib
                     ${CMAKE_BINARY_DIR}
                   )
add_definitions( -Wall )


if (USE_QT5)
    include(qtermwidget5_use)
else (USE_QT5)
    include(qtermwidget4_use)
endif (USE_QT5)


# main library

set(SRCS
    lib/BlockArray.cpp
    lib/ColorScheme.cpp
    lib/Emulation.cpp
    lib/Filter.cpp
    lib/History.cpp
    lib/HistorySearch.cpp
    lib/KeyboardTranslator.cpp
    lib/konsole_wcwidth.cpp
    lib/kprocess.cpp
    lib/kpty.cpp
    lib/kptydevice.cpp
    lib/kptyprocess.cpp
    lib/Pty.cpp
    lib/qtermwidget.cpp
    lib/Screen.cpp
    lib/ScreenWindow.cpp
    lib/SearchBar.cpp
    lib/Session.cpp
    lib/ShellCommand.cpp
    lib/TerminalCharacterDecoder.cpp
    lib/TerminalDisplay.cpp
    lib/tools.cpp
    lib/Vt102Emulation.cpp
)

# Only the Headers that need to be moc'd go here
set(HDRS
    lib/Emulation.h
    lib/Filter.h
    lib/HistorySearch.h
    lib/kprocess.h
    lib/kptydevice.h
    lib/kptyprocess.h
    lib/Pty.h
    lib/qtermwidget.h
    lib/ScreenWindow.h
    lib/SearchBar.h
    lib/Session.h
    lib/TerminalDisplay.h
    lib/Vt102Emulation.h
)

set(UI lib/SearchBar.ui)

# for distribution
set(HDRS_DISTRIB lib/qtermwidget.h lib/Filter.h)

# dirs
set ( KB_LAYOUT_DIR ${CMAKE_INSTALL_DATADIR}/${QTERMWIDGET_LIBRARY_NAME}/kb-layouts/ )
message ( STATUS "Keyboard layouts will be installed in: ${KB_LAYOUT_DIR}" )
add_definitions( -DKB_LAYOUT_DIR="${CMAKE_INSTALL_PREFIX}/${KB_LAYOUT_DIR}" )

set ( COLORSCHEMES_DIR ${CMAKE_INSTALL_DATADIR}/${QTERMWIDGET_LIBRARY_NAME}/color-schemes/ )
message ( STATUS "Color schemes will be installed in: ${COLORSCHEMES_DIR}" )
add_definitions( -DCOLORSCHEMES_DIR="${CMAKE_INSTALL_PREFIX}/${COLORSCHEMES_DIR}" )

set(QTERMWIDGET_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/${QTERMWIDGET_LIBRARY_NAME})

#| Defines
add_definitions ( -DHAVE_POSIX_OPENPT -DHAVE_SYS_TIME_H )
if (APPLE)
    add_definitions( -DHAVE_UTMPX -D_UTMPX_COMPAT )
endif (APPLE)
CHECK_FUNCTION_EXISTS(updwtmpx HAVE_UPDWTMPX)
if (HAVE_UPDWTMPX)
    add_definitions( -DHAVE_UPDWTMPX )
endif (HAVE_UPDWTMPX)


if (USE_QT5)
    qt5_wrap_cpp(MOCS ${HDRS})
    qt5_wrap_ui(UI_SRCS ${UI})
    set(PKG_CONFIG_REQ "Qt5Core, Qt5Xml, Qt5Widgets")
else (USE_QT5)
    qt4_wrap_cpp(MOCS ${HDRS})
    qt4_wrap_ui(UI_SRCS ${UI})
    set(PKG_CONFIG_REQ "QtCore, QtXml")
endif (USE_QT5)


add_library(${QTERMWIDGET_LIBRARY_NAME} SHARED ${SRCS} ${MOCS} ${UI_SRCS})
target_link_libraries(${QTERMWIDGET_LIBRARY_NAME} ${QTERMWIDGET_QT_LIBRARIES})
set_target_properties( ${QTERMWIDGET_LIBRARY_NAME} PROPERTIES
                       SOVERSION ${QTERMWIDGET_VERSION_MAJOR}
                       VERSION ${QTERMWIDGET_VERSION}
                     )
if (APPLE)
    # this is a must to load the lib correctly
    set_target_properties( ${QTERMWIDGET_LIBRARY_NAME} PROPERTIES INSTALL_NAME_DIR ${CMAKE_INSTALL_LIBDIR} )
endif (APPLE)

install(TARGETS ${QTERMWIDGET_LIBRARY_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${HDRS_DISTRIB} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${QTERMWIDGET_LIBRARY_NAME})
# keyboard layouts
install(DIRECTORY lib/kb-layouts/ DESTINATION ${KB_LAYOUT_DIR} FILES_MATCHING PATTERN "*.keytab" )
# color schemes
install(DIRECTORY lib/color-schemes/ DESTINATION ${COLORSCHEMES_DIR} FILES_MATCHING PATTERN "*.*schem*")

include(create_pkgconfig_file)
create_pkgconfig_file(${QTERMWIDGET_LIBRARY_NAME}
    "QTermWidget library for Qt ${QTERMWIDGET_VERSION_MAJOR}.x"
    ${PKG_CONFIG_REQ}
    ${QTERMWIDGET_LIBRARY_NAME}
    ${QTERMWIDGET_VERSION}
)

configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/${QTERMWIDGET_LIBRARY_NAME}-config.cmake.in
    ${CMAKE_BINARY_DIR}/${QTERMWIDGET_LIBRARY_NAME}-config.cmake
    @ONLY
)
install(FILES ${CMAKE_BINARY_DIR}/${QTERMWIDGET_LIBRARY_NAME}-config.cmake DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/${QTERMWIDGET_LIBRARY_NAME})
install(FILES ${CMAKE_SOURCE_DIR}/cmake/${QTERMWIDGET_LIBRARY_NAME}_use.cmake DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/${QTERMWIDGET_LIBRARY_NAME})
# end of main library


# designer plugin
if (BUILD_DESIGNER_PLUGIN)
    if (USE_QT5)
        message(FATAL_ERROR "Building Qt designer plugin is not supported for Qt5 yet. Use -DBUILD_DESIGNER_PLUGIN=0")
    endif (USE_QT5)
    message(STATUS "Building Qt designer plugin")

    include_directories(designer
                        ${QT_QTDESIGNER_INCLUDE_DIR})

    set(DESIGNER_SRC lib/designer/qtermwidgetplugin.cpp)
    QT4_WRAP_CPP( DESIGNER_MOC  lib/designer/qtermwidgetplugin.h)
    QT4_ADD_RESOURCES( DESIGNER_QRC lib/designer/qtermwidgetplugin.qrc)

    link_directories(${CMAKE_BINARY_DIR})
    add_library( qtermwidget4plugin SHARED
        ${DESIGNER_MOC}
        ${DESIGNER_QRC}
        ${DESIGNER_SRC}
    )
    add_dependencies(qtermwidget4plugin qtermwidget4)

    target_link_libraries( qtermwidget4plugin
        ${QT_QTCORE_LIBRARY}
        ${QT_QTDESIGNER_LIBRARY}
        ${QT_QTDESIGNERCOMPONENTS_LIBRARY}
        ${QTERMWIDGET_LIBRARY_NAME}
    )

    if (APPLE)
        # this is a must to load the lib correctly
        set_target_properties( qtermwidge4tplugin PROPERTIES
                                INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/qt4/plugins/designer )
    endif (APPLE)

    install( TARGETS qtermwidget4plugin DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/qt4/plugins/designer )

endif (BUILD_DESIGNER_PLUGIN)


# end of designer plugin


# test application
if (BUILD_TEST)
    set(TEST_SRC src/main.cpp)
    add_executable(test ${TEST_SRC})
    add_dependencies(test ${QTERMWIDGET_LIBRARY_NAME})
    link_directories(${CMAKE_BINARY_DIR})
    target_link_libraries(test ${QTERMWIDGET_QT_LIBRARIES} ${QTERMWIDGET_LIBRARY_NAME} util)
endif (BUILD_TEST)
# end of test application


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
SET(CPACK_PACKAGE_VERSION ${QTERMWIDGET_VERSION_MAJOR}.${QTERMWIDGET_VERSION_MINOR}.${QTERMWIDGET_VERSION_PATCH})
SET(CPACK_SOURCE_GENERATOR "TGZ;TBZ2")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
SET(CPACK_IGNORE_FILES "/CVS/;/\\\\.svn/;/\\\\.git/;\\\\.swp$;\\\\.#;/#;\\\\.tar.gz$;/CMakeFiles/;CMakeCache.txt;\\\\.qm$;/build/;\\\\.diff$;.DS_Store'")
SET(CPACK_SOURCE_IGNORE_FILES ${CPACK_IGNORE_FILES})
INCLUDE(CPack)
# simulate autotools' "make dist"
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

