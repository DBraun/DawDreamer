#pragma once
#ifdef BUILD_DAWDREAMER_FAUST

#include <faust/compiler/generator/libfaust.h>
#include <faust/dsp/libfaust-box.h>
#include <optional>
// #include <faust/dsp/libfaust-signal.h>
#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/variant.h>
#include <nanobind/stl/vector.h>

// todo: don't include a .hh file
#include <faust/compiler/tlib/tree.hh>

namespace nb = nanobind;

struct BoxWrapper
{
    CTree* ptr;
    BoxWrapper(Box ptr, int inputs, int outputs) : ptr{ptr}, mInputs{inputs}, mOutputs{outputs} {}
    BoxWrapper(Box ptr) : ptr{ptr} {}
    BoxWrapper(float val) : ptr{boxReal(val)} {}
    BoxWrapper(int val) : ptr{boxInt(val)} {}
    operator CTree*() { return ptr; }

    int getInputs()
    {
        if (!mInputs.has_value())
        {
            calculateBoxType();
        }
        return mInputs.value();
    }

    int getOutputs()
    {
        if (!mOutputs.has_value())
        {
            calculateBoxType();
        }
        return mOutputs.value();
    }

    bool getValid()
    {
        if (!mValid.has_value())
        {
            calculateBoxType();
        }
        return mValid.value();
    }

  private:
    std::optional<int> mInputs;
    std::optional<int> mOutputs;
    std::optional<bool> mValid;

    void calculateBoxType()
    {
        if (!ptr)
        {
            mValid = false;
            mInputs = 0;
            mOutputs = 0;
            return;
        }

        int inputs = 0;
        int outputs = 0;

        mValid = getBoxType(ptr, &inputs, &outputs);
        mInputs = inputs;
        mOutputs = outputs;
    }
};

class DawDreamerFaustLibContext
{
  public:
    int enter()
    {
        createLibContext();
        return 1;
    };
    void exit(nb::object type, nb::object value, nb::object traceback)
    {
        // Parameters are ignored, just destroy the context
        destroyLibContext();
    };
};

std::string getPathToFaustLibraries();

std::string getPathToArchitectureFiles();

nb::module_& create_bindings_for_faust_box(nb::module_& faust_module, nb::module_& box_module);

#endif
