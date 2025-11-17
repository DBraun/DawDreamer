#ifdef BUILD_DAWDREAMER_FAUST
#include "FaustSignalAPI.h"

#define TREE2STR(res, t) res ? tree2str(t->branch(0)) : ""

using arg = nb::arg;
using kw_only = nb::kw_only;

void add_operation(nb::class_<SigWrapper>& cls, const char* name, Signal (*func)(Signal, Signal))
{
    cls.def(name, [func](const SigWrapper& s, int other)
            { return SigWrapper(func((SigWrapper&)s, sigInt(other))); })
        .def(name, [func](const SigWrapper& s, float other)
             { return SigWrapper(func((SigWrapper&)s, sigReal(other))); })
        .def(name, [func](const SigWrapper& s, SigWrapper& s2)
             { return SigWrapper(func((SigWrapper&)s, s2)); });
}

void add_unary_operation(nb::class_<SigWrapper>& cls, const char* name, Signal (*func)(Signal))
{
    cls.def(name, [func](const SigWrapper& s) { return SigWrapper(func((SigWrapper&)s)); });
}

void create_bindings_for_faust_signal(nb::module_& faust_module, nb::module_& signal_module)
{
    using arg = nb::arg;
    using kw_only = nb::kw_only;

    faust_module.def(
        "boxToSignals",
        [](BoxWrapper& box)
        {
            std::string error_msg;
            tvec signals = boxesToSignals(box, error_msg);

            if (!error_msg.empty())
            {
                throw std::runtime_error(error_msg);
            }

            std::vector<SigWrapper> outSignals;
            for (Signal sig : signals)
            {
                outSignals.push_back(SigWrapper(sig));
            }

            return outSignals;
        },
        arg("box"), "Convert a box to a list of signals.", nb::rv_policy::take_ownership);

    // SIGNAL API
    signal_module.doc() = R"pbdoc(
        The Faust Signal API
        -------------------------------------------------------------

		For reference: https://faustdoc.grame.fr/tutorials/signal-api/

        .. currentmodule:: dawdreamer.faust.signal

        .. autosummary::
           :toctree: _generate
    )pbdoc";

    nb::class_<SigWrapper> cls(signal_module, "Signal");

    add_operation(cls, "__add__", static_cast<Signal (*)(Signal, Signal)>(sigAdd));
    add_operation(cls, "__radd__", static_cast<Signal (*)(Signal, Signal)>(sigAdd));
    add_operation(cls, "__sub__", static_cast<Signal (*)(Signal, Signal)>(sigSub));
    add_operation(cls, "__rsub__", static_cast<Signal (*)(Signal, Signal)>(sigSub));

    add_operation(cls, "__mul__", static_cast<Signal (*)(Signal, Signal)>(sigMul));
    add_operation(cls, "__rmul__", static_cast<Signal (*)(Signal, Signal)>(sigMul));

    add_operation(cls, "__truediv__", static_cast<Signal (*)(Signal, Signal)>(sigDiv));
    add_operation(cls, "__rtruediv__", static_cast<Signal (*)(Signal, Signal)>(sigDiv));

    add_operation(cls, "__mod__", static_cast<Signal (*)(Signal, Signal)>(sigFmod));

    add_operation(cls, "__lt__", static_cast<Signal (*)(Signal, Signal)>(sigLT));
    add_operation(cls, "__le__", static_cast<Signal (*)(Signal, Signal)>(sigLE));
    add_operation(cls, "__gt__", static_cast<Signal (*)(Signal, Signal)>(sigGT));
    add_operation(cls, "__ge__", static_cast<Signal (*)(Signal, Signal)>(sigGE));
    add_operation(cls, "__eq__", static_cast<Signal (*)(Signal, Signal)>(sigEQ));
    add_operation(cls, "__ne__", static_cast<Signal (*)(Signal, Signal)>(sigNE));

    add_operation(cls, "__pow__", static_cast<Signal (*)(Signal, Signal)>(sigPow));

    add_operation(cls, "__lshift__", static_cast<Signal (*)(Signal, Signal)>(sigLeftShift));
    add_operation(cls, "__rshift__", static_cast<Signal (*)(Signal, Signal)>(sigARightShift));

    add_operation(cls, "__and__", static_cast<Signal (*)(Signal, Signal)>(sigAND));
    add_operation(cls, "__or__", static_cast<Signal (*)(Signal, Signal)>(sigOR));
    add_operation(cls, "__xor__", static_cast<Signal (*)(Signal, Signal)>(sigXOR));

    auto floordiv_func = [](Signal s, Signal other) { return sigFloor(sigDiv(s, other)); };

    add_operation(cls, "__floordiv__", static_cast<Signal (*)(Signal, Signal)>(floordiv_func));

    add_unary_operation(cls, "__abs__", static_cast<Signal (*)(Signal)>(sigAbs));
    add_unary_operation(cls, "__int__", static_cast<Signal (*)(Signal)>(sigIntCast));
    add_unary_operation(cls, "__float__", static_cast<Signal (*)(Signal)>(sigFloatCast));
    add_unary_operation(cls, "__floor__", static_cast<Signal (*)(Signal)>(sigFloor));
    add_unary_operation(cls, "__ceil__", static_cast<Signal (*)(Signal)>(sigCeil));

    cls.def(nb::init<float>(), arg("val"), "Init with a float")
        .def(nb::init<int>(), arg("val"), "Init with an int")
        .def("__repr__", [](const SigWrapper& s) { return tree2str((SigWrapper&)s); })
        .def_prop_ro("userdata",
                     [](const SigWrapper& s) { return bool(getUserData((SigWrapper&)s)); })
        .def_prop_ro("branches",
                     [](const SigWrapper& sig)
                     {
                         std::vector<SigWrapper> branches;
                         for (Signal b : sig.ptr->branches())
                         {
                             branches.push_back(SigWrapper(b));
                         }
                         return branches;
                     })
        .def_prop_ro(
            "xtended_name", [](const SigWrapper& s1) { return xtendedName((SigWrapper&)s1); },
            "Return the name of the xtended signal.")
        .def_prop_ro(
            "ffarity", [](const SigWrapper& s1) { return ffarity((SigWrapper&)s1); },
            "Return the arity of a foreign function.")
        .def_prop_ro(
            "xtended_arity", [](const SigWrapper& s1) { return xtendedArity((SigWrapper&)s1); },
            "Return the arity of the xtended signal.")
        .def(
            "print_str", [](const SigWrapper& s1, const bool shared, const int max_size)
            { return printSignal((SigWrapper&)s1, shared, max_size); }, arg("shared"),
            arg("max_size"),
            "Return the signal as a string. The argument `shared` is whether "
            "identical sub signals are printed as identifiers. The argument "
            "`max_size` is the maximum number of characters to be printed "
            "(possibly needed for big expressions in non shared mode).")
        .def("__neg__",
             [](const SigWrapper& s1) { return SigWrapper(sigSub(sigReal(0), (SigWrapper&)s1)); });

    signal_module.def(
                     "sigInt", [](int val) { return SigWrapper(sigInt(val)); }, arg("val"))
        .def(
            "sigReal", [](double val) { return SigWrapper(sigReal(val)); }, arg("val"))
        .def(
            "sigInput", [](int index) { return SigWrapper(sigInput(index)); }, arg("index"))
        .def(
            "sigDelay", [](SigWrapper& sig1, SigWrapper& sig2)
            { return SigWrapper(sigDelay(sig1, sig2)); }, arg("sig1"), arg("sig2"))
        .def(
            "sigDelay1", [](SigWrapper& sig1) { return SigWrapper(sigDelay1(sig1)); }, arg("sig1"))

        .def(
            "sigIntCast", [](SigWrapper& sig1) { return SigWrapper(sigIntCast(sig1)); },
            arg("sig1"))
        .def(
            "sigFloatCast", [](SigWrapper& sig1) { return SigWrapper(sigFloatCast(sig1)); },
            arg("sig1"))

        .def(
            "sigReadOnlyTable", [](SigWrapper& n, SigWrapper& init, SigWrapper& ridx)
            { return SigWrapper(sigReadOnlyTable(n, init, sigIntCast(ridx))); }, arg("n"),
            arg("init"), arg("ridx"))
        .def(
            "sigWriteReadTable",
            [](SigWrapper& n, SigWrapper& init, SigWrapper& widx, SigWrapper& wsig,
               SigWrapper& ridx)
            {
                return SigWrapper(sigWriteReadTable(n, init, sigIntCast(widx), sigIntCast(wsig),
                                                    sigIntCast(ridx)));
            },
            arg("n"), arg("init"), arg("widx"), arg("wsig"), arg("ridx"))

        .def(
            "sigWaveform",
            [](std::vector<float> vals)
            {
                tvec waveform;
                for (auto& val : vals)
                {
                    waveform.push_back(sigReal(val));
                }
                auto mySigWaveform = sigWaveform(waveform);

                auto result = std::vector<SigWrapper>{SigWrapper(sigInt((int)waveform.size())),
                                                      SigWrapper(mySigWaveform)};

                return result;
            },
            arg("vals"))
        .def(
            "sigSoundfile",
            [](std::string& name, SigWrapper& rdx, SigWrapper& chan, SigWrapper& part)
            {
                // Soundfile definition
                Signal sf = sigSoundfile(name);
                Signal partInt = sigIntCast(part);
                // Wrapped index to avoid reading outside the buffer
                Signal wridx = sigIntCast(
                    sigMax(sigInt(0), sigMin(sigIntCast(rdx),
                                             sigSub(sigSoundfileLength(sf, partInt), sigInt(1)))));

                auto result = std::vector<SigWrapper>{
                    SigWrapper(sigSoundfileLength(sf, partInt)),
                    SigWrapper(sigSoundfileRate(sf, partInt)),
                    SigWrapper(sigSoundfileBuffer(sf, sigIntCast(chan), partInt, wridx))};

                return result;
            },
            arg("filepath"), arg("sig_read_index"), arg("sig_chan"), arg("sig_part"))

        .def(
            "sigSelect2", [](SigWrapper& selector, SigWrapper& sig1, SigWrapper& sig2)
            { return SigWrapper(sigSelect2(selector, sig1, sig2)); }, arg("selector"), arg("sig1"),
            arg("sig2"))
        .def(
            "sigSelect3",
            [](SigWrapper& selector, SigWrapper& sig1, SigWrapper& sig2, SigWrapper& sig3)
            { return SigWrapper(sigSelect3(selector, sig1, sig2, sig3)); }, arg("selector"),
            arg("sig1"), arg("sig2"), arg("sig3"))

        .def(
            "sigFFun",
            [](SType rtype, nvec names, svec atypes, const std::string& incfile,
               const std::string& libfile, tvec largs)
            { return SigWrapper(sigFFun(rtype, names, atypes, incfile, libfile, largs)); },
            arg("type"), arg("names"), arg("arg_types"), arg("inc_file"), arg("lib_file"),
            arg("largs"), "Create a foreign function signal.")
        .def(
            "sigFConst", [](SType type, const std::string& name, const std::string& file)
            { return SigWrapper(sigFConst(type, name, file)); }, arg("type"), arg("name"),
            arg("file"), "Create a foreign constant signal.")
        .def(
            "sigFVar", [](SType type, const std::string& name, const std::string& file)
            { return SigWrapper(sigFVar(type, name, file)); }, arg("type"), arg("name"),
            arg("file"))

        .def(
            "sigBinOp", [](SOperator op, SigWrapper& sig1, SigWrapper& sig2)
            { return SigWrapper(sigBinOp(op, sig1, sig2)); }, arg("op"), arg("x"), arg("y"))

        .def(
            "sigAdd", [](SigWrapper& sig1, SigWrapper& sig2)
            { return SigWrapper(sigAdd(sig1, sig2)); }, arg("sig1"), arg("sig2"))
        .def(
            "sigSub", [](SigWrapper& sig1, SigWrapper& sig2)
            { return SigWrapper(sigSub(sig1, sig2)); }, arg("sig1"), arg("sig2"))
        .def(
            "sigMul", [](SigWrapper& sig1, SigWrapper& sig2)
            { return SigWrapper(sigMul(sig1, sig2)); }, arg("sig1"), arg("sig2"))
        .def(
            "sigDiv", [](SigWrapper& sig1, SigWrapper& sig2)
            { return SigWrapper(sigDiv(sig1, sig2)); }, arg("sig1"), arg("sig2"))
        .def(
            "sigRem", [](SigWrapper& sig1, SigWrapper& sig2)
            { return SigWrapper(sigRem(sig1, sig2)); }, arg("sig1"), arg("sig2"))

        .def(
            "sigLeftShift", [](SigWrapper& sig1, SigWrapper& sig2)
            { return SigWrapper(sigLeftShift(sig1, sig2)); }, arg("sig1"), arg("sig2"))
        .def(
            "sigLRightShift", [](SigWrapper& sig1, SigWrapper& sig2)
            { return SigWrapper(sigLRightShift(sig1, sig2)); }, arg("sig1"), arg("sig2"))
        .def(
            "sigARightShift", [](SigWrapper& sig1, SigWrapper& sig2)
            { return SigWrapper(sigARightShift(sig1, sig2)); }, arg("sig1"), arg("sig2"))

        .def(
            "sigGT", [](SigWrapper& sig1, SigWrapper& sig2)
            { return SigWrapper(sigGT(sig1, sig2)); }, arg("sig1"), arg("sig2"))
        .def(
            "sigLT", [](SigWrapper& sig1, SigWrapper& sig2)
            { return SigWrapper(sigLT(sig1, sig2)); }, arg("sig1"), arg("sig2"))
        .def(
            "sigGE", [](SigWrapper& sig1, SigWrapper& sig2)
            { return SigWrapper(sigGE(sig1, sig2)); }, arg("sig1"), arg("sig2"))
        .def(
            "sigLE", [](SigWrapper& sig1, SigWrapper& sig2)
            { return SigWrapper(sigLE(sig1, sig2)); }, arg("sig1"), arg("sig2"))
        .def(
            "sigEQ", [](SigWrapper& sig1, SigWrapper& sig2)
            { return SigWrapper(sigEQ(sig1, sig2)); }, arg("sig1"), arg("sig2"))
        .def(
            "sigNE", [](SigWrapper& sig1, SigWrapper& sig2)
            { return SigWrapper(sigNE(sig1, sig2)); }, arg("sig1"), arg("sig2"))

        .def(
            "sigAND", [](SigWrapper& sig1, SigWrapper& sig2)
            { return SigWrapper(sigAND(sig1, sig2)); }, arg("sig1"), arg("sig2"))
        .def(
            "sigOR", [](SigWrapper& sig1, SigWrapper& sig2)
            { return SigWrapper(sigOR(sig1, sig2)); }, arg("sig1"), arg("sig2"))
        .def(
            "sigXOR", [](SigWrapper& sig1, SigWrapper& sig2)
            { return SigWrapper(sigXOR(sig1, sig2)); }, arg("sig1"), arg("sig2"))

        .def(
            "sigAbs", [](SigWrapper& sig1) { return SigWrapper(sigAbs(sig1)); }, arg("sig1"))
        .def(
            "sigAcos", [](SigWrapper& sig1) { return SigWrapper(sigAcos(sig1)); }, arg("sig1"))
        .def(
            "sigTan", [](SigWrapper& sig1) { return SigWrapper(sigTan(sig1)); }, arg("sig1"))
        .def(
            "sigSqrt", [](SigWrapper& sig1) { return SigWrapper(sigSqrt(sig1)); }, arg("sig1"))
        .def(
            "sigSin", [](SigWrapper& sig1) { return SigWrapper(sigSin(sig1)); }, arg("sig1"))
        .def(
            "sigRint", [](SigWrapper& sig1) { return SigWrapper(sigRint(sig1)); }, arg("sig1"))
        .def(
            "sigLog", [](SigWrapper& sig1) { return SigWrapper(sigLog(sig1)); }, arg("sig1"))
        .def(
            "sigLog10", [](SigWrapper& sig1) { return SigWrapper(sigLog10(sig1)); }, arg("sig1"))
        .def(
            "sigFloor", [](SigWrapper& sig1) { return SigWrapper(sigFloor(sig1)); }, arg("sig1"))
        .def(
            "sigExp", [](SigWrapper& sig1) { return SigWrapper(sigExp(sig1)); }, arg("sig1"))
        .def(
            "sigExp10", [](SigWrapper& sig1) { return SigWrapper(sigExp10(sig1)); }, arg("sig1"))
        .def(
            "sigCos", [](SigWrapper& sig1) { return SigWrapper(sigCos(sig1)); }, arg("sig1"))
        .def(
            "sigCeil", [](SigWrapper& sig1) { return SigWrapper(sigCeil(sig1)); }, arg("sig1"))
        .def(
            "sigAtan", [](SigWrapper& sig1) { return SigWrapper(sigAtan(sig1)); }, arg("sig1"))
        .def(
            "sigAsin", [](SigWrapper& sig1) { return SigWrapper(sigAsin(sig1)); }, arg("sig1"))

        .def(
            "sigRemainder", [](SigWrapper& sig1, SigWrapper& sig2)
            { return SigWrapper(sigRemainder(sig1, sig2)); }, arg("sig1"), arg("sig2"))
        .def(
            "sigPow", [](SigWrapper& sig1, SigWrapper& sig2)
            { return SigWrapper(sigPow(sig1, sig2)); }, arg("sig1"), arg("sig2"))
        .def(
            "sigMin", [](SigWrapper& sig1, SigWrapper& sig2)
            { return SigWrapper(sigMin(sig1, sig2)); }, arg("sig1"), arg("sig2"))
        .def(
            "sigMax", [](SigWrapper& sig1, SigWrapper& sig2)
            { return SigWrapper(sigMax(sig1, sig2)); }, arg("sig1"), arg("sig2"))
        .def(
            "sigFmod", [](SigWrapper& sig1, SigWrapper& sig2)
            { return SigWrapper(sigFmod(sig1, sig2)); }, arg("sig1"), arg("sig2"))
        .def(
            "sigAtan2", [](SigWrapper& sig1, SigWrapper& sig2)
            { return SigWrapper(sigAtan2(sig1, sig2)); }, arg("sig1"), arg("sig2"))

        .def("sigSelf", []() { return SigWrapper(sigSelf()); })

        .def(
            "sigSelfN", [](const int id) { return SigWrapper(sigSelfN(id)); }, arg("id"),
            "`id` is the recursive signal index (starting from 0, up to the "
            "number of outputs signals in the recursive block)")

        .def(
            "sigRecursion", [](SigWrapper& sig1) { return SigWrapper(sigRecursion(sig1)); },
            arg("sig"))

        .def(
            "sigRecursionN",
            [](const std::vector<SigWrapper>& signals)
            {
                tvec rf;
                for (SigWrapper s : signals)
                {
                    rf.push_back(s);
                }

                tvec b = sigRecursionN(rf);
                std::vector<SigWrapper> out;
                for (Signal s : b)
                {
                    out.push_back(s);
                }
                return out;
            },
            arg("sig"))

        .def(
            "sigButton", [](std::string& label) { return SigWrapper(sigButton(label)); },
            arg("label"))
        .def(
            "sigCheckbox", [](std::string& label) { return SigWrapper(sigCheckbox(label)); },
            arg("label"))

        .def(
            "sigVSlider",
            [](std::string& label, SigWrapper& sigInit, SigWrapper& sigMin, SigWrapper& sigMax,
               SigWrapper& sigStep)
            { return SigWrapper(sigVSlider(label, sigInit, sigMin, sigMax, sigStep)); },
            arg("label"), arg("init"), arg("min"), arg("max"), arg("step"))
        .def(
            "sigHSlider",
            [](std::string& label, SigWrapper& sigInit, SigWrapper& sigMin, SigWrapper& sigMax,
               SigWrapper& sigStep)
            { return SigWrapper(sigHSlider(label, sigInit, sigMin, sigMax, sigStep)); },
            arg("label"), arg("init"), arg("min"), arg("max"), arg("step"))

        .def(
            "sigNumEntry",
            [](std::string& label, SigWrapper& sigInit, SigWrapper& sigMin, SigWrapper& sigMax,
               SigWrapper& sigStep)
            { return SigWrapper(sigNumEntry(label, sigInit, sigMin, sigMax, sigStep)); },
            arg("label"), arg("init"), arg("min"), arg("max"), arg("step"))

        .def(
            "sigVBargraph",
            [](std::string& label, SigWrapper& sigMin, SigWrapper& sigMax, SigWrapper& sig)
            { return SigWrapper(sigVBargraph(label, sigMin, sigMax, sig)); }, arg("label"),
            arg("min"), arg("max"), arg("step"))
        .def(
            "sigHBargraph",
            [](std::string& label, SigWrapper& sigMin, SigWrapper& sigMax, SigWrapper& sig)
            { return SigWrapper(sigHBargraph(label, sigMin, sigMax, sig)); }, arg("label"),
            arg("min"), arg("max"), arg("step"))

        .def(
            "sigAttach", [](SigWrapper& s1, SigWrapper& s2)
            { return SigWrapper(sigAttach(s1, s2)); }, arg("sig1"), arg("sig2"))

        .def(
            "simplifyToNormalForm",
            [](SigWrapper& s1) { return SigWrapper(simplifyToNormalForm(s1)); }, arg("sig"))

        .def(
            "simplifyToNormalForm2",
            [](std::vector<SigWrapper>& wrappers)
            {
                tvec siglist;
                for (SigWrapper s : wrappers)
                {
                    siglist.push_back(s);
                }
                tvec b = simplifyToNormalForm2(siglist);
                std::vector<SigWrapper> out;
                for (Signal s : b)
                {
                    out.push_back(s);
                }

                return out;
            },
            arg("sig"))

        .def("sigSampleRate",
             []()
             {
                 return SigWrapper(sigMin(
                     sigReal(192000.0),
                     sigMax(sigReal(1.0), sigFConst(SType::kSInt, "fSamplingFreq", "<math.h>"))));
             })
        .def("sigBufferSize",
             []() { return SigWrapper(sigFVar(SType::kSInt, "count", "<math.h>")); })

        .def(
            "isSigNil", [](SigWrapper& s1) { return isNil(s1); }, arg("sig"))

        .def(
            "isSigInt",
            [](SigWrapper& s1)
            {
                int i = 0;
                bool res = isSigInt(s1, &i);
                return nb::make_tuple<nb::rv_policy::take_ownership>(res, i);
            },
            arg("sig"))

        .def(
            "isSigReal",
            [](SigWrapper& s1)
            {
                double r = 0;
                bool res = isSigReal(s1, &r);
                return nb::make_tuple<nb::rv_policy::take_ownership>(res, r);
            },
            arg("sig"))

        .def(
            "isSigWaveform", [](SigWrapper& s1) { return isSigWaveform(s1); }, arg("sig"))

        .def(
            "isSigInput",
            [](SigWrapper& s1)
            {
                int i = 0;
                bool res = isSigInput(s1, &i);
                return nb::make_tuple<nb::rv_policy::take_ownership>(res, i);
            },
            arg("sig"))

        .def(
            "isSigOutput",
            [](SigWrapper& s1)
            {
                int i = 0;
                Signal s2;
                bool res = isSigOutput(s1, &i, s2);
                return nb::make_tuple<nb::rv_policy::take_ownership>(res, i, SigWrapper(s2));
            },
            arg("sig"))

        .def(
            "isSigDelay1",
            [](SigWrapper& s1)
            {
                Signal s2;
                bool res = isSigDelay1(s1, s2);
                return nb::make_tuple<nb::rv_policy::take_ownership>(res, SigWrapper(s2));
            },
            arg("sig"))

        .def(
            "isSigDelay",
            [](SigWrapper& s1)
            {
                Signal s2;
                Signal s3;
                bool res = isSigDelay(s1, s2, s3);
                return nb::make_tuple<nb::rv_policy::take_ownership>(res, SigWrapper(s2),
                                                                     SigWrapper(s3));
            },
            arg("sig"))

        .def(
            "isSigPrefix",
            [](SigWrapper& s1)
            {
                Signal s2;
                Signal s3;
                bool res = isSigPrefix(s1, s2, s3);
                return nb::make_tuple<nb::rv_policy::take_ownership>(res, SigWrapper(s2),
                                                                     SigWrapper(s3));
            },
            arg("sig"))

        .def(
            "isSigIntCast",
            [](SigWrapper& s1)
            {
                Signal s2;
                bool res = isSigIntCast(s1, s2);
                return nb::make_tuple<nb::rv_policy::take_ownership>(res, SigWrapper(s2));
            },
            arg("sig"))

        .def(
            "isSigFloatCast",
            [](SigWrapper& s1)
            {
                Signal s2;
                bool res = isSigFloatCast(s1, s2);
                return nb::make_tuple<nb::rv_policy::take_ownership>(res, SigWrapper(s2));
            },
            arg("sig"))

        .def(
            "isSigRDTbl",
            [](SigWrapper& s)
            {
                Signal t;
                Signal i;
                bool res = isSigRDTbl(s, t, i);
                return nb::make_tuple<nb::rv_policy::take_ownership>(res, SigWrapper(t),
                                                                     SigWrapper(i));
            },
            arg("sig"))

        .def(
            "isSigWRTbl",
            [](SigWrapper& u)
            {
                Signal id, t, i, s;
                bool res = isSigWRTbl(u, id, t, i, s);
                return nb::make_tuple<nb::rv_policy::take_ownership>(
                    res, SigWrapper(id), SigWrapper(t), SigWrapper(i), SigWrapper(s));
            },
            arg("sig"))

        .def(
            "isSigDocConstantTbl",
            [](SigWrapper& s)
            {
                Signal n, init;
                bool res = isSigDocConstantTbl(s, n, init);
                return nb::make_tuple<nb::rv_policy::take_ownership>(res, SigWrapper(n),
                                                                     SigWrapper(init));
            },
            arg("sig"))

        .def(
            "isSigDocWriteTbl",
            [](SigWrapper& s)
            {
                Signal n, init, widx, wsig;
                bool res = isSigDocWriteTbl(s, n, init, widx, wsig);
                return nb::make_tuple<nb::rv_policy::take_ownership>(
                    res, SigWrapper(n), SigWrapper(init), SigWrapper(widx), SigWrapper(wsig));
            },
            arg("sig"))

        .def(
            "isSigDocAccessTbl",
            [](SigWrapper& s)
            {
                Signal doctbl, ridx;
                bool res = isSigDocConstantTbl(s, doctbl, ridx);
                return nb::make_tuple<nb::rv_policy::take_ownership>(res, SigWrapper(doctbl),
                                                                     SigWrapper(ridx));
            },
            arg("sig"))

        .def(
            "isSigSelect2",
            [](SigWrapper& s)
            {
                Signal selector, s1, s2;
                bool res = isSigSelect2(s, selector, s1, s2);
                return nb::make_tuple<nb::rv_policy::take_ownership>(
                    res, SigWrapper(selector), SigWrapper(s1), SigWrapper(s2));
            },
            arg("sig"))

        .def(
            "isSigAssertBounds",
            [](SigWrapper& s)
            {
                Signal s1, s2, s3;
                bool res = isSigAssertBounds(s, s1, s2, s3);
                return nb::make_tuple<nb::rv_policy::take_ownership>(
                    res, SigWrapper(s1), SigWrapper(s2), SigWrapper(s3));
            },
            arg("sig"))

        .def(
            "isProj",
            [](SigWrapper& s)
            {
                int i = 0;
                Signal rgroup;
                bool res = isProj(s.ptr, &i, rgroup);

                return nb::make_tuple<nb::rv_policy::take_ownership>(res, i, SigWrapper(rgroup));
            },
            arg("sig"))

        .def(
            "isRec",
            [](SigWrapper& s)
            {
                Signal var, body;
                bool res = isRec(s, var, body);
                return nb::make_tuple<nb::rv_policy::take_ownership>(res, SigWrapper(var),
                                                                     SigWrapper(body));
            },
            arg("sig"))

        .def(
            "isSigBinOp",
            [](SigWrapper& s)
            {
                Signal x, y;
                int i = 0;
                bool res = isSigBinOp(s, &i, x, y);
                return nb::make_tuple<nb::rv_policy::take_ownership>(res, i, SigWrapper(x),
                                                                     SigWrapper(y));
            },
            arg("sig"))

        .def(
            "isSigButton",
            [](SigWrapper& s)
            {
                Signal label;
                bool res = isSigButton(s, label);
                return nb::make_tuple<nb::rv_policy::take_ownership>(res, TREE2STR(res, label));
            },
            arg("sig"))

        .def(
            "isSigCheckbox",
            [](SigWrapper& s)
            {
                Signal label;
                bool res = isSigCheckbox(s, label);
                return nb::make_tuple<nb::rv_policy::take_ownership>(res, TREE2STR(res, label));
            },
            arg("sig"))

        .def(
            "isSigVSlider",
            [](SigWrapper& s)
            {
                Signal label, init, theMin, theMax, step;
                bool res = isSigVSlider(s, label, init, theMin, theMax, step);
                return nb::make_tuple<nb::rv_policy::take_ownership>(
                    res, TREE2STR(res, label), SigWrapper(init), SigWrapper(theMin),
                    SigWrapper(theMax), SigWrapper(step));
            },
            arg("sig"))

        .def(
            "isSigHSlider",
            [](SigWrapper& s)
            {
                Signal label, init, theMin, theMax, step;
                bool res = isSigHSlider(s, label, init, theMin, theMax, step);
                return nb::make_tuple<nb::rv_policy::take_ownership>(
                    res, TREE2STR(res, label), SigWrapper(init), SigWrapper(theMin),
                    SigWrapper(theMax), SigWrapper(step));
            },
            arg("sig"))

        .def(
            "isSigNumEntry",
            [](SigWrapper& s)
            {
                Signal label, init, theMin, theMax, step;
                bool res = isSigNumEntry(s, label, init, theMin, theMax, step);
                return nb::make_tuple<nb::rv_policy::take_ownership>(
                    res, TREE2STR(res, label), SigWrapper(init), SigWrapper(theMin),
                    SigWrapper(theMax), SigWrapper(step));
            },
            arg("sig"))

        .def(
            "isSigVBargraph",
            [](SigWrapper& s)
            {
                Signal label, theMin, theMax, t0;
                bool res = isSigVBargraph(s, label, theMin, theMax, t0);
                return nb::make_tuple<nb::rv_policy::take_ownership>(
                    res, TREE2STR(res, label), SigWrapper(theMin), SigWrapper(theMax),
                    SigWrapper(t0));
            },
            arg("sig"))

        .def(
            "isSigHBargraph",
            [](SigWrapper& s)
            {
                Signal label, theMin, theMax, t0;
                bool res = isSigHBargraph(s, label, theMin, theMax, t0);
                return nb::make_tuple<nb::rv_policy::take_ownership>(
                    res, TREE2STR(res, label), SigWrapper(theMin), SigWrapper(theMax),
                    SigWrapper(t0));
            },
            arg("sig"))

        .def(
            "isSigAttach",
            [](SigWrapper& s)
            {
                Signal x, y;
                bool res = isSigAttach(s, x, y);
                return nb::make_tuple<nb::rv_policy::take_ownership>(res, SigWrapper(x),
                                                                     SigWrapper(y));
            },
            arg("sig"))

        .def(
            "isSigEnable",
            [](SigWrapper& s)
            {
                Signal x, y;
                bool res = isSigEnable(s, x, y);
                return nb::make_tuple<nb::rv_policy::take_ownership>(res, SigWrapper(x),
                                                                     SigWrapper(y));
            },
            arg("sig"))

        .def(
            "isSigControl",
            [](SigWrapper& s)
            {
                Signal x, y;
                bool res = isSigControl(s, x, y);
                return nb::make_tuple<nb::rv_policy::take_ownership>(res, SigWrapper(x),
                                                                     SigWrapper(y));
            },
            arg("sig"))

        .def(
            "isSigSoundfile",
            [](SigWrapper& s)
            {
                Signal label;
                bool res = isSigSoundfile(s, label);
                return nb::make_tuple<nb::rv_policy::take_ownership>(res, TREE2STR(res, label));
            },
            arg("sig"))

        .def(
            "isSigSoundfileLength",
            [](SigWrapper& s)
            {
                Signal sf, part;
                bool res = isSigSoundfileLength(s, sf, part);
                return nb::make_tuple<nb::rv_policy::take_ownership>(res, SigWrapper(sf),
                                                                     SigWrapper(part));
            },
            arg("sig"))

        .def(
            "isSigSoundfileRate",
            [](SigWrapper& s)
            {
                Signal sf, part;
                bool res = isSigSoundfileRate(s, sf, part);
                return nb::make_tuple<nb::rv_policy::take_ownership>(res, SigWrapper(sf),
                                                                     SigWrapper(part));
            },
            arg("sig"))

        .def(
            "isSigSoundfileBuffer",
            [](SigWrapper& s)
            {
                Signal sf, chan, part, ridx;
                bool res = isSigSoundfileBuffer(s, sf, chan, part, ridx);
                return nb::make_tuple<nb::rv_policy::take_ownership>(
                    res, SigWrapper(sf), SigWrapper(chan), SigWrapper(part), SigWrapper(ridx));
            },
            arg("sig"))

        .def(
            "isSigLowest",
            [](SigWrapper& s)
            {
                Signal x;
                bool res = isSigLowest(s, x);
                return nb::make_tuple<nb::rv_policy::take_ownership>(res, SigWrapper(x));
            },
            arg("sig"))

        .def(
            "isSigHighest",
            [](SigWrapper& s)
            {
                Signal x;
                bool res = isSigHighest(s, x);
                return nb::make_tuple<nb::rv_policy::take_ownership>(res, SigWrapper(x));
            },
            arg("sig"))

        .def(
            "isSigGen",
            [](SigWrapper& s)
            {
                Signal x;
                bool res = isSigGen(s, x);
                return nb::make_tuple<nb::rv_policy::take_ownership>(res, SigWrapper(x));
            },
            arg("sig"))

        .def(
            "isSigFFun",
            [](SigWrapper& s)
            {
                Signal ff, largs;
                bool res = isSigFFun(s, ff, largs);
                return nb::make_tuple<nb::rv_policy::take_ownership>(
                    res, res ? tree2str(ff->branch(1)) : "", SigWrapper(largs));
            },
            arg("sig"))
        .def(
            "isSigFVar",
            [](SigWrapper& s)
            {
                Signal type, name, file;
                bool res = isSigFVar(s, type, name, file);
                return nb::make_tuple<nb::rv_policy::take_ownership>(
                    res, SigWrapper(type), res ? tree2str(name) : "", res ? tree2str(file) : "");
            },
            arg("sig"))

        .def(
            "isSigFConst",
            [](SigWrapper& s)
            {
                Signal type, name, file;
                bool res = isSigFConst(s, type, name, file);
                return nb::make_tuple<nb::rv_policy::take_ownership>(
                    res, SigWrapper(type), res ? tree2str(name) : "", res ? tree2str(file) : "");
            },
            arg("sig"))

        .def(
            "signalsToSource",
            [](std::vector<SigWrapper>& wrappers, const std::string& lang,
               const std::string& class_name)
            {
                tvec signals;
                for (auto wrapper : wrappers)
                {
                    signals.push_back(wrapper);
                }

                auto pathToFaustLibraries = getPathToFaustLibraries();

                if (pathToFaustLibraries.empty())
                {
                    throw std::runtime_error("Unable to load Faust Libraries.");
                }

                int argc = 0;
                const char* argv[64];

                argv[argc++] = "-I";
                argv[argc++] = strdup(pathToFaustLibraries.c_str());

                argv[argc++] = "-I";
                argv[argc++] = strdup((pathToFaustLibraries + "/dx7").c_str());

                argv[argc++] = "-cn";
                argv[argc++] = strdup(class_name.c_str());

                std::string error_msg = "";

                std::string source_code =
                    createSourceFromSignals("dawdreamer", signals, lang, argc, argv, error_msg);

                // Clean up strdup'd strings (odd indices 1,3,5 and all from index 6 onwards)
                for (int i = 1; i < 6; i += 2)
                {
                    free((void*)argv[i]);
                }
                for (int i = 6; i < argc; i++)
                {
                    free((void*)argv[i]);
                }

                if (!error_msg.empty())
                {
                    throw std::runtime_error(error_msg);
                }

                return source_code;
            },
            arg("signals"), arg("language"), arg("class_name"))
        .def(
            "signalsToSource",
            [](std::vector<SigWrapper>& wrappers, const std::string& lang,
               const std::string& class_name, const std::vector<std::string>& in_argv)
            {
                tvec signals;
                for (auto wrapper : wrappers)
                {
                    signals.push_back(wrapper);
                }

                auto pathToFaustLibraries = getPathToFaustLibraries();

                if (pathToFaustLibraries.empty())
                {
                    throw std::runtime_error("Unable to load Faust Libraries.");
                }

                int argc = 0;
                const char* argv[64];

                argv[argc++] = "-I";
                argv[argc++] = strdup(pathToFaustLibraries.c_str());

                argv[argc++] = "-I";
                argv[argc++] = strdup((pathToFaustLibraries + "/dx7").c_str());

                argv[argc++] = "-cn";
                argv[argc++] = strdup(class_name.c_str());

                for (auto v : in_argv)
                {
                    argv[argc++] = strdup(v.c_str());
                }

                std::string error_msg = "";

                std::string source_code =
                    createSourceFromSignals("dawdreamer", signals, lang, argc, argv, error_msg);

                // Clean up strdup'd strings (odd indices 1,3,5 and all from index 6 onwards)
                for (int i = 1; i < 6; i += 2)
                {
                    free((void*)argv[i]);
                }
                for (int i = 6; i < argc; i++)
                {
                    free((void*)argv[i]);
                }

                if (!error_msg.empty())
                {
                    throw std::runtime_error(error_msg);
                }

                return source_code;
            },
            arg("signals"), arg("language"), arg("class_name"), arg("argv"),
            "Turn a list of signals into source code in a target language such "
            "as \"cpp\". The second argument `argv` is a list of strings to send "
            "to a Faust command line.")

        ;

    nb::implicitly_convertible<float, SigWrapper>();
    nb::implicitly_convertible<int, SigWrapper>();
}

#endif
