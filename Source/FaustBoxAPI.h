#pragma once
#ifdef BUILD_DAWDREAMER_FAUST

#include <faust/compiler/generator/libfaust.h>
#include <faust/dsp/libfaust-box.h>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>  // this lets std::vector<float> be a default arg

// todo: don't include a .hh file
#include <faust/compiler/tlib/tree.hh>

namespace py = pybind11;

struct BoxWrapper {
  CTree *ptr;
  BoxWrapper(Box ptr) : ptr{ptr} {}
  BoxWrapper(float val) : ptr{boxReal(val)} {}
  BoxWrapper(int val) : ptr{boxInt(val)} {}
  operator CTree *() { return ptr; }
};

class DawDreamerFaustLibContext {
 public:
  int enter() {
    createLibContext();
    return 1;
  };
  void exit(const py::object &type, const py::object &value,
            const py::object &traceback) {
    destroyLibContext();
  };
};

std::string getPathToFaustLibraries();

std::string getPathToArchitectureFiles();

py::module_ &create_bindings_for_faust_box(py::module &faust_module, py::module& box_module);

#endif
