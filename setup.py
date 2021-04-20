#!/usr/bin/env python
# -*- coding:utf-8 -*-

"""Setup this SWIG library."""
import sys
from distutils.command.build import build
from pathlib import Path

import pkgconfig
from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext

# basedir = Path(__file__).parent.resolve()

source_list = sorted(
    str(p)
    for p in Path("src").glob("*.c")
    if p.name not in ["ObitAIPSObject.c", "ObitAIPSFortran.c"]
)
source_list.append("obit/Obit.i")

OBIT_EXT = Extension(
    name="obit._Obit",
    sources=source_list,
    include_dirs=["include"],
    define_macros=[
        # ("HAVE_GSL", "1"),
        # ("HAVE_FFTW3", "1"),
        # ("HAVE_PGPLOT", "1"),
        ("OBIT_THREADS_ENABLED", "1"),
        ("HAVE_SSE", "1"),
    ],
    extra_compile_args=["-O2", "-march=native"],
)


def update_pkg(ext, package, macro=None):
    """Update extention cflags and libs according to pkg-config info."""
    if not pkgconfig.exists(package):
        print(f"Could not find package {package}", file=sys.stderr)
        sys.exit(1)

    for k, value in pkgconfig.parse(package).items():
        getattr(ext, k).extend(value)

    if macro:
        ext.define_macros.append((macro, "1"))


class CustomBuild(build):
    """Ensure build_ext runs first in build command."""

    def run(self):
        self.run_command("build_ext")
        return build.run(self)


class CustomBuildExt(build_ext):
    def run(self):
        ext = self.extensions[0]
        assert ext.name == "obit._Obit"

        update_pkg(ext, "glib-2.0 gthread-2.0")
        update_pkg(ext, "fftw3f", "HAVE_FFTW3")
        update_pkg(ext, "gsl", "HAVE_GSL")
        update_pkg(ext, "xmlrpc_server_abyss xmlrpc_client")
        update_pkg(ext, "cfitsio")

        return build_ext.run(self)

    def swig_sources(self, sources, extension):
        """Make Obit.i file from *.inc files."""
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
    cmdclass={
        "build": CustomBuild,
        "build_ext": CustomBuildExt,
    },
    python_requires=">=3.4",
    setup_requires=["pkgconfig"],
)
