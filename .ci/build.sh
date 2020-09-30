set -ex

mkdir build
cd build
cmake \
    -DBUILD_EXAMPLE=ON \
    -DQTERMWIDGET_BUILD_PYTHON_BINDING=ON \
    -DQTERMWIDGET_USE_UTEMPTER=ON \
    ..
make
