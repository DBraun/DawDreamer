#pragma once
#ifdef BUILD_DAWDREAMER_FAUST

#include <faust/dsp/libfaust-signal.h>

#include "FaustBoxAPI.h"

struct SigWrapper {
  CTree *ptr;
  SigWrapper(Signal ptr) : ptr{ptr} {}
  SigWrapper(float val) : ptr{sigReal(val)} {}
  SigWrapper(int val) : ptr{sigInt(val)} {}
  operator CTree *() { return ptr; }
};

void create_bindings_for_faust_signal(py::module &faust_module,
                                      py::module &box_module);

#endif