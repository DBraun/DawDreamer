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


class BinaryDistribution(Distribution):
    """Distribution which always forces a binary package with platform name"""

    def has_ext_modules(foo):
        return True


ext_modules = []
package_data = []

with contextlib.suppress(Exception):
    shutil.copytree("licenses", os.path.join("dawdreamer", "licenses"))

if platform.system() == "Windows":
    build_folder = os.path.join(
        this_dir, "Builds", "VisualStudio2022", "x64", "Release", "Dynamic Library"
    )
    shutil.copy(
        os.path.join(build_folder, "dawdreamer.dll"), os.path.join("dawdreamer", "dawdreamer.pyd")
    )

    package_data += ["dawdreamer/dawdreamer.pyd"]

elif platform.system() == "Linux":
    files = ["dawdreamer/dawdreamer.so"]
    for file in files:
        filepath = os.path.abspath(file)
        assert os.path.isfile(filepath), ValueError("File not found: " + filepath)
    print("Using compiled files: ", str(files))

    package_data += files

elif platform.system() == "Darwin":
    build_folder = os.path.join(
        this_dir, "Builds", "MacOSX", "build", "Release-" + os.environ["ARCHS"]
    )

    shutil.copy(
        os.path.join(build_folder, "dawdreamer.so"), os.path.join("dawdreamer", "dawdreamer.so")
    )

    package_data += ["dawdreamer/dawdreamer.so"]

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
print("package_data: ", package_data)
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
