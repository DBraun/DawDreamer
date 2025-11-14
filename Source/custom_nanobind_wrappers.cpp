#include "custom_nanobind_wrappers.h"

typedef std::vector<std::pair<int, float>> PluginPatch;

namespace customBoost
{

template <class T> nb::list arrayToList(std::array<T, 13> array)
{
    typename std::array<T, 13>::iterator iter;
    nb::list list;
    for (iter = array.begin(); iter != array.end(); ++iter)
    {
        list.append(*iter);
    }
    return list;
}

// Converts a C++ vector to a python list
template <class T> nb::list toPythonList(std::vector<T> vector)
{
    typename std::vector<T>::iterator iter;
    nb::list list;
    for (iter = vector.begin(); iter != vector.end(); ++iter)
    {
        list.append(*iter);
    }
    return list;
}

//==========================================================================
// Converts a std::pair which is used as a parameter in C++
// into a tuple with the respective types int and float for
// use in Python.
nb::tuple parameterToTuple(std::pair<int, float> parameter)
{
    nb::tuple parameterTuple;
    parameterTuple = nb::make_tuple(parameter.first, parameter.second);
    return parameterTuple;
}

//==========================================================================
// Converts a PluginPatch ( std::vector <std::pair <int, float>> )
// to a Python list.
nb::list pluginPatchToListOfTuples(PluginPatch parameters)
{
    std::vector<std::pair<int, float>>::iterator iter;
    nb::list list;
    for (iter = parameters.begin(); iter != parameters.end(); ++iter)
    {
        auto tup = parameterToTuple(*iter);
        list.append(tup);
    }
    return list;
}

//==========================================================================
PluginPatch listOfTuplesToPluginPatch(nb::list listOfTuples)
{
    PluginPatch patch;
    const int size = nb::len(listOfTuples);
    patch.reserve(size);
    std::pair<int, float> parameter;
    for (int i = 0; i < size; ++i)
    {
        nb::tuple tup;

        tup = nb::cast<nb::tuple>(listOfTuples[i]);
        int index = int(nb::cast<float>(tup[0])); // todo: go straight to int?
        float value = nb::cast<float>(tup[1]);
        parameter = std::make_pair(index, value);
        patch.push_back(parameter);
    }
    return patch;
}
} // namespace customBoost
