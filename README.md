# QTermWidget

## Overview

A terminal emulator widget for Qt 5.   

QTermWidget is an open-source project originally based on KDE4 Konsole application, but it took its own direction later.   
The main goal of this project is to provide a unicode-enabled, embeddable Qt widget for using as a built-in console (or terminal emulation widget).   

It is compatible with BSD, Linux and OS X.   

This project is licensed under the terms of the [GPLv2](https://www.gnu.org/licenses/gpl-2.0.en.html) or any later version. See the LICENSE file for the full text of the license.   

## Installation

### Compiling sources

The only runtime dependency is qtbase ≥ 5.4.   
In order to build CMake ≥ 3.0 is needed as well as optionally Git to pull latest VCS checkouts.

Code configuration is handled by CMake. Building out of source is strongly recommended. CMake variable `CMAKE_INSTALL_PREFIX` will normally have to be set to `/usr`, depending on the way library paths are dealt with on 64bit systems variables like `CMAKE_INSTALL_LIBDIR` may have to be set as well.   

To build run `make`, to install `make install` which accepts variable `DESTDIR` as usual.   

### Binary packages

The library is provided by all major Linux distributions like Arch Linux, Debian, Fedora and openSUSE.   
Just use the distributions' package managers to search for string `qtermwidget`.
