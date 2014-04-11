find_package(Qt5Widgets REQUIRED)

include_directories(${Qt5Widgets_INCLUDE_DIRS})
add_definitions(${Qt5Core_DEFINITIONS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
set(QTERMWIDGET_QT_LIBRARIES ${Qt5Widgets_LIBRARIES})

include_directories(${QTERMWIDGET_INCLUDE_DIRS})

