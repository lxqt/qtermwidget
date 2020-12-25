set -ex

cat >> /etc/pacman.conf <<EOF
[archlinuxcn]
Server = https://repo.archlinuxcn.org/\$arch
SigLevel = Never
EOF

pacman -Syu --noconfirm
# See *depends in https://github.com/archlinuxcn/repo/blob/master/archlinuxcn/qtermwidget-git/PKGBUILD
pacman -S --noconfirm --needed git cmake lxqt-build-tools-git qt5-tools python-pyqt5 python-sip sip

cmake -B build -S . \
    -DBUILD_EXAMPLE=ON \
    -DQTERMWIDGET_BUILD_PYTHON_BINDING=ON \
    -DQTERMWIDGET_USE_UTEMPTER=ON
make -C build
