#ifdef BUILD_DAWDREAMER_FAUST
#include "FaustBoxAPI.h"

#ifdef WIN32
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <variant>
#include <windows.h>

// Find path to .dll */
// https://stackoverflow.com/a/57738892/12327461
HMODULE hMod;
std::wstring MyDLLPathFull;
std::wstring MyDLLDir;
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    hMod = hModule;
    const int BUFSIZE = 4096;
    wchar_t buffer[BUFSIZE];
    if (::GetModuleFileNameW(hMod, buffer, BUFSIZE - 1) <= 0)
    {
        return TRUE;
    }

    MyDLLPathFull = buffer;

    size_t found = MyDLLPathFull.find_last_of(L"/\\");
    MyDLLDir = MyDLLPathFull.substr(0, found);

    return TRUE;
}

#else

// this applies to both __APPLE__ and linux?
#include <filesystem>
#include <variant>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>

// https://stackoverflow.com/a/51993539/911207
const char* getMyDLLPath(void)
{
    Dl_info dl_info;
    dladdr((void*)getMyDLLPath, &dl_info);
    return (dl_info.dli_fname);
}
#endif

std::string getPathToFaustLibraries()
{
    // Get the path to the directory containing basics.lib, stdfaust.lib etc.

    try
    {
#ifdef WIN32
        const std::wstring ws_shareFaustDir = MyDLLDir + L"\\faustlibraries";
        // std::cerr << "MyDLLDir: ";
        // std::wcerr << MyDLLDir << L'\n';
        // convert const wchar_t to char
        // https://stackoverflow.com/a/4387335
        const wchar_t* wc_shareFaustDir = ws_shareFaustDir.c_str();
        // Count required buffer size (plus one for null-terminator).
        size_t size = (wcslen(wc_shareFaustDir) + 1) * sizeof(wchar_t);
        char* char_shareFaustDir = new char[size];
        std::wcstombs(char_shareFaustDir, wc_shareFaustDir, size);

        std::string p(char_shareFaustDir);

        delete[] char_shareFaustDir;
        return p;
#else
        // this applies to __APPLE__ and LINUX
        const char* myDLLPath = getMyDLLPath();
        // std::cerr << "myDLLPath: " << myDLLPath << std::endl;
        std::filesystem::path p = std::filesystem::path(myDLLPath);
        p = p.parent_path() / "faustlibraries";
        return p.string();
#endif
    }
    catch (...)
    {
        throw std::runtime_error("Error getting path to faustlibraries.");
    }
}

std::string getPathToArchitectureFiles()
{
    // Get the path to the directory containing jax/minimal.py, unity/unity.cpp
    // etc.

    try
    {
#ifdef WIN32
        const std::wstring ws_shareFaustDir = MyDLLDir + L"\\architecture";
        // std::cerr << "MyDLLDir: ";
        // std::wcerr << MyDLLDir << L'\n';
        // convert const wchar_t to char
        // https://stackoverflow.com/a/4387335
        const wchar_t* wc_shareFaustDir = ws_shareFaustDir.c_str();
        // Count required buffer size (plus one for null-terminator).
        size_t size = (wcslen(wc_shareFaustDir) + 1) * sizeof(wchar_t);
        char* char_shareFaustDir = new char[size];
        std::wcstombs(char_shareFaustDir, wc_shareFaustDir, size);

        std::string p(char_shareFaustDir);

        delete[] char_shareFaustDir;
        return p;
#else
        // this applies to __APPLE__ and LINUX
        const char* myDLLPath = getMyDLLPath();
        // std::cerr << "myDLLPath: " << myDLLPath << std::endl;
        std::filesystem::path p = std::filesystem::path(myDLLPath);
        p = p.parent_path() / "architecture";
        return p.string();
#endif
    }
    catch (...)
    {
        throw std::runtime_error("Error getting path to architecture.");
    }
}

using arg = nb::arg;
using kw_only = nb::kw_only;

void add_operation(nb::class_<BoxWrapper>& cls, const char* name, Box (*func)(Box, Box))
{
    cls.def(name, [func](const BoxWrapper& box, int other)
            { return BoxWrapper(func((BoxWrapper&)box, boxInt(other))); })
        .def(name, [func](const BoxWrapper& box, float other)
             { return BoxWrapper(func((BoxWrapper&)box, boxReal(other))); })
        .def(name, [func](const BoxWrapper& box1, BoxWrapper& box2)
             { return BoxWrapper(func((BoxWrapper&)box1, box2)); });
}

void add_unary_operation(nb::class_<BoxWrapper>& cls, const char* name, Box (*func)(Box))
{
    cls.def(name, [func](const BoxWrapper& box1) { return BoxWrapper(func((BoxWrapper&)box1)); });
}

nb::module_& create_bindings_for_faust_box(nb::module_& faust_module, nb::module_& box_module)
{
    using arg = nb::arg;
    using kw_only = nb::kw_only;

    box_module.doc() = R"pbdoc(
        The Faust Box API
        -----------------------------------------------------------

		For reference: https://faustdoc.grame.fr/tutorials/box-api/

        .. currentmodule:: dawdreamer.faust.box

        .. autosummary::
           :toctree: _generate
    )pbdoc";

    nb::class_<BoxWrapper> cls(box_module, "Box");

    cls.def_prop_ro("inputs", &BoxWrapper::getInputs, "Get the box's number of inputs.");
    cls.def_prop_ro("outputs", &BoxWrapper::getOutputs, "Get the box's number of outputs.");
    cls.def_prop_ro("valid", &BoxWrapper::getValid, "Get a bool for whether the box is valid.");

    add_operation(cls, "__add__", static_cast<Box (*)(Box, Box)>(boxAdd));
    add_operation(cls, "__radd__", static_cast<Box (*)(Box, Box)>(boxAdd));
    add_operation(cls, "__sub__", static_cast<Box (*)(Box, Box)>(boxSub));
    add_operation(cls, "__rsub__", static_cast<Box (*)(Box, Box)>(boxSub));
    add_operation(cls, "__mul__", static_cast<Box (*)(Box, Box)>(boxMul));
    add_operation(cls, "__rmul__", static_cast<Box (*)(Box, Box)>(boxMul));
    add_operation(cls, "__truediv__", static_cast<Box (*)(Box, Box)>(boxDiv));
    add_operation(cls, "__rtruediv__", static_cast<Box (*)(Box, Box)>(boxDiv));
    add_operation(cls, "__mod__", static_cast<Box (*)(Box, Box)>(boxFmod));

    add_operation(cls, "__lt__", static_cast<Box (*)(Box, Box)>(boxLT));
    add_operation(cls, "__le__", static_cast<Box (*)(Box, Box)>(boxLE));
    add_operation(cls, "__gt__", static_cast<Box (*)(Box, Box)>(boxGT));
    add_operation(cls, "__ge__", static_cast<Box (*)(Box, Box)>(boxGE));
    add_operation(cls, "__eq__", static_cast<Box (*)(Box, Box)>(boxEQ));
    add_operation(cls, "__ne__", static_cast<Box (*)(Box, Box)>(boxNE));

    add_operation(cls, "__pow__", static_cast<Box (*)(Box, Box)>(boxPow));

    add_operation(cls, "__lshift__", static_cast<Box (*)(Box, Box)>(boxLeftShift));
    add_operation(cls, "__rshift__", static_cast<Box (*)(Box, Box)>(boxARightShift));

    add_operation(cls, "__and__", static_cast<Box (*)(Box, Box)>(boxAND));
    add_operation(cls, "__or__", static_cast<Box (*)(Box, Box)>(boxOR));
    add_operation(cls, "__xor__", static_cast<Box (*)(Box, Box)>(boxXOR));

    auto floordiv_func = [](Box box1, Box other) { return boxFloor(boxDiv(box1, other)); };

    add_operation(cls, "__floordiv__", static_cast<Box (*)(Box, Box)>(floordiv_func));

    add_unary_operation(cls, "__abs__", static_cast<Box (*)(Box)>(boxAbs));
    add_unary_operation(cls, "__int__", static_cast<Box (*)(Box)>(boxIntCast));
    add_unary_operation(cls, "__float__", static_cast<Box (*)(Box)>(boxFloatCast));
    add_unary_operation(cls, "__floor__", static_cast<Box (*)(Box)>(boxFloor));

    add_unary_operation(cls, "__ceil__", static_cast<Box (*)(Box)>(boxCeil));

    add_unary_operation(cls, "__ceil__", static_cast<Box (*)(Box)>(boxCeil));

    cls.def(nb::init<float>(), arg("val"), "Init with a float")
        .def(nb::init<int>(), arg("val"), "Init with an int")
        .def("__repr__",
             [](const BoxWrapper& b)
             {
                 try
                 {
                     return tree2str((BoxWrapper&)b);
                 }
                 catch (faustexception& e)
                 {
                     return "UNKNOWN";
                 }
             })
        .def(
            "extract_name", [](const BoxWrapper& b) { return extractName((BoxWrapper&)b); },
            "Return the name from a label.")
        .def(
            "print_str", [](const BoxWrapper& box1, const bool shared, const int max_size)
            { return printBox((BoxWrapper&)box1, shared, max_size); }, arg("shared"),
            arg("max_size"),
            "Return the box as a string. The argument `shared` is whether "
            "identical sub boxes are printed as identifiers. The argument "
            "`max_size` is the maximum number of characters to be printed "
            "(possibly needed for big expressions in non shared mode).")

        .def("__neg__", [](const BoxWrapper& box1)
             { return BoxWrapper(boxSub(boxReal(0), (BoxWrapper&)box1)); });

    box_module
        .def(
            "getUserData",
            [](BoxWrapper& box1)
            {
                void* data = (void*)getUserData(box1);
                return data ? tree2str(box1) : "";
            },
            arg("box"))
        .def(
            "getDefNameProperty",
            [](const BoxWrapper& b)
            {
                Box id = nullptr;
                bool res = getDefNameProperty((BoxWrapper&)b, id);
                return nb::make_tuple(res, BoxWrapper(id));
            },
            arg("box"),
            "Return a tuple of (bool, Box). If the bool is True, the input box "
            "was a definition of the second item in the tuple. Otherwise, the "
            "second item should not be used.")
        .def(
            "boxInt", [](int val) { return BoxWrapper(boxInt(val)); }, arg("val"))
        .def(
            "boxReal", [](double val) { return BoxWrapper(boxReal(val)); }, arg("val"))
        .def("boxWire", []() { return BoxWrapper(boxWire()); })
        .def("boxCut", []() { return BoxWrapper(boxCut()); })

        .def(
            "boxSeq", [](BoxWrapper& box1, BoxWrapper& box2)
            { return BoxWrapper(boxSeq(box1, box2)); }, arg("box1"), arg("box2"),
            "The sequential composition of two blocks (e.g., A:B) expects: "
            "outputs(A)=inputs(B)")

        .def(
            "boxPar", [](BoxWrapper& box1, BoxWrapper& box2)
            { return BoxWrapper(boxPar(box1, box2)); }, arg("box1"), arg("box2"),
            "The parallel composition of two blocks (e.g., A,B). It places the "
            "two block-diagrams one on top of the other, without connections.")
        .def(
            "boxPar3", [](BoxWrapper& box1, BoxWrapper& box2, BoxWrapper& box3)
            { return BoxWrapper(boxPar3(box1, box2, box3)); }, arg("box1"), arg("box2"),
            arg("box3"), "The parallel composition of three blocks (e.g., A,B,C).")
        .def(
            "boxPar4", [](BoxWrapper& box1, BoxWrapper& box2, BoxWrapper& box3, BoxWrapper& box4)
            { return BoxWrapper(boxPar4(box1, box2, box3, box4)); }, arg("box1"), arg("box2"),
            arg("box3"), arg("box4"), "The parallel composition of four blocks (e.g., A,B,C,D).")
        .def(
            "boxPar5",
            [](BoxWrapper& box1, BoxWrapper& box2, BoxWrapper& box3, BoxWrapper& box4,
               BoxWrapper& box5) { return BoxWrapper(boxPar5(box1, box2, box3, box4, box5)); },
            arg("box1"), arg("box2"), arg("box3"), arg("box4"), arg("box5"),
            "The parallel composition of five blocks (e.g., A,B,C,D,E).")

        .def(
            "boxSplit", [](BoxWrapper& box1, BoxWrapper& box2)
            { return BoxWrapper(boxSplit(box1, box2)); }, arg("box1"), arg("box2"),
            "The split composition (e.g., A<:B) operator is used to distribute "
            "the outputs of A to the inputs of B. The number of inputs of B must "
            "be a multiple of the number of outputs of A: outputs(A).k=inputs(B)")
        .def(
            "boxMerge", [](BoxWrapper& box1, BoxWrapper& box2)
            { return BoxWrapper(boxMerge(box1, box2)); }, arg("box1"), arg("box2"),
            "The merge composition (e.g., A:>B) is the dual of the split "
            "composition. The number of outputs of A must be a multiple of the "
            "number of inputs of B: outputs(A)=k.inputs(B)")

        .def(
            "boxRoute", [](BoxWrapper& box1, BoxWrapper& box2, BoxWrapper& box3)
            { return BoxWrapper(boxRoute(box1, box2, box3)); }, arg("box_n"), arg("box_m"),
            arg("box_r"),
            "The route primitive facilitates the routing of signals in Faust. It "
            "has the following syntax: route(A,B,a,b,c,d,...) or "
            "route(A,B,(a,b),(c,d),...)\n* @param n -  the number of input "
            "signals\n* @param m -  the number of output signals\n* @param r - "
            "the routing description, a 'par' expression of a,b / (a,b) "
            "input/output pairs")

        .def(
            "boxDelay",
            [](BoxWrapper box1, BoxWrapper box2) { return BoxWrapper(boxDelay(box1, box2)); },
            arg("box1"), arg("box2"), "Create a delayed box.")
        .def(
            "boxDelay", []() { return BoxWrapper(boxDelay()); }, "Create a delayed box.")

        .def(
            "boxIntCast", [](BoxWrapper& box1) { return BoxWrapper(boxIntCast(box1)); },
            arg("box1"))
        .def("boxIntCast", []() { return BoxWrapper(boxIntCast()); })

        .def(
            "boxFloatCast", [](BoxWrapper& box1) { return BoxWrapper(boxFloatCast(box1)); },
            arg("box1"))
        .def("boxFloatCast", []() { return BoxWrapper(boxFloatCast()); })

        .def(
            "boxReadOnlyTable", [](BoxWrapper& n, BoxWrapper& init, BoxWrapper& ridx)
            { return BoxWrapper(boxReadOnlyTable(boxIntCast(n), init, boxIntCast(ridx))); },
            arg("n"), arg("init"), arg("ridx"))
        .def("boxReadOnlyTable", []() { return BoxWrapper(boxReadOnlyTable()); })

        .def(
            "boxWriteReadTable",
            [](BoxWrapper& n, BoxWrapper& init, BoxWrapper& widx, BoxWrapper& wsig,
               BoxWrapper& ridx)
            {
                return BoxWrapper(boxWriteReadTable(boxIntCast(n), init, boxIntCast(widx),
                                                    boxIntCast(wsig), boxIntCast(ridx)));
            },
            arg("n"), arg("init"), arg("widx"), arg("wsig"), arg("ridx"))
        .def("boxWriteReadTable", []() { return BoxWrapper(boxWriteReadTable()); })

        .def(
            "boxWaveform",
            [](std::vector<float> vals)
            {
                tvec waveform;
                for (auto& val : vals)
                {
                    waveform.push_back(boxReal(val));
                }
                return BoxWrapper(boxWaveform(waveform));
            },
            arg("vals"), "Create a waveform from a list of values.")
        .def(
            "boxSoundfile", [](const std::string& label, BoxWrapper& chan)
            { return BoxWrapper(boxSoundfile(label, chan)); }, arg("filepath"), arg("chan"),
            "Create a soundfile from a path and channel count.")
        .def(
            "boxSoundfile",
            [](const std::string& label, BoxWrapper& chan, BoxWrapper& part, BoxWrapper& rdx)
            { return BoxWrapper(boxSoundfile(label, chan, part, rdx)); }, arg("filepath"),
            arg("chan"), arg("part"), arg("ridx"),
            "Create a soundfile from a path and channel count. The third "
            "argument for the `part` is in the [0..255] range to select "
            "a given sound number, a constant numerical expression. The fourth "
            "argument for the `ridx` is the read index (an integer "
            "between 0 and the selected sound length).")

        .def(
            "boxSelect2", [](BoxWrapper& selector, BoxWrapper& box1, BoxWrapper& box2)
            { return BoxWrapper(boxSelect2(selector, box1, box2)); }, arg("selector"), arg("box1"),
            arg("box2"))
        .def("boxSelect2", []() { return BoxWrapper(boxSelect2()); })

        .def(
            "boxSelect3",
            [](BoxWrapper& selector, BoxWrapper& box1, BoxWrapper& box2, BoxWrapper& box3)
            { return BoxWrapper(boxSelect3(selector, box1, box2, box3)); }, arg("selector"),
            arg("box1"), arg("box2"), arg("box3"))
        .def("boxSelect3", []() { return BoxWrapper(boxSelect3()); })

        .def(
            "boxFConst", [](SType type, const std::string& name, const std::string& file)
            { return BoxWrapper(boxFConst(type, name, file)); }, arg("type"), arg("name"),
            arg("file"))
        .def(
            "boxFVar", [](SType type, const std::string& name, const std::string& file)
            { return BoxWrapper(boxFVar(type, name, file)); }, arg("type"), arg("name"),
            arg("file"))
        .def(
            "boxFFun",
            [](SType type, nvec names, svec atypes, const std::string& incfile,
               const std::string& libfile)
            { return BoxWrapper(boxFFun(type, names, atypes, incfile, libfile)); },
            arg("type"), arg("names"), arg("arg_types"), arg("inc_file"), arg("lib_file"),
            "Return a foreign function box.")

        .def(
            "boxBinOp", [](SOperator op, BoxWrapper& x, BoxWrapper& y)
            { return BoxWrapper(boxBinOp(op, x, y)); }, arg("op"), arg("x"), arg("y"))
        .def(
            "boxBinOp", [](SOperator op) { return BoxWrapper(boxBinOp(op)); }, arg("op"))

        .def(
            "boxAdd", [](BoxWrapper box1, BoxWrapper box2)
            { return BoxWrapper(boxAdd(box1, box2)); }, arg("box1"), arg("box2"))
        .def("boxAdd", []() { return BoxWrapper(boxAdd()); })
        .def(
            "boxSub", [](BoxWrapper box1, BoxWrapper box2)
            { return BoxWrapper(boxSub(box1, box2)); }, arg("box1"), arg("box2"))
        .def("boxSub", []() { return BoxWrapper(boxSub()); })
        .def(
            "boxMul", [](BoxWrapper box1, BoxWrapper box2)
            { return BoxWrapper(boxMul(box1, box2)); }, arg("box1"), arg("box2"))
        .def("boxMul", []() { return BoxWrapper(boxMul()); })
        .def(
            "boxDiv", [](BoxWrapper box1, BoxWrapper box2)
            { return BoxWrapper(boxDiv(box1, box2)); }, arg("box1"), arg("box2"))
        .def("boxDiv", []() { return BoxWrapper(boxDiv()); })
        .def(
            "boxRem", [](BoxWrapper box1, BoxWrapper box2)
            { return BoxWrapper(boxRem(box1, box2)); }, arg("box1"), arg("box2"))
        .def("boxRem", []() { return BoxWrapper(boxRem()); })

        .def(
            "boxLeftShift", [](BoxWrapper box1, BoxWrapper box2)
            { return BoxWrapper(boxLeftShift(box1, box2)); }, arg("box1"), arg("box2"))
        .def("boxLeftShift", []() { return BoxWrapper(boxLeftShift()); })
        .def(
            "boxLRightShift", [](BoxWrapper box1, BoxWrapper box2)
            { return BoxWrapper(boxLRightShift(box1, box2)); }, arg("box1"), arg("box2"))
        .def("boxLRightShift", []() { return BoxWrapper(boxLRightShift()); })
        .def(
            "boxARightShift", [](BoxWrapper box1, BoxWrapper box2)
            { return BoxWrapper(boxARightShift(box1, box2)); }, arg("box1"), arg("box2"))
        .def("boxARightShift", []() { return BoxWrapper(boxARightShift()); })

        .def(
            "boxGT", [](BoxWrapper box1, BoxWrapper box2) { return BoxWrapper(boxGT(box1, box2)); },
            arg("box1"), arg("box2"))
        .def("boxGT", []() { return BoxWrapper(boxGT()); })
        .def(
            "boxLT", [](BoxWrapper box1, BoxWrapper box2) { return BoxWrapper(boxLT(box1, box2)); },
            arg("box1"), arg("box2"))
        .def("boxLT", []() { return BoxWrapper(boxLT()); })
        .def(
            "boxGE", [](BoxWrapper box1, BoxWrapper box2) { return BoxWrapper(boxGE(box1, box2)); },
            arg("box1"), arg("box2"))
        .def("boxGE", []() { return BoxWrapper(boxGE()); })
        .def(
            "boxLE", [](BoxWrapper box1, BoxWrapper box2) { return BoxWrapper(boxLE(box1, box2)); },
            arg("box1"), arg("box2"))
        .def("boxLE", []() { return BoxWrapper(boxLE()); })
        .def(
            "boxEQ", [](BoxWrapper box1, BoxWrapper box2) { return BoxWrapper(boxEQ(box1, box2)); },
            arg("box1"), arg("box2"))
        .def("boxEQ", []() { return BoxWrapper(boxEQ()); })
        .def(
            "boxNE", [](BoxWrapper box1, BoxWrapper box2) { return BoxWrapper(boxNE(box1, box2)); },
            arg("box1"), arg("box2"))
        .def("boxNE", []() { return BoxWrapper(boxNE()); })

        .def(
            "boxAND", [](BoxWrapper box1, BoxWrapper box2)
            { return BoxWrapper(boxAND(box1, box2)); }, arg("box1"), arg("box2"))
        .def("boxAND", []() { return BoxWrapper(boxAND()); })
        .def(
            "boxOR", [](BoxWrapper box1, BoxWrapper box2) { return BoxWrapper(boxOR(box1, box2)); },
            arg("box1"), arg("box2"))
        .def("boxOR", []() { return BoxWrapper(boxOR()); })
        .def(
            "boxXOR", [](BoxWrapper box1, BoxWrapper box2)
            { return BoxWrapper(boxXOR(box1, box2)); }, arg("box1"), arg("box2"))
        .def("boxXOR", []() { return BoxWrapper(boxXOR()); })

        .def(
            "boxAbs", [](BoxWrapper& box1) { return BoxWrapper(boxAbs(box1)); }, arg("box1"))
        .def("boxAbs", []() { return BoxWrapper(boxAbs()); })
        .def(
            "boxAcos", [](BoxWrapper& box1) { return BoxWrapper(boxAcos(box1)); }, arg("box1"))
        .def("boxAcos", []() { return BoxWrapper(boxAcos()); })
        .def(
            "boxTan", [](BoxWrapper& box1) { return BoxWrapper(boxTan(box1)); }, arg("box1"))
        .def("boxTan", []() { return BoxWrapper(boxTan()); })
        .def(
            "boxSqrt", [](BoxWrapper& box1) { return BoxWrapper(boxSqrt(box1)); }, arg("box1"))
        .def("boxSqrt", []() { return BoxWrapper(boxSqrt()); })
        .def(
            "boxSin", [](BoxWrapper& box1) { return BoxWrapper(boxSin(box1)); }, arg("box1"))
        .def("boxSin", []() { return BoxWrapper(boxSin()); })
        .def(
            "boxRint", [](BoxWrapper& box1) { return BoxWrapper(boxRint(box1)); }, arg("box1"))
        .def("boxRint", []() { return BoxWrapper(boxRint()); })
        .def(
            "boxRound", [](BoxWrapper& box1) { return BoxWrapper(boxRound(box1)); }, arg("box1"))
        .def("boxRound", []() { return BoxWrapper(boxRound()); })
        .def(
            "boxLog", [](BoxWrapper& box1) { return BoxWrapper(boxLog(box1)); }, arg("box1"))
        .def("boxLog", []() { return BoxWrapper(boxLog()); })
        .def(
            "boxLog10", [](BoxWrapper& box1) { return BoxWrapper(boxLog10(box1)); }, arg("box1"))
        .def("boxLog10", []() { return BoxWrapper(boxLog10()); })
        .def(
            "boxFloor", [](BoxWrapper& box1) { return BoxWrapper(boxFloor(box1)); }, arg("box1"))
        .def("boxFloor", []() { return BoxWrapper(boxFloor()); })
        .def(
            "boxExp", [](BoxWrapper& box1) { return BoxWrapper(boxExp(box1)); }, arg("box1"))
        .def("boxExp", []() { return BoxWrapper(boxExp()); })
        .def(
            "boxExp10", [](BoxWrapper& box1) { return BoxWrapper(boxExp10(box1)); }, arg("box1"))
        .def("boxExp10", []() { return BoxWrapper(boxExp10()); })
        .def(
            "boxCos", [](BoxWrapper& box1) { return BoxWrapper(boxCos(box1)); }, arg("box1"))
        .def("boxCos", []() { return BoxWrapper(boxCos()); })
        .def(
            "boxCeil", [](BoxWrapper& box1) { return BoxWrapper(boxCeil(box1)); }, arg("box1"))
        .def("boxCeil", []() { return BoxWrapper(boxCeil()); })
        .def(
            "boxAtan", [](BoxWrapper& box1) { return BoxWrapper(boxAtan(box1)); }, arg("box1"))
        .def("boxAtan", []() { return BoxWrapper(boxAtan()); })
        .def(
            "boxAsin", [](BoxWrapper& box1) { return BoxWrapper(boxAsin(box1)); }, arg("box1"))
        .def("boxAsin", []() { return BoxWrapper(boxAsin()); })

        .def(
            "boxRemainder", [](BoxWrapper box1, BoxWrapper box2)
            { return BoxWrapper(boxRemainder(box1, box2)); }, arg("box1"), arg("box2"))
        .def("boxRemainder", []() { return BoxWrapper(boxRemainder()); })
        .def(
            "boxPow", [](BoxWrapper box1, BoxWrapper box2)
            { return BoxWrapper(boxPow(box1, box2)); }, arg("box1"), arg("box2"))
        .def("boxPow", []() { return BoxWrapper(boxPow()); })
        .def(
            "boxMin", [](BoxWrapper box1, BoxWrapper box2)
            { return BoxWrapper(boxMin(box1, box2)); }, arg("box1"), arg("box2"))
        .def("boxMin", []() { return BoxWrapper(boxMin()); })
        .def(
            "boxMax", [](BoxWrapper box1, BoxWrapper box2)
            { return BoxWrapper(boxMax(box1, box2)); }, arg("box1"), arg("box2"))
        .def("boxMax", []() { return BoxWrapper(boxMax()); })
        .def(
            "boxFmod", [](BoxWrapper box1, BoxWrapper box2)
            { return BoxWrapper(boxFmod(box1, box2)); }, arg("box1"), arg("box2"))
        .def("boxFmod", []() { return BoxWrapper(boxFmod()); })
        .def(
            "boxAtan2", [](BoxWrapper box1, BoxWrapper box2)
            { return BoxWrapper(boxAtan2(box1, box2)); }, arg("box1"), arg("box2"))
        .def("boxAtan2", []() { return BoxWrapper(boxAtan2()); })

        .def(
            "boxRec", [](BoxWrapper& box1, BoxWrapper& box2)
            { return BoxWrapper(boxRec(box1, box2)); }, arg("box1"), arg("box2"),
            "The recursive composition (e.g., A~B) is used to create cycles in "
            "the block-diagram in order to express recursive computations. It is "
            "the most complex operation in terms of connections: "
            "outputs(A)≥inputs(B) and inputs(A)≥outputs(B)")

        .def(
            "boxButton", [](const std::string& label) { return BoxWrapper(boxButton(label)); },
            arg("label"))
        .def(
            "boxCheckbox", [](const std::string& label) { return BoxWrapper(boxCheckbox(label)); },
            arg("label"))

        .def(
            "boxVSlider",
            [](const std::string& label, BoxWrapper& boxInit, BoxWrapper& boxMin,
               BoxWrapper& boxMax, BoxWrapper& boxStep)
            { return BoxWrapper(boxVSlider(label, boxInit, boxMin, boxMax, boxStep)); },
            arg("label"), arg("init"), arg("min"), arg("max"), arg("step"),
            "Create a vertical slider.")
        .def(
            "boxHSlider",
            [](const std::string& label, BoxWrapper& boxInit, BoxWrapper& boxMin,
               BoxWrapper& boxMax, BoxWrapper& boxStep)
            { return BoxWrapper(boxHSlider(label, boxInit, boxMin, boxMax, boxStep)); },
            arg("label"), arg("init"), arg("min"), arg("max"), arg("step"),
            "Create a horizontal slider.")

        .def(
            "boxNumEntry",
            [](const std::string& label, BoxWrapper& boxInit, BoxWrapper& boxMin,
               BoxWrapper& boxMax, BoxWrapper& boxStep)
            { return BoxWrapper(boxNumEntry(label, boxInit, boxMin, boxMax, boxStep)); },
            arg("label"), arg("init"), arg("min"), arg("max"), arg("step"),
            "Create a numerical entry.")

        .def(
            "boxHGroup", [](const std::string& label, BoxWrapper& box1)
            { return BoxWrapper(boxHGroup(label, box1)); }, arg("label"), arg("box"),
            "Create an hgroup.")

        .def(
            "boxVGroup", [](const std::string& label, BoxWrapper& box1)
            { return BoxWrapper(boxVGroup(label, box1)); }, arg("label"), arg("box"),
            "Create a vgroup.")

        .def(
            "boxTGroup", [](const std::string& label, BoxWrapper& box1)
            { return BoxWrapper(boxTGroup(label, box1)); }, arg("label"), arg("box"),
            "Create a tgroup.")

        .def(
            "boxVBargraph",
            [](const std::string& label, BoxWrapper& boxMin, BoxWrapper& boxMax, BoxWrapper& box)
            { return BoxWrapper(boxVBargraph(label, boxMin, boxMax, box)); }, arg("label"),
            arg("min"), arg("max"), arg("step"))

        .def(
            "boxHBargraph",
            [](const std::string& label, BoxWrapper& boxMin, BoxWrapper& boxMax, BoxWrapper& box)
            { return BoxWrapper(boxHBargraph(label, boxMin, boxMax, box)); }, arg("label"),
            arg("min"), arg("max"), arg("step"))

        .def(
            "boxAttach", [](BoxWrapper& box1, BoxWrapper& box2)
            { return BoxWrapper(boxAttach(box1, box2)); }, arg("box1"), arg("box2"))
        .def("boxAttach", []() { return BoxWrapper(boxAttach()); })
        .def(
            "boxSampleRate",
            []()
            {
                return BoxWrapper(boxMin(
                    boxReal(192000.0),
                    boxMax(boxReal(1.0), boxFConst(SType::kSInt, "fSamplingFreq", "<math.h>"))));
            },
            "Return a box representing the constant sample rate, such as 44100.")
        .def(
            "boxBufferSize",
            []() { return BoxWrapper(boxFVar(SType::kSInt, "count", "<math.h>")); },
            "Return a box representing the buffer size (also known as block "
            "size), such as 1, 2, 4, 8, etc.")
        .def(
            "boxFromDSP",
            [](const std::string& dsp_content)
            {
                int inputs = 0;
                int outputs = 0;
                std::string error_msg = "";
                const std::string dsp_content2 =
                    std::string("import(\"stdfaust.lib\");\n") + dsp_content;

                int argc = 0;
                const char* argv[512];

                auto pathToFaustLibraries = getPathToFaustLibraries();
                if (pathToFaustLibraries.empty())
                {
                    throw std::runtime_error("Unable to load Faust Libraries.");
                }

                argv[argc++] = "-I";
                argv[argc++] = strdup(pathToFaustLibraries.c_str());

                Box box = DSPToBoxes("dawdreamer", dsp_content2, argc, argv, &inputs, &outputs,
                                     error_msg);

                // Clean up strdup'd strings (skip argv[0] which is "-I" literal)
                for (int i = 1; i < argc; i++)
                {
                    free((void*)argv[i]);
                }

                if (!error_msg.empty())
                {
                    throw std::runtime_error(error_msg);
                }

                return BoxWrapper(box, inputs, outputs);
            },
            arg("dsp_code"), "Convert Faust DSP code to a Box.")
        .def(
            "boxFromDSP",
            [](const std::string& dsp_content, const std::vector<std::string>& in_argv)
            {
                int inputs = 0;
                int outputs = 0;
                std::string error_msg = "";
                const std::string dsp_content2 =
                    std::string("import(\"stdfaust.lib\");\n") + dsp_content;

                int argc = 0;
                const char* argv[512];

                auto pathToFaustLibraries = getPathToFaustLibraries();
                if (pathToFaustLibraries.empty())
                {
                    throw std::runtime_error("Unable to load Faust Libraries.");
                }

                argv[argc++] = "-I";
                argv[argc++] = strdup(pathToFaustLibraries.c_str());

                for (auto v : in_argv)
                {
                    argv[argc++] = strdup(v.c_str());
                }

                Box box = DSPToBoxes("dawdreamer", dsp_content2, argc, argv, &inputs, &outputs,
                                     error_msg);

                // Clean up strdup'd strings (skip argv[0] which is "-I" literal)
                for (int i = 1; i < argc; i++)
                {
                    free((void*)argv[i]);
                }

                if (!error_msg.empty())
                {
                    throw std::runtime_error(error_msg);
                }

                return BoxWrapper(box, inputs, outputs);
            },
            arg("dsp_code"), arg("argv"),
            "Convert Faust DSP code to a Box. The second argument `argv` is a list "
            "of strings to send to a Faust command line.")

        .def(
            "isBoxNil", [](BoxWrapper& b) { return isNil(b); }, arg("box"))

        .def(
            "isBoxAbstr",
            [](BoxWrapper& b)
            {
                Box x = nullptr, y = nullptr;
                bool res = isBoxAbstr(b, x, y);
                return nb::make_tuple(res, BoxWrapper(x), BoxWrapper(y));
            },
            arg("box"))

        .def(
            "isBoxAccess",
            [](BoxWrapper& box)
            {
                Box b2 = nullptr, b3 = nullptr;
                bool res = isBoxAccess(box, b2, b3);
                return nb::make_tuple(res, BoxWrapper(b2), BoxWrapper(b3));
            },
            arg("box"))

        .def(
            "isBoxAppl",
            [](BoxWrapper& box)
            {
                Box b2 = nullptr, b3 = nullptr;
                bool res = isBoxAppl(box, b2, b3);
                return nb::make_tuple(res, BoxWrapper(b2), BoxWrapper(b3));
            },
            arg("box_t"))

        .def(
            "isBoxButton",
            [](BoxWrapper& b)
            {
                Box label = nullptr;
                bool res = isBoxButton(b, label);
                return nb::make_tuple(res, BoxWrapper(label));
            },
            arg("box"))

        .def(
            "isBoxCase",
            [](BoxWrapper& b)
            {
                Box rules = nullptr;
                bool res = isBoxCase(b, rules);
                return nb::make_tuple(res, BoxWrapper(rules));
            },
            arg("box"))

        .def(
            "isBoxCheckbox",
            [](BoxWrapper& b)
            {
                Box label = nullptr;
                bool res = isBoxCheckbox(b, label);
                return nb::make_tuple(res, BoxWrapper(label));
            },
            arg("box"))

        .def(
            "isBoxComponent",
            [](BoxWrapper& b)
            {
                Box filename = nullptr;
                bool res = isBoxComponent(b, filename);
                return nb::make_tuple(res, BoxWrapper(filename));
            },
            arg("box"))

        .def(
            "isBoxCut", [](BoxWrapper& b) { return isBoxCut(b); }, arg("box"))

        .def(
            "isBoxEnvironment", [](BoxWrapper& b) { return isBoxEnvironment(b); }, arg("box"))

        .def(
            "isBoxError", [](BoxWrapper& b) { return isBoxError(b); }, arg("box"))

        .def(
            "isBoxFConst",
            [](BoxWrapper& b)
            {
                Box type = nullptr, name = nullptr, file = nullptr;
                bool res = isBoxFConst(b, type, name, file);
                return nb::make_tuple(res, BoxWrapper(type), BoxWrapper(name), BoxWrapper(file));
            },
            arg("box"))

        .def(
            "isBoxFFun",
            [](BoxWrapper& b)
            {
                Box ffun = nullptr;
                bool res = isBoxFFun(b, ffun);
                return nb::make_tuple(res, BoxWrapper(ffun));
            },
            arg("box"))

        .def(
            "isBoxFVar",
            [](BoxWrapper& b)
            {
                Box type = nullptr, name = nullptr, file = nullptr;
                bool res = isBoxFVar(b, type, name, file);
                return nb::make_tuple(res, BoxWrapper(type), BoxWrapper(name), BoxWrapper(file));
            },
            arg("box"))

        .def(
            "isBoxHBargraph",
            [](BoxWrapper& b)
            {
                Box label = nullptr, a_min = nullptr, a_max = nullptr;
                bool res = isBoxHBargraph(b, label, a_min, a_max);
                return nb::make_tuple(res, BoxWrapper(label), BoxWrapper(a_min), BoxWrapper(a_max));
            },
            arg("box"))

        .def(
            "isBoxHGroup",
            [](BoxWrapper& b)
            {
                Box label = nullptr, x = nullptr;
                bool res = isBoxHGroup(b, label, x);
                return nb::make_tuple(res, BoxWrapper(label), BoxWrapper(x));
            },
            arg("box"))

        .def(
            "isBoxHSlider",
            [](BoxWrapper& b)
            {
                Box label = nullptr, init = nullptr, a_min = nullptr, a_max = nullptr,
                    step = nullptr;
                bool res = isBoxHSlider(b, label, init, a_min, a_max, step);
                return nb::make_tuple(res, BoxWrapper(label), BoxWrapper(init), BoxWrapper(a_min),
                                      BoxWrapper(a_max), BoxWrapper(step));
            },
            arg("box"))

        .def(
            "isBoxIdent",
            [](BoxWrapper& b)
            {
                const char* str = nullptr;
                bool res = isBoxIdent(b, &str);
                std::string s = std::string(res ? str : "");
                return nb::make_tuple(res, s);
            },
            arg("box"))

        .def(
            "isBoxInputs",
            [](BoxWrapper& b)
            {
                Box x = nullptr;
                bool res = isBoxInputs(b, x);
                return nb::make_tuple(res, BoxWrapper(x));
            },
            arg("box"))

        .def(
            "isBoxInt",
            [](BoxWrapper& b)
            {
                int i = 0;
                bool res = isBoxInt(b, &i);
                return nb::make_tuple(res, i);
            },
            arg("box"))

        .def(
            "isBoxIPar",
            [](BoxWrapper& b)
            {
                Box x = nullptr, y = nullptr, z = nullptr;
                bool res = isBoxIPar(b, x, y, z);
                return nb::make_tuple(res, BoxWrapper(x), BoxWrapper(y), BoxWrapper(z));
            },
            arg("box"))

        .def(
            "isBoxIProd",
            [](BoxWrapper& b)
            {
                Box x = nullptr, y = nullptr, z = nullptr;
                bool res = isBoxIProd(b, x, y, z);
                return nb::make_tuple(res, BoxWrapper(x), BoxWrapper(y), BoxWrapper(z));
            },
            arg("box"))

        .def(
            "isBoxISeq",
            [](BoxWrapper& b)
            {
                Box x = nullptr, y = nullptr, z = nullptr;
                bool res = isBoxISeq(b, x, y, z);
                return nb::make_tuple(res, BoxWrapper(x), BoxWrapper(y), BoxWrapper(z));
            },
            arg("box"))

        .def(
            "isBoxISum",
            [](BoxWrapper& b)
            {
                Box x = nullptr, y = nullptr, z = nullptr;
                bool res = isBoxISum(b, x, y, z);
                return nb::make_tuple(res, BoxWrapper(x), BoxWrapper(y), BoxWrapper(z));
            },
            arg("box"))

        .def(
            "isBoxLibrary",
            [](BoxWrapper& b)
            {
                Box filename = nullptr;
                bool res = isBoxLibrary(b, filename);
                return nb::make_tuple(res, BoxWrapper(filename));
            },
            arg("box"))

        .def(
            "isBoxMerge",
            [](BoxWrapper& b)
            {
                Box x = nullptr, y = nullptr;
                bool res = isBoxMerge(b, x, y);
                return nb::make_tuple(res, BoxWrapper(x), BoxWrapper(y));
            },
            arg("box"))

        .def(
            "isBoxMetadata",
            [](BoxWrapper& b)
            {
                Box x = nullptr, y = nullptr;
                bool res = isBoxMetadata(b, x, y);
                return nb::make_tuple(res, BoxWrapper(x), BoxWrapper(y));
            },
            arg("box"))

        .def(
            "isBoxNumEntry",
            [](BoxWrapper& b)
            {
                Box label = nullptr, init = nullptr, a_min = nullptr, a_max = nullptr,
                    step = nullptr;
                bool res = isBoxNumEntry(b, label, init, a_min, a_max, step);
                return nb::make_tuple(res, BoxWrapper(label), BoxWrapper(init), BoxWrapper(a_min),
                                      BoxWrapper(a_max), BoxWrapper(step));
            },
            arg("box"))

        .def(
            "isBoxOutputs",
            [](BoxWrapper& b)
            {
                Box x = nullptr;
                bool res = isBoxOutputs(b, x);
                return nb::make_tuple(res, BoxWrapper(x));
            },
            arg("box"))

        .def(
            "isBoxPar",
            [](BoxWrapper& b)
            {
                Box x = nullptr, y = nullptr;
                bool res = isBoxPar(b, x, y);
                return nb::make_tuple(res, BoxWrapper(x), BoxWrapper(y));
            },
            arg("box"))

        .def(
            "isBoxPrim0",
            [](BoxWrapper& b)
            {
                prim0 p;
                bool res = isBoxPrim0(b, &p);
                return nb::make_tuple(res, res ? prim0name(p) : "");
            },
            arg("box"))

        .def(
            "isBoxPrim1",
            [](BoxWrapper& b)
            {
                prim1 p;
                bool res = isBoxPrim1(b, &p);
                return nb::make_tuple(res, res ? prim1name(p) : "");
            },
            arg("box"))

        .def(
            "isBoxPrim2",
            [](BoxWrapper& b)
            {
                prim2 p;
                bool res = isBoxPrim2(b, &p);
                return nb::make_tuple(res, res ? prim2name(p) : "");
            },
            arg("box"))

        .def(
            "isBoxPrim3",
            [](BoxWrapper& b)
            {
                prim3 p;
                bool res = isBoxPrim3(b, &p);
                return nb::make_tuple(res, res ? prim3name(p) : "");
            },
            arg("box"))

        .def(
            "isBoxPrim4",
            [](BoxWrapper& b)
            {
                prim4 p;
                bool res = isBoxPrim4(b, &p);
                return nb::make_tuple(res, res ? prim4name(p) : "");
            },
            arg("box"))

        .def(
            "isBoxPrim5",
            [](BoxWrapper& b)
            {
                prim5 p;
                bool res = isBoxPrim5(b, &p);
                return nb::make_tuple(res, res ? prim5name(p) : "");
            },
            arg("box"))

        .def(
            "isBoxReal",
            [](BoxWrapper& b)
            {
                double r = 0;
                bool res = isBoxReal(b, &r);
                return nb::make_tuple(res, r);
            },
            arg("box"))

        .def(
            "isBoxRec",
            [](BoxWrapper& b)
            {
                Box x = nullptr, y = nullptr;
                bool res = isBoxRec(b, x, y);
                return nb::make_tuple(res, BoxWrapper(x), BoxWrapper(y));
            },
            arg("box"))

        .def(
            "isBoxRoute",
            [](BoxWrapper& b)
            {
                Box n = nullptr, m = nullptr, r = nullptr;
                bool res = isBoxRoute(b, n, m, r);
                return nb::make_tuple(res, BoxWrapper(n), BoxWrapper(m), BoxWrapper(r));
            },
            arg("box"))

        .def(
            "isBoxSeq",
            [](BoxWrapper& b)
            {
                Box x = nullptr, y = nullptr;
                bool res = isBoxSeq(b, x, y);
                return nb::make_tuple(res, BoxWrapper(x), BoxWrapper(y));
            },
            arg("box"))

        .def(
            "isBoxSlot",
            [](BoxWrapper& b)
            {
                int i = 0;
                bool res = isBoxSlot(b, &i);
                return nb::make_tuple(res, i);
            },
            arg("box"))

        .def(
            "isBoxSoundfile",
            [](BoxWrapper& b)
            {
                Box label = nullptr, chan = nullptr;
                bool res = isBoxSoundfile(b, label, chan);
                return nb::make_tuple(res, BoxWrapper(label), BoxWrapper(chan));
            },
            arg("box"))

        .def(
            "isBoxSplit",
            [](BoxWrapper& b)
            {
                Box x = nullptr, y = nullptr;
                bool res = isBoxSplit(b, x, y);
                return nb::make_tuple(res, BoxWrapper(x), BoxWrapper(y));
            },
            arg("box"))

        .def(
            "isBoxSymbolic",
            [](BoxWrapper& b)
            {
                Box slot = nullptr, body = nullptr;
                bool res = isBoxSymbolic(b, slot, body);
                return nb::make_tuple(res, BoxWrapper(slot), BoxWrapper(body));
            },
            arg("box"))

        .def(
            "isBoxTGroup",
            [](BoxWrapper& b)
            {
                Box label = nullptr, x = nullptr;
                bool res = isBoxTGroup(b, label, x);
                return nb::make_tuple(res, BoxWrapper(label), BoxWrapper(x));
            },
            arg("box"))

        .def(
            "isBoxVBargraph",
            [](BoxWrapper& b)
            {
                Box label = nullptr, a_min = nullptr, a_max = nullptr;
                bool res = isBoxVBargraph(b, label, a_min, a_max);
                return nb::make_tuple(res, BoxWrapper(label), BoxWrapper(a_min), BoxWrapper(a_max));
            },
            arg("box"))

        .def(
            "isBoxVGroup",
            [](BoxWrapper& b)
            {
                Box label = nullptr, x = nullptr;
                bool res = isBoxVGroup(b, label, x);
                return nb::make_tuple(res, BoxWrapper(label), BoxWrapper(x));
            },
            arg("box"))

        .def(
            "isBoxVSlider",
            [](BoxWrapper& b)
            {
                Box label = nullptr, init = nullptr, a_min = nullptr, a_max = nullptr,
                    step = nullptr;
                bool res = isBoxVSlider(b, label, init, a_min, a_max, step);
                return nb::make_tuple(res, BoxWrapper(label), BoxWrapper(init), BoxWrapper(a_min),
                                      BoxWrapper(a_max), BoxWrapper(step));
            },
            arg("box"))

        .def(
            "isBoxWaveform",
            [](BoxWrapper& b)
            {
                bool res = isBoxWaveform(b);
                std::vector<float> vals;
                if (res)
                {
                    double r = 0;
                    int j = 0;
                    const tvec br = b.ptr->branches();
                    for (int i = 0; i < br.size(); i++)
                    {
                        if (isBoxReal(br[i], &r))
                        {
                            vals.push_back(r);
                        }
                        else if (isBoxInt(br[i], &j))
                        {
                            vals.push_back(j);
                        }
                    }
                }
                return nb::make_tuple(res, vals);
            },
            arg("box"))

        .def(
            "isBoxWire", [](BoxWrapper& b) { return isBoxWire(b); }, arg("box"))

        .def(
            "isBoxWithLocalDef",
            [](BoxWrapper& b)
            {
                Box body = nullptr, ldef = nullptr;
                bool res = isBoxWithLocalDef(b, body, ldef);
                return nb::make_tuple(res, BoxWrapper(body), BoxWrapper(ldef));
            },
            arg("box"))

        .def(
            "boxToSource",
            [](BoxWrapper& box, const std::string& lang, const std::string& class_name)
            {
                auto pathToFaustLibraries = getPathToFaustLibraries();

                if (pathToFaustLibraries.empty())
                {
                    throw std::runtime_error("Unable to load Faust Libraries.");
                }

                auto pathToArchitecture = getPathToArchitectureFiles();
                if (pathToArchitecture.empty())
                {
                    throw std::runtime_error("Unable to find Faust architecture files.");
                }

                int argc = 0;
                const char* argv[512];

                argv[argc++] = "-I";
                argv[argc++] = strdup(pathToFaustLibraries.c_str());

                argv[argc++] = "-cn";
                argv[argc++] = strdup(class_name.c_str());

                argv[argc++] = "-A";
                argv[argc++] = strdup(pathToArchitecture.c_str());

                std::string error_msg = "";

                std::string source_code =
                    createSourceFromBoxes("dawdreamer", box, lang, argc, argv, error_msg);

                // Clean up strdup'd strings (only odd indices: 1, 3, 5)
                for (int i = 1; i < argc; i += 2)
                {
                    free((void*)argv[i]);
                }

                if (source_code == "")
                {
                    throw std::runtime_error(error_msg);
                }

                std::variant<std::string, nb::bytes> result;
                if (lang == "wasm" || lang == "wast")
                {
                    result = nb::bytes(source_code.c_str(), source_code.size());
                    return result;
                }
                result = source_code;
                return result;
            },
            arg("box"), arg("language"), arg("class_name"))
        .def(
            "boxToSource",
            [](BoxWrapper& box, const std::string& lang, const std::string& class_name,
               std::vector<std::string> in_argv)
            {
                auto pathToFaustLibraries = getPathToFaustLibraries();

                if (pathToFaustLibraries.empty())
                {
                    throw std::runtime_error("Unable to load Faust Libraries.");
                }

                auto pathToArchitecture = getPathToArchitectureFiles();
                if (pathToArchitecture.empty())
                {
                    throw std::runtime_error("Unable to find Faust architecture files.");
                }

                int argc = 0;
                const char* argv[512];

                argv[argc++] = "-I";
                argv[argc++] = strdup(pathToFaustLibraries.c_str());

                argv[argc++] = "-cn";
                argv[argc++] = strdup(class_name.c_str());

                argv[argc++] = "-A";
                argv[argc++] = strdup(pathToArchitecture.c_str());

                for (auto& v : in_argv)
                {
                    argv[argc++] = strdup(v.c_str());
                }

                std::string error_msg = "";

                std::string source_code =
                    createSourceFromBoxes("dawdreamer", box, lang, argc, argv, error_msg);

                // Clean up strdup'd strings (odd indices 1,3,5 and all from index 6 onwards)
                for (int i = 1; i < 6; i += 2)
                {
                    free((void*)argv[i]);
                }
                for (int i = 6; i < argc; i++)
                {
                    free((void*)argv[i]);
                }

                if (source_code == "")
                {
                    throw std::runtime_error(error_msg);
                }

                std::variant<std::string, nb::bytes> result;
                if (lang == "wasm" || lang == "wast")
                {
                    result = nb::bytes(source_code.c_str(), source_code.size());
                    return result;
                }
                result = source_code;
                return result;
            },
            arg("box"), arg("language"), arg("class_name"), arg("argv") = nb::none(),
            "Turn a box into source code in a target language such as \"cpp\". "
            "The second argument `argv` is a list of strings to send to a Faust "
            "command line.");

    nb::enum_<SType>(box_module, "SType")
        .value("kSInt", SType::kSInt)
        .value("kSReal", SType::kSReal)
        .export_values();

    nb::implicitly_convertible<float, BoxWrapper>();
    nb::implicitly_convertible<int, BoxWrapper>();
    nb::implicitly_convertible<int, SType>();

    return box_module;
}
#endif
