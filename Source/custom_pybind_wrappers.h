#pragma once

#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

namespace nb = nanobind;

typedef std::vector<std::pair<int, float>> PluginPatch;

namespace customBoost
{
//==========================================================================
// Converts a C++ vector to a Python list. All following functions
// are essentially cheap ripoffs from this one.
// https://gist.github.com/octavifs/5362272
template <class T> nb::list vectorToList(std::vector<T> vector)
{
    typename std::vector<T>::iterator iter;
    nb::list list;
    for (iter = vector.begin(); iter != vector.end(); ++iter)
    {
        list.append(*iter);
    }
    return list;
}

template <class T> nb::list arrayToList(std::array<T, 13> array);

// Converts a C++ vector to a python list
template <class T> nb::list toPythonList(std::vector<T> vector);

//==========================================================================
// Converts a std::pair which is used as a parameter in C++
// into a tuple with the respective types int and float for
// use in Python.
nb::tuple parameterToTuple(std::pair<int, float> parameter);

//==========================================================================
// Converts a PluginPatch ( std::vector <std::pair <int, float>> )
// to a Python list.
nb::list pluginPatchToListOfTuples(PluginPatch parameters);

//==========================================================================
PluginPatch listOfTuplesToPluginPatch(nb::list listOfTuples);

template <typename T> inline std::vector<T> to_std_vector(const nb::object& iterable);
} // namespace customBoost
