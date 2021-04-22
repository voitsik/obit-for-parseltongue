#!/usr/bin/env python
# -*- coding:utf-8 -*-

"""Setup this SWIG library."""
import sys
from distutils.command.build import build
from pathlib import Path

import pkgconfig
from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext
from setuptools.command.build_clib import build_clib


cflags = ["-fno-strict-aliasing", "-O2", "-march=native"]

# Build C-part of Obit as static lib
obit_lib = (
    "obit_lib",
    {
        "sources": sorted(str(p) for p in Path("src").glob("*.c")),
        "include_dirs": ["include"],
        "macros": [
            ("OBIT_THREADS_ENABLED", "1"),
            ("HAVE_SSE", "1"),
            ("FASTOBITMEM", "1"),
        ],
        "cflags": cflags,
    },
)


OBIT_EXT = Extension(
    name="obit._Obit",
    sources=["obit/Obit.i"],
    include_dirs=["include"],
    extra_compile_args=cflags,
    libraries=["obit_lib"],
)


def update_build_info(ext, package, macro=None):
    """Update extention or library build info according to pkg-config info."""
    if not pkgconfig.exists(package):
        print(f"Could not find package {package}", file=sys.stderr)
        sys.exit(1)

    if isinstance(ext, Extension):
        for k, value in pkgconfig.parse(package).items():
            getattr(ext, k).extend(value)

        if macro:
            ext.define_macros.append((macro, "1"))
    elif isinstance(ext, tuple):
        pkg_info = pkgconfig.parse(package)
        if "include_dirs" in pkg_info:
            ext[1]["include_dirs"].extend(pkg_info["include_dirs"])

        if "define_macros" in pkg_info:
            ext[1]["macros"].extend(pkg_info["define_macros"])

        if macro:
            ext[1]["macros"].append((macro, "1"))
    else:
        assert False


class CustomBuild(build):
    """Ensure build_clib runs before build_ext."""

    def run(self):
        self.run_command("build_clib")
        self.run_command("build_ext")
        return build.run(self)


class CustomBuildClib(build_clib):
    def run(self):
        ext = self.libraries[0]
        assert ext[0] == "obit_lib"

        update_build_info(ext, "glib-2.0 gthread-2.0")
        update_build_info(ext, "fftw3f", "HAVE_FFTW3")
        update_build_info(ext, "gsl", "HAVE_GSL")
        update_build_info(ext, "xmlrpc_server_abyss xmlrpc_client")
        update_build_info(ext, "cfitsio")

        return build_clib.run(self)


class CustomBuildExt(build_ext):
    def run(self):
        ext = self.extensions[0]
        assert ext.name == "obit._Obit"

        update_build_info(ext, "glib-2.0 gthread-2.0")
        update_build_info(ext, "fftw3f", "HAVE_FFTW3")
        update_build_info(ext, "gsl", "HAVE_GSL")
        update_build_info(ext, "xmlrpc_server_abyss xmlrpc_client")
        update_build_info(ext, "cfitsio")

        return build_ext.run(self)

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
    libraries=[obit_lib],
    cmdclass={
        "build": CustomBuild,
        "build_ext": CustomBuildExt,
        "build_clib": CustomBuildClib,
    },
    python_requires=">=3.6",
    setup_requires=["pkgconfig"],
    install_requires=["six"],
)
