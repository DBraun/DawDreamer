#pragma once
#ifdef BUILD_DAWDREAMER_FAUST

#include <faust/compiler/generator/libfaust.h>
#include <faust/dsp/libfaust-box.h>
//#include <faust/dsp/libfaust-signal.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>  // this lets std::vector<float> be a default arg

// todo: don't include a .hh file
#include <faust/compiler/tlib/tree.hh>

namespace py = pybind11;

struct BoxWrapper {
  CTree *ptr;
  BoxWrapper(Box ptr, int inputs, int outputs)
      : ptr{ptr}, mInputs{inputs}, mOutputs{outputs} {}
  BoxWrapper(Box ptr) : ptr{ptr} {}
  BoxWrapper(float val) : ptr{boxReal(val)} {}
  BoxWrapper(int val) : ptr{boxInt(val)} {}
  operator CTree *() { return ptr; }

  int getInputs() {
    if (!mInputs.has_value()) {
      calculateBoxType();
    }
    return mInputs.value();
  }

  int getOutputs() {
    if (!mOutputs.has_value()) {
      calculateBoxType();
    }
    return mOutputs.value();
  }

  bool getValid() {
    if (!mValid.has_value()) {
      calculateBoxType();
    }
    return mValid.value();
  }

 private:
  std::optional<int> mInputs;
  std::optional<int> mOutputs;
  std::optional<bool> mValid;

  void calculateBoxType() {
    int inputs = 0;
    int outputs = 0;

    mValid = getBoxType(ptr, &inputs, &outputs);
    mInputs = inputs;
    mOutputs = outputs;
  }
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

py::module_ &create_bindings_for_faust_box(py::module &faust_module,
                                           py::module &box_module);

#endif
