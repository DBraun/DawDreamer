# Test

`python -m pytest .`

Or verbosely

`python -m pytest -s .`

Or

`python -m pytest -v .`

You can do specific files and tests:

`python -m pytest -v test_plugins.py -k "test_plugin_editor" -p no:faulthandler`

Note that `-p no:faulthandler` is included to avoid errors when GUIs are opened.
