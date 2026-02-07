#! /usr/bin/env python

# NOTE: You can test building wheels locally with
# `python -m build --wheel`
# Then in the `dist` directory, `pip install dawdreamer`

import contextlib
import glob
import os
import os.path
import platform
import shutil
import subprocess
import sysconfig
from pathlib import Path

import setuptools
from setuptools import setup
from setuptools.dist import Distribution

this_dir = os.path.abspath(os.path.dirname(__file__))


def get_dawdreamer_version():
    import xml.etree.ElementTree as ET

    tree = ET.parse(Path(__file__).parent / "DawDreamer.jucer")
    root = tree.getroot()
    version = root.attrib["version"]
    return version


DAWDREAMER_VERSION = get_dawdreamer_version()

# Write version file so it's available in installed packages
version_file = Path(this_dir) / "dawdreamer" / "_version.py"
version_file.write_text(f'__version__ = "{DAWDREAMER_VERSION}"\n')


class BinaryDistribution(Distribution):
    """Distribution which always forces a binary package with platform name"""

    def has_ext_modules(foo):
        return True


def _needs_build(build_so: str, source_dir: str) -> bool:
    """Check if the .so needs rebuilding by comparing mtimes against C++ sources."""
    if not os.path.isfile(build_so):
        return True
    so_mtime = os.path.getmtime(build_so)
    for ext in ("*.cpp", "*.h"):
        for src_file in glob.glob(os.path.join(source_dir, ext)):
            if os.path.getmtime(src_file) > so_mtime:
                return True
    return False


def _run(cmd: list[str], **kwargs):
    """Run a subprocess, printing the command and raising on failure."""
    print(f"  Running: {' '.join(cmd)}")
    subprocess.check_call(cmd, **kwargs)


def _build_libsamplerate():
    """Build libsamplerate if not already built."""
    libsr_dir = os.path.join(this_dir, "thirdparty", "libsamplerate")
    build_dir = os.path.join(libsr_dir, "build_release")
    # Check if already built (look for the library file)
    if os.path.isdir(build_dir) and any(
        f.endswith((".a", ".so", ".dylib", ".lib"))
        for f in os.listdir(build_dir)
        if os.path.isfile(os.path.join(build_dir, f))
    ):
        return
    print("Building libsamplerate...")
    cmake_args = ["-DCMAKE_BUILD_TYPE=Release", f"-B{build_dir}"]
    if platform.system() == "Darwin":
        archs = os.environ.get("ARCHS", "arm64" if os.uname().machine == "arm64" else "x86_64")
        cmake_args.append(f"-DCMAKE_OSX_ARCHITECTURES={archs}")
        cmake_args.append("-DCMAKE_OSX_DEPLOYMENT_TARGET=12.0")
    cmake_args.append("-DLIBSAMPLERATE_EXAMPLES=off")
    _run(["cmake"] + cmake_args, cwd=libsr_dir)
    _run(["cmake", "--build", build_dir, "--config", "Release"], cwd=libsr_dir)


def _build_and_copy_linux() -> str:
    """Build on Linux and return the destination .so path."""
    dest_so = os.path.join(this_dir, "dawdreamer", "dawdreamer.so")
    build_so = os.path.join(this_dir, "Builds", "LinuxMakefile", "build", "libdawdreamer.so")
    source_dir = os.path.join(this_dir, "Source")

    if (
        not _needs_build(build_so, source_dir)
        and os.path.isfile(dest_so)
        and os.path.getmtime(dest_so) >= os.path.getmtime(build_so)
    ):
        return dest_so

    if _needs_build(build_so, source_dir):
        _build_libsamplerate()
        python_include = sysconfig.get_path("include")
        makefile_dir = os.path.join(this_dir, "Builds", "LinuxMakefile")
        print(f"Building DawDreamer (Python include: {python_include})...")
        _run(
            ["make", "CONFIG=Release", f"CXXFLAGS=-I{python_include}", f"-j{os.cpu_count() or 1}"],
            cwd=makefile_dir,
        )

    if not os.path.isfile(build_so):
        raise FileNotFoundError(
            f"Build output not found: {build_so}\n"
            "  Try: cd Builds/LinuxMakefile && make CONFIG=Release"
        )

    print(f"Copying {build_so} -> {dest_so}")
    shutil.copy2(build_so, dest_so)
    return dest_so


def _build_and_copy_darwin() -> str:
    """Build on macOS and return the destination .so path."""
    dest_so = os.path.join(this_dir, "dawdreamer", "dawdreamer.so")
    archs = os.environ.get("ARCHS", "arm64" if os.uname().machine == "arm64" else "x86_64")
    configuration = f"Release-{archs}"
    build_folder = os.path.join(this_dir, "Builds", "MacOSX", "build", configuration)
    build_so = os.path.join(build_folder, "dawdreamer.so")
    build_dylib = os.path.join(build_folder, "dawdreamer.so.dylib")
    source_dir = os.path.join(this_dir, "Source")

    # Check if we need to build
    effective_build = build_so if os.path.isfile(build_so) else build_dylib
    if (
        not _needs_build(effective_build, source_dir)
        and os.path.isfile(dest_so)
        and os.path.getmtime(dest_so) >= os.path.getmtime(effective_build)
    ):
        return dest_so

    if _needs_build(effective_build, source_dir):
        _build_libsamplerate()
        print(f"Building DawDreamer (ARCHS={archs})...")
        _run(
            [
                "xcodebuild",
                f"ARCHS={archs}",
                f"-configuration={configuration}",
                "-project",
                os.path.join(this_dir, "Builds", "MacOSX", "DawDreamer.xcodeproj"),
                "CODE_SIGN_IDENTITY=",
                "CODE_SIGNING_REQUIRED=NO",
                "CODE_SIGN_ENTITLEMENTS=",
                "CODE_SIGNING_ALLOWED=NO",
            ]
        )
        # Xcode produces .dylib, rename to .so
        if os.path.isfile(build_dylib) and not os.path.isfile(build_so):
            os.rename(build_dylib, build_so)

    if not os.path.isfile(build_so):
        raise FileNotFoundError(
            f"Build output not found: {build_so}\n" f"  Try: ARCHS={archs} ./build_macos.sh"
        )

    print(f"Copying {build_so} -> {dest_so}")
    shutil.copy2(build_so, dest_so)
    return dest_so


def _build_and_copy_windows() -> str:
    """Build on Windows and return the destination .pyd path."""
    dest_pyd = os.path.join(this_dir, "dawdreamer", "dawdreamer.pyd")
    build_folder = os.path.join(
        this_dir, "Builds", "VisualStudio2022", "x64", "Release", "Dynamic Library"
    )
    build_dll = os.path.join(build_folder, "dawdreamer.dll")
    source_dir = os.path.join(this_dir, "Source")

    if (
        not _needs_build(build_dll, source_dir)
        and os.path.isfile(dest_pyd)
        and os.path.getmtime(dest_pyd) >= os.path.getmtime(build_dll)
    ):
        return dest_pyd

    if _needs_build(build_dll, source_dir):
        _build_libsamplerate()
        print("Building DawDreamer...")
        _run(
            [
                "msbuild",
                os.path.join(this_dir, "Builds", "VisualStudio2022", "DawDreamer.sln"),
                "/property:Configuration=Release",
            ]
        )

    if not os.path.isfile(build_dll):
        raise FileNotFoundError(
            f"Build output not found: {build_dll}\n"
            "  Try: msbuild Builds/VisualStudio2022/DawDreamer.sln /property:Configuration=Release"
        )

    print(f"Copying {build_dll} -> {dest_pyd}")
    shutil.copy2(build_dll, dest_pyd)
    return dest_pyd


ext_modules = []
package_data = []

with contextlib.suppress(Exception):
    shutil.copytree("licenses", os.path.join("dawdreamer", "licenses"))

if platform.system() == "Windows":
    dest = _build_and_copy_windows()
    package_data += [dest]

elif platform.system() == "Linux":
    dest = _build_and_copy_linux()
    package_data += [dest]

elif platform.system() == "Darwin":
    dest = _build_and_copy_darwin()
    package_data += [dest]

else:
    raise NotImplementedError(f"setup.py hasn't been implemented for platform: {platform}.")

# copy architecture files


def copytree(src, dst, symlinks=False, ignore=None):
    if not os.path.exists(dst):
        os.makedirs(dst)
    for item in os.listdir(src):
        s = os.path.join(src, item)
        d = os.path.join(dst, item)
        if os.path.isdir(s):
            copytree(s, d, symlinks, ignore)
        else:
            if ignore is not None and ignore(os.fspath(src), [item]):
                continue

            if not os.path.exists(d) or os.stat(s).st_mtime - os.stat(d).st_mtime > 1:
                shutil.copy2(s, d)


destination = copytree(
    os.path.join(this_dir, "thirdparty", "faust", "architecture"),
    os.path.join(this_dir, "dawdreamer", "architecture"),
    ignore=shutil.ignore_patterns(
        "*.dll",
        "*.so",
        "*.html",
        "*.wav",
        "*.mp3",
        "*.png",
        "*.jpg",
        "*.pdf",
        "*.a",
        "*.wasm",
        "*.data",
    ),
)

# architecture files
architecture_files = list(
    glob.glob(os.path.join(this_dir, "dawdreamer/architecture/*"), recursive=True)
)
if not architecture_files:
    raise ValueError("You need to put the architecture directory inside dawdreamer.")
package_data += architecture_files

# Faust libraries
faustlibraries = list(
    glob.glob(os.path.join(this_dir, "dawdreamer/faustlibraries/*"), recursive=True)
)
if not faustlibraries:
    raise ValueError("You need to put the faustlibraries repo inside dawdreamer.")
package_data += faustlibraries

package_data += list(glob.glob("dawdreamer/licenses/*", recursive=True))

# Every item in package_data should be inside the dawdreamer directory.
# Then we make the paths relative to this directory.
package_data = [
    os.path.relpath(os.path.abspath(a), os.path.join(this_dir, "dawdreamer")).replace("\\", "/")
    for a in package_data
]
# print("package_data: ", package_data)
long_description = (Path(__file__).parent / "README.md").read_text()

setup(
    name="dawdreamer",
    url="https://github.com/DBraun/DawDreamer",
    project_urls={
        "Documentation": "https://dirt.design/DawDreamer",
        "Source": "https://github.com/DBraun/DawDreamer",
    },
    version=DAWDREAMER_VERSION,
    author="David Braun",
    author_email="braun@ccrma.stanford.edu",
    description="An audio-processing Python library supporting core DAW features",
    long_description=long_description,
    long_description_content_type="text/markdown",
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: GNU General Public License v3 (GPLv3)",
        "Operating System :: MacOS",
        "Operating System :: Microsoft :: Windows",
        "Operating System :: POSIX :: Linux",
        "Programming Language :: C++",
        "Programming Language :: Python",
        "Topic :: Multimedia :: Sound/Audio",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
    ],
    keywords="audio music sound",
    python_requires=">=3.8",
    install_requires=[],
    packages=setuptools.find_packages(),
    py_modules=["dawdreamer"],
    include_package_data=True,
    package_data={"": package_data},
    zip_safe=False,
    distclass=BinaryDistribution,
    ext_modules=ext_modules,
)
