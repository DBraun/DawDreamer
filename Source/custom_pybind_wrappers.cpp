#include "custom_pybind_wrappers.h"

typedef std::vector<std::pair<int, float>> PluginPatch;

namespace customBoost {

template <class T>
py::list arrayToList(std::array<T, 13> array) {
  typename std::array<T, 13>::iterator iter;
  py::list list;
  for (iter = array.begin(); iter != array.end(); ++iter) {
    list.append(*iter);
  }
  return list;
}

// Converts a C++ vector to a python list
template <class T>
py::list toPythonList(std::vector<T> vector) {
  typename std::vector<T>::iterator iter;
  py::list list;
  for (iter = vector.begin(); iter != vector.end(); ++iter) {
    list.append(*iter);
  }
  return list;
}

//==========================================================================
// Converts a std::pair which is used as a parameter in C++
// into a tuple with the respective types int and float for
// use in Python.
py::tuple parameterToTuple(std::pair<int, float> parameter) {
  py::tuple parameterTuple;
  parameterTuple = py::make_tuple(parameter.first, parameter.second);
  return parameterTuple;
}

//==========================================================================
// Converts a PluginPatch ( std::vector <std::pair <int, float>> )
// to a Python list.
py::list pluginPatchToListOfTuples(PluginPatch parameters) {
  std::vector<std::pair<int, float>>::iterator iter;
  py::list list;
  for (iter = parameters.begin(); iter != parameters.end(); ++iter) {
    auto tup = parameterToTuple(*iter);
    list.append(tup);
  }
  return list;
}

//==========================================================================
PluginPatch listOfTuplesToPluginPatch(py::list listOfTuples) {
  PluginPatch patch;
  const int size = py::len(listOfTuples);
  patch.reserve(size);
  std::pair<int, float> parameter;
  for (int i = 0; i < size; ++i) {
    py::tuple tup;

    tup = listOfTuples[i].cast<py::tuple>();
    int index = int(tup[0].cast<float>());  // todo: go straight to int?
    float value = tup[1].cast<float>();
    parameter = std::make_pair(index, value);
    patch.push_back(parameter);
  }
  return patch;
}
}  // namespace customBoost
