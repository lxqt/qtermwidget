PyQt5 Bindings for QTermWidget
==============================


INSTALL: 
------------
####1. Download QTermWidget -> https://github.com/lxde/qtermwidget	
	
####2. Compile and install it:
	$ mkdir build && cd build
	$ cmake ..
	$ make
	$ sudo make install
If `make install` command will not work just copy the `qtermwidget.so*` files to /usr/lib directory.
####3. Install PyQt5 and PyQt5-devel if not yet installed.
####4. Configure, compile and install bindings. Execute in terminal in the qtermwidget bindings folder:

	$ python config.py
	$ make
	$ sudo make install

####5. You can run ./test.py to test the installed module.


ABOUT:
---------
Based on previous PyQt4 bindings  by:
- Piotr "Riklaunim" Maliński <riklaunim@gmail.com>,
- Alexander Slesarev <alex.slesarev@gmail.com>


PyQt5 QTermWidget Bindings 
License: GPL3

