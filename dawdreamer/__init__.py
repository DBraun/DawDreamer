#!/usr/bin/env python
# -*- encoding: utf-8 -*-

import importlib
# https://stackoverflow.com/a/43059528
# get a handle on the module
mdl = importlib.import_module('.dawdreamer', '.dawdreamer')

# is there an __all__?  if so respect it
if "__all__" in mdl.__dict__:
    names = mdl.__dict__["__all__"]
else:
    # otherwise we import all names that don't begin with _
    names = [x for x in mdl.__dict__ if not x.startswith("_")]

# now drag them in
globals().update({k: getattr(mdl, k) for k in names})