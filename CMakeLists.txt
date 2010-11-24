###| Cmake qtermwidget |###

option(BUILD_DESIGNER_PLUGIN "Build Qt4 designer plugin" ON)

#| Setting required version
cmake_minimum_required( VERSION 2.4 )

#| Project
project( qtermwidget )

subdirs( 
	lib
	src
)

