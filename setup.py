#! /usr/bin/env python

# NOTE: You can test building wheels locally with
# `py -m build --wheel`
# Then in the `dist` directory, `pip install dawdreamer`

from setuptools import setup, Extension
from setuptools.dist import Distribution
import os
import os.path
from pathlib import Path
import shutil
import platform
import glob


this_dir = os.path.abspath(os.path.dirname(__file__))

def get_dawdreamer_version():
    import xml.etree.ElementTree as ET
    tree = ET.parse(Path(__file__).parent / 'DawDreamer.jucer')
    root = tree.getroot()
    version = root.attrib['version']
    return version


DAWDREAMER_VERSION = get_dawdreamer_version()


class BinaryDistribution(Distribution):
    """Distribution which always forces a binary package with platform name"""
    def has_ext_modules(foo):
        return True


ext_modules = []
package_data = []

try:
    shutil.copytree('licenses', os.path.join('dawdreamer', 'licenses'))
except Exception as e:
    pass

if platform.system() == "Windows":

    build_folder = os.path.join(this_dir, "Builds", "VisualStudio2019", "x64", "Release", "Dynamic Library")
    libfaust_folder = os.path.join(this_dir, "thirdparty", "libfaust", "win-x64", "Release", "bin")

    shutil.copy(os.path.join(build_folder, 'dawdreamer.dll'), os.path.join('dawdreamer', 'dawdreamer.pyd'))
    shutil.copy(os.path.join(libfaust_folder, 'faust.dll'), os.path.join('dawdreamer', 'faust.dll'))
    
    package_data += ['dawdreamer/faust.dll', 'dawdreamer/dawdreamer.pyd']

elif platform.system() == "Linux":

    files = ['dawdreamer/dawdreamer.so']
    files += list(glob.glob(os.path.join(this_dir, 'dawdreamer/libfaust*.so*')))
    for file in files:
        filepath = os.path.abspath(file)
        assert os.path.isfile(filepath), ValueError("File not found: " + filepath)
    print('Using compiled files: ', str(files))

    package_data += files

    # For Linux, we do a hacky thing where we force a compilation of an empty file
    # in order for auditwheel to work.
    dawdreamer_dir = os.path.join(this_dir, 'dawdreamer')
    ext_modules = [
        Extension(
            'dawdreamer',
            ['dawdreamer/null.c'],
            language='c++',
            libraries=['faust'],
            extra_link_args=["-Wl,--no-as-needed"],
            library_dirs=[dawdreamer_dir, './', 'dawdreamer', './dawdreamer', '/usr/local/lib', '/usr/lib/x86_64-linux-gnu'],
            runtime_library_dirs=[dawdreamer_dir, './', 'dawdreamer', './dawdreamer', '/usr/local/lib', '/usr/lib/x86_64-linux-gnu'],
            data_files=[('/usr/lib/x86_64-linux-gnu', glob.glob(os.path.join(this_dir, 'dawdreamer/libfaust*.so*')))]
        ),
    ]

elif platform.system() == "Darwin":

    build_folder = os.path.join(this_dir, "Builds", "MacOSX", "build", "Release")
    libfaust_folder = os.path.join(this_dir, "thirdparty", "libfaust", "darwin-x64", "Release")

    shutil.copy(os.path.join(build_folder, 'dawdreamer.so'), os.path.join('dawdreamer', 'dawdreamer.so'))
    shutil.copy(os.path.join(libfaust_folder, 'libfaust.a'), os.path.join('dawdreamer', 'libfaust.2.dylib'))

    package_data += ['dawdreamer/libfaust.2.dylib', 'dawdreamer/dawdreamer.so']

else:
    raise NotImplementedError(
        f"setup.py hasn't been implemented for platform: {platform}."
    )

faustlibraries = list(glob.glob(os.path.join(this_dir, 'dawdreamer/faustlibraries/*'), recursive=True))

if not faustlibraries:
    raise ValueError("You need to put the faustlibraries repo inside dawdreamer.")

package_data += faustlibraries

package_data += list(glob.glob('dawdreamer/licenses/*', recursive=True))

# Every item in package_data should be inside the dawdreamer directory.
# Then we make the paths relative to this directory.
package_data = [os.path.relpath(os.path.abspath(a), os.path.join(this_dir, "dawdreamer")).replace('\\', '/') for a in package_data]
print('package_data: ', package_data)
long_description = (Path(__file__).parent / "README.md").read_text()

setup(
    name='dawdreamer',
    url='https://github.com/DBraun/DawDreamer',
    project_urls={
        'Documentation': 'https://dirt.design/DawDreamer',
        'Source': 'https://github.com/DBraun/DawDreamer',
    },
    version=DAWDREAMER_VERSION,
    author='David Braun',
    author_email='braun@ccrma.stanford.edu',
    description='An audio-processing Python library supporting core DAW features',
    long_description=long_description,
    long_description_content_type='text/markdown',
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
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10"
    ],
    keywords='audio music sound',
    python_requires=">=3.7",
    install_requires=[],
    packages=['dawdreamer'],
    py_modules=['dawdreamer'],
    include_package_data=True,
    package_data={
        "": package_data
    },
    zip_safe=False,
    distclass=BinaryDistribution,
    ext_modules=ext_modules
)
