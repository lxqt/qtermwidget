from setuptools.command.build_ext import build_ext as build_ext_orig
from setuptools import setup, Extension
import subprocess
import tempfile
import platform
import pathlib
import shutil
import glob
import sys
import os


if platform.system() == "Windows":
    print("Windows is not supportet", file=sys.stderr)
    sys.exit(1)


with open("README.md", "r", encoding="utf-8") as f:
    description = f.read()


build_dir = tempfile.mkdtemp()


def prepare_build():
    shutil.copy("QTermWidget/__init__.py", os.path.join(build_dir, "__init__.py"))
    shutil.copytree("lib/color-schemes", os.path.join(build_dir, "color-schemes"))
    shutil.copytree("lib/kb-layouts", os.path.join(build_dir, "kb-layouts"))
    shutil.copytree("lib/translations", os.path.join(build_dir, "translations"))
    subprocess.run(["lrelease"] + glob.glob(os.path.join(build_dir, "translations", "*.ts")))


class CMakeExtension(Extension):

    def __init__(self, name):
        # don't invoke the original build_ext for this special extension
        super().__init__(name, sources=[])


class build_ext(build_ext_orig):

    def run(self):
        for ext in self.extensions:
            self.build_cmake(ext)
        super().run()

    def build_cmake(self, ext):
        cwd = pathlib.Path().absolute()

        # these dirs will be created in build_py, so if you don't have
        # any python sources to bundle, the dirs will be missing
        build_temp = pathlib.Path(self.build_temp)
        build_temp.mkdir(parents=True, exist_ok=True)
        extdir = pathlib.Path(self.get_ext_fullpath(ext.name))
        extdir.mkdir(parents=True, exist_ok=True)

        # example of cmake args
        config = "Debug" if self.debug else "Release"
        cmake_args = [
            "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=" + str(extdir.parent.absolute()),
            "-DCMAKE_BUILD_TYPE=" + config,
            "-D",
            "QTERMWIDGET_BUILD_PYTHON_BINDING=ON"
        ]

        # example of build args
        build_args = [
            "--config", config,
            "--", "-j4"
        ]

        os.chdir(str(build_temp))
        self.spawn(["cmake", str(cwd)] + cmake_args)
        if not self.dry_run:
            self.spawn(["cmake", "--build", "."] + build_args)
        # Troubleshooting: if fail on line above then delete all possible
        # temporary CMake files including "CMakeCache.txt" in top level dir.
        os.chdir(str(cwd))


prepare_build()

setup(
    name="QTermWidget",
    version='0.1',
    description=" The terminal widget for QTerminal",
    long_description=description,
    long_description_content_type="text/markdown",
    url="https://github.com/lxqt/qtermwidget",
    python_requires=">=3.7",
    install_requires=["PyQt5"],
    packages=["QTermWidget"],
    ext_modules=[CMakeExtension("QTermWidget/*")],
    cmdclass={
        "build_ext": build_ext,
    },
    package_dir={"QTermWidget": build_dir},
    package_data={
        "QTermWidget": [
            "color-schemes/*",
            "color-schemes/historic/*",
            "kb-layouts/*",
            "kb-layouts/historic/*",
            "translations/*.qm"
        ]
    },
    classifiers=[
        "Development Status :: 5 - Production/Stable",
        "Intended Audience :: Developers",
        "Environment :: Other Environment",
        "Environment :: X11 Applications :: Qt",
        "License :: OSI Approved :: BSD License",
        "Operating System :: POSIX",
        "Operating System :: POSIX :: BSD",
        "Operating System :: POSIX :: Linux",
        "Operating System :: MacOS :: MacOS X",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3 :: Only",
        "Programming Language :: Python :: Implementation :: CPython"
      ]
)

shutil.rmtree(build_dir)
