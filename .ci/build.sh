set -ex

source shared-ci/prepare-archlinux.sh

# See *depends in https://github.com/archlinuxcn/repo/blob/master/archlinuxcn/qtermwidget-git/PKGBUILD
pacman -S --noconfirm --needed git cmake lxqt-build-tools-git qt5-tools python-sip4 sip4

# Use older PyQt for now as 5.15.6 does not work with SIP 4
# https://www.riverbankcomputing.com/pipermail/pyqt/2021-November/044345.html
pacman -U --noconfirm https://archive.archlinux.org/packages/p/python-pyqt5/python-pyqt5-5.15.5-1-x86_64.pkg.tar.zst

cmake -B build -S . \
    -DBUILD_EXAMPLE=ON \
    -DQTERMWIDGET_BUILD_PYTHON_BINDING=ON \
    -DQTERMWIDGET_USE_UTEMPTER=ON
make -C build
