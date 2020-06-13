set -ex

mkdir build
cd build
CC=clang CXX=clang++ cmake \
    -DBUILD_EXAMPLE=ON \
    -DQTERMWIDGET_BUILD_PYTHON_BINDING=ON \
    -DLXQT_ENABLE_CLAZY=ON \
    ..
make
