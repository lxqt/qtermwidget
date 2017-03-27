PyQt5 Bindings for QTermWidget
==============================


INSTALL: 
------------
	
####1. Download, compile and install QTermWidget:
	$ git clone https://github.com/lxde/qtermwidget.git
	$ cd qtermwidget && mkdir build && cd build
	$ cmake ..
	$ make
	$ sudo make install
If `make install` command will not work just copy the `qtermwidget.so*` files to /usr/lib directory.
####2. Install PyQt5 and PyQt5-devel if not yet installed.
####3. Configure, compile and install Python bindings. Execute in terminal in the qtermwidget bindings folder:
	$ cd pyqt/
	$ QT_SELECT=5 python config.py
	$ make
	$ sudo make install

####4. You can run ./test.py to test the installed module.


ABOUT:
---------
Curently maintained by:
- Pawel Koston <pawelkoston@gmail.com>

Based on previous PyQt4 bindings  by:
- Piotr "Riklaunim" Maliński <riklaunim@gmail.com>,
- Alexander Slesarev <alex.slesarev@gmail.com>

PyQt5 QTermWidget Bindings 
License: GPL3

