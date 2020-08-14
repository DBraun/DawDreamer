#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include "pybind11/stl.h" // this lets std::vector<float> be a default arg

namespace py = pybind11;

typedef std::vector<std::pair<int, float>> PluginPatch;

namespace customBoost {
    //==========================================================================
    // Converts a C++ vector to a Python list. All following functions
    // are essentially cheap ripoffs from this one.
    // https://gist.github.com/octavifs/5362272
    template <class T>
    py::list vectorToList(std::vector<T> vector)
    {
        typename std::vector<T>::iterator iter;
        py::list list;
        for (iter = vector.begin(); iter != vector.end(); ++iter)
        {
            list.append(*iter);
        }
        return list;
    }

    template <class T>
    py::list
    arrayToList(std::array<T, 13> array);

    // Converts a C++ vector to a python list
    template <class T>
    py::list
    toPythonList(std::vector<T> vector);

    //==========================================================================
    // Converts a std::pair which is used as a parameter in C++
    // into a tuple with the respective types int and float for
    // use in Python.
    py::tuple
    parameterToTuple(std::pair<int, float> parameter);

    //==========================================================================
    // Converts a PluginPatch ( std::vector <std::pair <int, float>> )
    // to a Python list.
    py::list
    pluginPatchToListOfTuples(PluginPatch parameters);

    //==========================================================================
    PluginPatch
    listOfTuplesToPluginPatch(py::list listOfTuples);

    template< typename T >
    inline
    std::vector< T >
    to_std_vector(const py::object& iterable);
}
