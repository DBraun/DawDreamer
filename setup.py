#! /usr/bin/env python

from setuptools import setup, Extension
from setuptools.dist import Distribution
import os
from pathlib import Path
import shutil
import platform


if platform.system() == "Windows":

    this_dir = os.path.abspath(os.path.dirname(__file__))

    build_folder = os.path.join(this_dir, "Builds", "VisualStudio2019", "x64", "Release", "Dynamic Library")
    libfaust_folder = os.path.join(this_dir, "thirdparty", "libfaust", "win-x64", "Release", "bin")

    shutil.copy(os.path.join(build_folder, 'dawdreamer.dll'), os.path.join('dawdreamer', 'dawdreamer.pyd'))
    shutil.copy(os.path.join(libfaust_folder, 'faust.dll'), os.path.join('dawdreamer', 'faust.dll'))

    package_data = ['dawdreamer/faust.dll', 'dawdreamer/dawdreamer.pyd']

elif platform.system() == "Linux":

    this_dir = os.path.abspath(os.path.dirname(__file__))

    build_folder = os.path.join(this_dir, "Builds", "LinuxMakefile", "build")

    shutil.copy(os.path.join(build_folder, 'libdawdreamer.so'), os.path.join('dawdreamer', 'dawdreamer.so'))

    package_data = ['dawdreamer/dawdreamer.so']

elif platform.system() == "Darwin":

    this_dir = os.path.abspath(os.path.dirname(__file__))

    build_folder = os.path.join(this_dir, "Builds", "MacOSX", "build", "Release")
    libfaust_folder = os.path.join(this_dir, "thirdparty", "libfaust", "darwin-x64", "Release")

    shutil.copy(os.path.join(build_folder, 'dawdreamer.so'), os.path.join('dawdreamer', 'dawdreamer.so'))
    shutil.copy(os.path.join(libfaust_folder, 'libfaust.a'), os.path.join('dawdreamer', 'libfaust.2.dylib'))

    package_data = ['dawdreamer/libfaust.2.dylib', 'dawdreamer/dawdreamer.so']

else:
    raise NotImplementedError(
        f"setup.py hasn't been implemented for platform: {platform}."
    )

class BinaryDistribution(Distribution):
    """Distribution which always forces a binary package with platform name"""
    def has_ext_modules(foo):
        return True

this_dir = Path(__file__).parent
long_description = (this_dir / "README.md").read_text()

setup(
    name='dawdreamer',
    url='https://github.com/DBraun/DawDreamer/',
    version='0.5.1',
    author='David Braun',
    author_email='braun@ccrma.stanford.edu',
    description='An audio-processing Python library supporting core DAW features',
    long_description=long_description,
    long_description_content_type='text/markdown',
    classifiers=[
        "Development Status :: 4 - Beta",
        "License :: OSI Approved :: GNU General Public License v3 (GPLv3)",
        "Operating System :: MacOS",
        "Operating System :: Microsoft :: Windows",
        "Operating System :: POSIX :: Linux",
        "Programming Language :: C++",
        "Programming Language :: Python",
        "Topic :: Multimedia :: Sound/Audio",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
    ],
    install_requires=[],
    packages=['dawdreamer'],
    include_package_data=True,
    package_data={
        "": package_data,
    },
    zip_safe=False,
    distclass=BinaryDistribution
)
