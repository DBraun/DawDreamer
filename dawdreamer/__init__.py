from .dawdreamer import *

# Version is written to _version.py by setup.py during installation
try:
    from ._version import __version__
except ImportError:
    __version__ = "unknown"
