#!/usr/bin/env python

import shlex
import subprocess
import sys
from distutils.command.build import build
from pathlib import Path

import pkgconfig
from setuptools import Extension, setup
from setuptools.command.build_clib import build_clib
from setuptools.command.build_ext import build_ext

LIBS_BUILD_INFO = {
    "include_dirs": [],
    "define_macros": [],
    "library_dirs": [],
    "libraries": [],
}

CFLAGS = ["-fno-strict-aliasing", "-O2", "-march=native"]

# Build C-part of Obit as static lib
OBIT_LIB = (
    "obit_lib",
    {
        "sources": sorted(str(p) for p in Path("src").glob("*.c")),
        "include_dirs": ["include"],
        "macros": [
            ("OBIT_THREADS_ENABLED", "1"),
            ("HAVE_SSE", "1"),
            ("FASTOBITMEM", "1"),
        ],
        "cflags": CFLAGS,
    },
)


OBIT_EXT = Extension(
    name="obit._Obit",
    sources=["obit/Obit.i"],
    include_dirs=["include"],
    extra_compile_args=CFLAGS,
    libraries=["obit_lib"],
)


def run_command(cmd):
    """Run `cmd` and return its stdout."""
    if isinstance(cmd, str):
        cmd = shlex.split(cmd)

    ret = subprocess.run(cmd, capture_output=True, check=True)

    return ret.stdout.decode().strip()


def update_build_info(build_info):
    """Update LIBS_BUILD_INFO dictionary."""
    #
    # For xmlrpc-c use xmlrpc-c-config command
    #
    try:
        ret = run_command("xmlrpc-c-config --features")
    except OSError as err:
        print(err, file=sys.stderr)
        print("Error: xmlrpc-c not found", file=sys.stderr)
        sys.exit(1)

    if "abyss-server" not in ret:
        print("Error: xmlrpc-c does not support abyss-server feature", file=sys.stderr)
        sys.exit(1)

    ret = run_command("xmlrpc-c-config abyss-server client --cflags")
    include_dirs = [x[2:] for x in ret.split() if x[0:2] == "-I"]
    ret = run_command("xmlrpc-c-config abyss-server client --libs")
    libraries = [x[2:] for x in ret.split() if x[0:2] == "-l"]
    library_dirs = [x[2:] for x in ret.split() if x[0:2] == "-L"]

    build_info["include_dirs"].extend(include_dirs)
    build_info["libraries"].extend(libraries)
    build_info["library_dirs"].extend(library_dirs)

    #
    # For the rest libs use pkg-config
    #
    try:
        ret = pkgconfig.parse("glib-2.0 gthread-2.0 fftw3f gsl cfitsio")
    except pkgconfig.PackageNotFoundError as err:
        print(err, file=sys.stderr)
        sys.exit(1)

    for key, value in ret.items():
        build_info[key].extend(value)

    build_info["define_macros"].extend([("HAVE_FFTW3", 1), ("HAVE_GSL", 1)])


class CustomBuild(build):
    """Ensure build_clib runs before build_ext."""

    def run(self):
        self.run_command("build_clib")
        self.run_command("build_ext")
        return build.run(self)


class CustomBuildClib(build_clib):
    def finalize_options(self):
        build_clib.finalize_options(self)

        ext = self.libraries[0]
        assert ext[0] == "obit_lib"

        if not LIBS_BUILD_INFO["define_macros"]:
            update_build_info(LIBS_BUILD_INFO)

        ext[1]["include_dirs"].extend(LIBS_BUILD_INFO["include_dirs"])
        ext[1]["macros"].extend(LIBS_BUILD_INFO["define_macros"])


class CustomBuildExt(build_ext):
    def finalize_options(self):
        build_ext.finalize_options(self)

        ext = self.extensions[0]
        assert ext.name == "obit._Obit"

        if not LIBS_BUILD_INFO["define_macros"]:
            update_build_info(LIBS_BUILD_INFO)

        for k, value in LIBS_BUILD_INFO.items():
            getattr(ext, k).extend(value)

    def swig_sources(self, sources, extension):
        """Make Obit.i file from the *.inc files."""
        for source in sources:
            source_path = Path(source)

            if source_path.suffix == ".i":
                swig_dir = source_path.parent
                with source_path.open("w") as file:
                    print("%module Obit", file=file)
                    file.write((swig_dir / "ObitTypeMaps.swig").read_text())

                    for inc_file in sorted(swig_dir.glob("*.inc")):
                        file.write(inc_file.read_text())
                break

        return build_ext.swig_sources(self, sources, extension)


setup(
    name="obit",
    description="Obit for ParselTongue",
    version="0.1.0",
    author="Bill Cotton",
    license="GPL-2",
    keywords=["SWIG", "Obit"],
    packages=["obit"],
    ext_modules=[OBIT_EXT],
    libraries=[OBIT_LIB],
    cmdclass={
        "build": CustomBuild,
        "build_ext": CustomBuildExt,
        "build_clib": CustomBuildClib,
    },
    python_requires=">=3.6",
    setup_requires=["pkgconfig"],
    install_requires=["six"],
)
