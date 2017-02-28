cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

project(qtermwidget)

include(GNUInstallDirs)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
include(CheckFunctionExists)

set(LXQTBT_MINIMUM_VERSION "0.3.0")

option(BUILD_TEST "Build test application. Default OFF." OFF)
# just change version for releases
set(QTERMWIDGET_VERSION_MAJOR "0")
set(QTERMWIDGET_VERSION_MINOR "7")
set(QTERMWIDGET_VERSION_PATCH "1")

set(QTERMWIDGET_VERSION "${QTERMWIDGET_VERSION_MAJOR}.${QTERMWIDGET_VERSION_MINOR}.${QTERMWIDGET_VERSION_PATCH}")

# additional cmake files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Qt5LinguistTools REQUIRED)
find_package(lxqt-build-tools ${LXQTBT_MINIMUM_VERSION} REQUIRED)
include(LXQtTranslateTs)
include(LXQtCompilerSettings NO_POLICY_SCOPE)

lxqt_translate_ts(QTERMWIDGET_QM
    TRANSLATION_DIR "lib/translations"
    PULL_TRANSLATIONS
        ${PULL_TRANSLATIONS}
    CLEAN_TRANSLATIONS
        ${CLEAN_TRANSLATIONS}
    TRANSLATIONS_REPO
        ${TRANSLATIONS_REPO}
    TRANSLATIONS_REFSPEC
        ${TRANSLATIONS_REFSPEC}
)

include_directories(
    "${CMAKE_SOURCE_DIR}/lib"
    "${CMAKE_BINARY_DIR}/lib"
    "${CMAKE_BINARY_DIR}"
)

set(QTERMWIDGET_LIBRARY_NAME qtermwidget5)
include(qtermwidget5_use)

option(USE_UTF8PROC "Use libutf8proc for better Unicode support. Default OFF" OFF)

if(USE_UTF8PROC)
    find_package(Utf8Proc)
endif()

if (UTF8PROC_FOUND)
    add_definitions(-DHAVE_UTF8PROC)
    include_directories("${UTF8PROC_INCLUDE_DIRS}")
endif()

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

set(UI
    lib/SearchBar.ui
)

# for distribution
set(HDRS_DISTRIB
    lib/qtermwidget.h
    lib/Filter.h
)

# dirs
set(KB_LAYOUT_DIR "${CMAKE_INSTALL_FULL_DATADIR}/${QTERMWIDGET_LIBRARY_NAME}/kb-layouts")
message(STATUS "Keyboard layouts will be installed in: ${KB_LAYOUT_DIR}")

set(COLORSCHEMES_DIR "${CMAKE_INSTALL_FULL_DATADIR}/${QTERMWIDGET_LIBRARY_NAME}/color-schemes")
message(STATUS "Color schemes will be installed in: ${COLORSCHEMES_DIR}" )

set(TRANSLATIONS_DIR "${CMAKE_INSTALL_FULL_DATADIR}/${QTERMWIDGET_LIBRARY_NAME}/translations")
message(STATUS "Translations will be installed in: ${TRANSLATIONS_DIR}")

set(QTERMWIDGET_INCLUDE_DIR "${CMAKE_INSTALL_FULL_INCLUDEDIR}/${QTERMWIDGET_LIBRARY_NAME}")

#| Defines
if(APPLE)
    add_definitions(-DHAVE_UTMPX -D_UTMPX_COMPAT)
endif()
CHECK_FUNCTION_EXISTS(updwtmpx HAVE_UPDWTMPX)
if(HAVE_UPDWTMPX)
    add_definitions(-DHAVE_UPDWTMPX)
endif()

qt5_wrap_cpp(MOCS ${HDRS})
qt5_wrap_ui(UI_SRCS ${UI})
set(PKG_CONFIG_REQ "Qt5Core, Qt5Xml, Qt5Widgets")

add_library(${QTERMWIDGET_LIBRARY_NAME} SHARED ${SRCS} ${MOCS} ${UI_SRCS} ${QTERMWIDGET_QM})
target_link_libraries(${QTERMWIDGET_LIBRARY_NAME} ${QTERMWIDGET_QT_LIBRARIES})
set_target_properties( ${QTERMWIDGET_LIBRARY_NAME} PROPERTIES
                       SOVERSION ${QTERMWIDGET_VERSION_MAJOR}
                       VERSION ${QTERMWIDGET_VERSION}
                     )
if (UTF8PROC_FOUND)
    target_link_libraries(${QTERMWIDGET_LIBRARY_NAME} ${UTF8PROC_LIBRARIES})
endif()
if(APPLE)
    set (CMAKE_SKIP_RPATH 1)
    # this is a must to load the lib correctly
    set_target_properties(${QTERMWIDGET_LIBRARY_NAME} PROPERTIES INSTALL_NAME_DIR ${CMAKE_INSTALL_FULL_LIBDIR})
endif()

target_compile_definitions(${QTERMWIDGET_LIBRARY_NAME}
    PRIVATE
        "KB_LAYOUT_DIR=\"${KB_LAYOUT_DIR}\""
        "COLORSCHEMES_DIR=\"${COLORSCHEMES_DIR}\""
        "TRANSLATIONS_DIR=\"${TRANSLATIONS_DIR}\""
        "HAVE_POSIX_OPENPT"
        "HAVE_SYS_TIME_H"
)


generate_export_header(${QTERMWIDGET_LIBRARY_NAME}
    EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/lib/qtermwidget_export.h"
    EXPORT_MACRO_NAME QTERMWIDGET_EXPORT
)

write_basic_package_version_file(
    "${CMAKE_BINARY_DIR}/${QTERMWIDGET_LIBRARY_NAME}-config-version.cmake"
    VERSION ${QTERMWIDGET_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_BINARY_DIR}/${QTERMWIDGET_LIBRARY_NAME}-config-version.cmake"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/${QTERMWIDGET_LIBRARY_NAME}"
    COMPONENT Devel
)
install(TARGETS ${QTERMWIDGET_LIBRARY_NAME} DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(FILES
    ${HDRS_DISTRIB} "${CMAKE_CURRENT_BINARY_DIR}/lib/qtermwidget_export.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${QTERMWIDGET_LIBRARY_NAME}"
)
# keyboard layouts
install(DIRECTORY lib/kb-layouts/ DESTINATION "${KB_LAYOUT_DIR}" FILES_MATCHING PATTERN "*.keytab" )
# color schemes
install(DIRECTORY lib/color-schemes/ DESTINATION "${COLORSCHEMES_DIR}" FILES_MATCHING PATTERN "*.*schem*")

include(create_pkgconfig_file)
create_pkgconfig_file(${QTERMWIDGET_LIBRARY_NAME}
    "QTermWidget library for Qt ${QTERMWIDGET_VERSION_MAJOR}.x"
    ${PKG_CONFIG_REQ}
    ${QTERMWIDGET_LIBRARY_NAME}
    ${QTERMWIDGET_VERSION}
)

configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/${QTERMWIDGET_LIBRARY_NAME}-config.cmake.in"
    "${CMAKE_BINARY_DIR}/${QTERMWIDGET_LIBRARY_NAME}-config.cmake"
    @ONLY
)
install(FILES
    "${CMAKE_BINARY_DIR}/${QTERMWIDGET_LIBRARY_NAME}-config.cmake"
    "${CMAKE_SOURCE_DIR}/cmake/${QTERMWIDGET_LIBRARY_NAME}_use.cmake"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/${QTERMWIDGET_LIBRARY_NAME}"
)
install(FILES ${QTERMWIDGET_QM} DESTINATION ${TRANSLATIONS_DIR})
# end of main library


# test application
if(BUILD_TEST)
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
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
)
