# Unreleased

- Removed C sources not reachable from the Python interface (Weather, SkyModel, DCon/DConClean, and other unused classes).
- Removed ObitDisplay, ObitRPC, ObitXML, the `ODisplay` module and `Image.TVFit`; `xmlrpc-c` is no longer required.
- Removed `UV.PEdit*`, `OWindow`, `UVSoln` and the unused `TableOT/PT/SY` wrappers together with their C code.

# 0.2.2 (2026-10-18)

- Updated SWIG wrappers for Python 3 and expanded supported Python versions.
- Fixed UV and Image existence checks.
- Added basic FArray tests.

# 0.2.1 (2023-01-15)

- Migrated packaging configuration to `pyproject.toml` and excluded non-Python files from package installation.
- Added InfoList tests.
- Removed unused modules and the `six` dependency.
- Added macOS build support.

# 0.2.0 (2023-01-08)

- Refactored and reformatted Python modules, with fixes to error handling and installation.
