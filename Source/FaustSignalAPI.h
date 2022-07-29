#include "FaustProcessor.h"

#define TREE2STR(res, t) res ? tree2str(t->branch(0)) : ""

inline void create_bindings_for_faust_signal(py::module &faust_module) {
  using arg = py::arg;
  using kw_only = py::kw_only;

  auto signal_module = faust_module.def_submodule("signal");

  signal_module.doc() = R"pbdoc(
        dawdreamer
        -----------------------

		For reference: https://faustdoc.grame.fr/tutorials/signal-api/
    
        .. currentmodule:: dawdreamer.faust.signal
      
        .. autosummary::
           :toctree: _generate
    )pbdoc";

  auto returnPolicy = py::return_value_policy::take_ownership;

  py::class_<SigWrapper>(signal_module, "Signal")
      .def("__repr__",
           [](const SigWrapper &sig) {
             std::ostringstream out;
             out << "Signal[" << sig.ptr << "]";
             return out.str();
           })
      .def(py::init<float>(), arg("val"), "Init with a float")
      .def(py::init<int>(), arg("val"), "Init with an int")
      .def(
          "branches",
          [](const SigWrapper &sig) {
            std::vector<SigWrapper> branches;
            for (Signal b : sig.ptr->branches()) {
              branches.push_back(SigWrapper(b));
            }
            return branches;
          },
          returnPolicy)
      .def("node", [](const SigWrapper &sig) { return sig.ptr->node(); })
      .def("__add__",
           [](const SigWrapper &s1, SigWrapper &s2) {
             return SigWrapper(sigAdd((SigWrapper &)s1, s2));
           })
      .def("__add__",
           [](const SigWrapper &s1, float other) {
             return SigWrapper(sigAdd((SigWrapper &)s1, sigReal(other)));
           })
      .def("__radd__",
           [](const SigWrapper &s1, float other) {
             return SigWrapper(sigAdd((SigWrapper &)s1, sigReal(other)));
           })
      .def("__add__",
           [](const SigWrapper &s1, int other) {
             return SigWrapper(sigAdd((SigWrapper &)s1, sigInt(other)));
           })
      .def("__radd__",
           [](const SigWrapper &s1, int other) {
             return SigWrapper(sigAdd((SigWrapper &)s1, sigInt(other)));
           })

      .def("__sub__",
           [](const SigWrapper &s1, SigWrapper &s2) {
             return SigWrapper(sigSub((SigWrapper &)s1, s2));
           })
      .def("__sub__",
           [](const SigWrapper &s1, float other) {
             return SigWrapper(sigSub((SigWrapper &)s1, sigReal(other)));
           })
      .def("__rsub__",
           [](const SigWrapper &s1, float other) {
             return SigWrapper(sigSub((SigWrapper &)s1, sigReal(other)));
           })
      .def("__sub__",
           [](const SigWrapper &s1, int other) {
             return SigWrapper(sigSub((SigWrapper &)s1, sigInt(other)));
           })
      .def("__rsub__",
           [](const SigWrapper &s1, int other) {
             return SigWrapper(sigSub((SigWrapper &)s1, sigInt(other)));
           })

      .def("__mul__",
           [](const SigWrapper &s1, SigWrapper &s2) {
             return SigWrapper(sigMul((SigWrapper &)s1, s2));
           })
      .def("__mul__",
           [](const SigWrapper &s1, float other) {
             return SigWrapper(sigMul((SigWrapper &)s1, sigReal(other)));
           })
      .def("__rmul__",
           [](const SigWrapper &s1, float other) {
             return SigWrapper(sigMul((SigWrapper &)s1, sigReal(other)));
           })
      .def("__mul__",
           [](const SigWrapper &s1, int other) {
             return SigWrapper(sigMul((SigWrapper &)s1, sigInt(other)));
           })
      .def("__rmul__",
           [](const SigWrapper &s1, int other) {
             return SigWrapper(sigMul((SigWrapper &)s1, sigInt(other)));
           })

      .def("__truediv__",
           [](const SigWrapper &s1, SigWrapper &s2) {
             return SigWrapper(sigDiv((SigWrapper &)s1, s2));
           })
      .def("__truediv__",
           [](const SigWrapper &s1, float other) {
             return SigWrapper(sigDiv((SigWrapper &)s1, sigReal(other)));
           })
      .def("__rtruediv__",
           [](const SigWrapper &s1, float other) {
             return SigWrapper(sigDiv((SigWrapper &)s1, sigReal(other)));
           })
      .def("__truediv__",
           [](const SigWrapper &s1, int other) {
             return SigWrapper(sigDiv((SigWrapper &)s1, sigInt(other)));
           })
      .def("__rtruediv__",
           [](const SigWrapper &s1, int other) {
             return SigWrapper(sigDiv((SigWrapper &)s1, sigInt(other)));
           })

      .def("__mod__",
           [](const SigWrapper &s1, SigWrapper &s2) {
             return SigWrapper(sigFmod((SigWrapper &)s1, s2));
           })
      .def("__mod__",
           [](const SigWrapper &s1, float other) {
             return SigWrapper(sigFmod((SigWrapper &)s1, sigReal(other)));
           })
      .def("__mod__", [](const SigWrapper &s1, int other) {
        return SigWrapper(sigFmod((SigWrapper &)s1, sigInt(other)));
      });

  // SIGNAL API
  signal_module
      .def(
          "sigInt", [](int val) { return SigWrapper(sigInt(val)); }, arg("val"),
          returnPolicy)
      .def(
          "sigReal", [](double val) { return SigWrapper(sigReal(val)); },
          arg("val"), returnPolicy)
      .def(
          "sigInput", [](int index) { return SigWrapper(sigInput(index)); },
          arg("index"), returnPolicy)
      .def(
          "sigDelay",
          [](SigWrapper &sig1, SigWrapper &sig2) {
            return SigWrapper(sigDelay(sig1, sig2));
          },
          arg("sig1"), arg("sig2"), returnPolicy)

      .def(
          "sigIntCast",
          [](SigWrapper &sig1) { return SigWrapper(sigIntCast(sig1)); },
          arg("sig1"), returnPolicy)
      .def(
          "sigFloatCast",
          [](SigWrapper &sig1) { return SigWrapper(sigFloatCast(sig1)); },
          arg("sig1"), returnPolicy)

      .def(
          "sigReadOnlyTable",
          [](SigWrapper &n, SigWrapper &init, SigWrapper &ridx) {
            return SigWrapper(sigReadOnlyTable(n, init, sigIntCast(ridx)));
          },
          arg("n"), arg("init"), arg("ridx"), returnPolicy)
      .def(
          "sigWriteReadTable",
          [](SigWrapper &n, SigWrapper &init, SigWrapper &widx,
             SigWrapper &wsig, SigWrapper &ridx) {
            return SigWrapper(sigWriteReadTable(
                n, init, sigIntCast(widx), sigIntCast(wsig), sigIntCast(ridx)));
          },
          arg("n"), arg("init"), arg("widx"), arg("wsig"), arg("ridx"),
          returnPolicy)

      .def(
          "sigWaveform",
          [](std::vector<float> vals) {
            tvec waveform;
            for (auto &val : vals) {
              waveform.push_back(sigReal(val));
            }
            auto mySigWaveform = sigWaveform(waveform);

            auto result = std::vector<SigWrapper>{
                SigWrapper(sigInt((int)waveform.size())),
                SigWrapper(mySigWaveform)};

            return result;
          },
          arg("vals"), returnPolicy)
      .def(
          "sigSoundfile",
          [](std::string &name, SigWrapper &rdx, SigWrapper &chan,
             SigWrapper &part) {
            // Soundfile definition
            Signal sf = sigSoundfile(name);
            Signal partInt = sigIntCast(part);
            // Wrapped index to avoid reading outside the buffer
            Signal wridx = sigIntCast(sigMax(
                sigInt(0),
                sigMin(sigIntCast(rdx),
                       sigSub(sigSoundfileLength(sf, partInt), sigInt(1)))));

            auto result = std::vector<SigWrapper>{
                SigWrapper(sigSoundfileLength(sf, partInt)),
                SigWrapper(sigSoundfileRate(sf, partInt)),
                SigWrapper(
                    sigSoundfileBuffer(sf, sigIntCast(chan), partInt, wridx))};

            return result;
          },
          arg("filepath"), arg("sig_read_index"), arg("sig_chan"),
          arg("sig_part"), returnPolicy)

      .def(
          "sigSelect2",
          [](SigWrapper &selector, SigWrapper &sig1, SigWrapper &sig2) {
            return SigWrapper(sigSelect2(selector, sig1, sig2));
          },
          arg("selector"), arg("sig1"), arg("sig2"), returnPolicy)
      .def(
          "sigSelect3",
          [](SigWrapper &selector, SigWrapper &sig1, SigWrapper &sig2,
             SigWrapper &sig3) {
            return SigWrapper(sigSelect3(selector, sig1, sig2, sig3));
          },
          arg("selector"), arg("sig1"), arg("sig2"), arg("sig3"), returnPolicy)

      .def(
          "sigFConst",
          [](SType type, const std::string &name, const std::string &file) {
            return SigWrapper(sigFConst(type, name, file));
          },
          arg("type"), arg("name"), arg("file"), returnPolicy)
      .def(
          "sigFVar",
          [](SType type, const std::string &name, const std::string &file) {
            return SigWrapper(sigFVar(type, name, file));
          },
          arg("type"), arg("name"), arg("file"), returnPolicy)

      .def(
          "sigBinOp",
          [](SOperator op, SigWrapper &sig1, SigWrapper &sig2) {
            return SigWrapper(sigBinOp(op, sig1, sig2));
          },
          arg("op"), arg("x"), arg("y"), returnPolicy)

      .def(
          "sigAdd",
          [](SigWrapper &sig1, SigWrapper &sig2) {
            return SigWrapper(sigAdd(sig1, sig2));
          },
          arg("sig1"), arg("sig2"), returnPolicy)
      .def(
          "sigSub",
          [](SigWrapper &sig1, SigWrapper &sig2) {
            return SigWrapper(sigSub(sig1, sig2));
          },
          arg("sig1"), arg("sig2"), returnPolicy)
      .def(
          "sigMul",
          [](SigWrapper &sig1, SigWrapper &sig2) {
            return SigWrapper(sigMul(sig1, sig2));
          },
          arg("sig1"), arg("sig2"), returnPolicy)
      .def(
          "sigDiv",
          [](SigWrapper &sig1, SigWrapper &sig2) {
            return SigWrapper(sigDiv(sig1, sig2));
          },
          arg("sig1"), arg("sig2"), returnPolicy)
      .def(
          "sigRem",
          [](SigWrapper &sig1, SigWrapper &sig2) {
            return SigWrapper(sigRem(sig1, sig2));
          },
          arg("sig1"), arg("sig2"), returnPolicy)

      .def(
          "sigLeftShift",
          [](SigWrapper &sig1, SigWrapper &sig2) {
            return SigWrapper(sigLeftShift(sig1, sig2));
          },
          arg("sig1"), arg("sig2"), returnPolicy)
      .def(
          "sigLRightShift",
          [](SigWrapper &sig1, SigWrapper &sig2) {
            return SigWrapper(sigLRightShift(sig1, sig2));
          },
          arg("sig1"), arg("sig2"), returnPolicy)
      .def(
          "sigARightShift",
          [](SigWrapper &sig1, SigWrapper &sig2) {
            return SigWrapper(sigARightShift(sig1, sig2));
          },
          arg("sig1"), arg("sig2"), returnPolicy)

      .def(
          "sigGT",
          [](SigWrapper &sig1, SigWrapper &sig2) {
            return SigWrapper(sigGT(sig1, sig2));
          },
          arg("sig1"), arg("sig2"), returnPolicy)
      .def(
          "sigLT",
          [](SigWrapper &sig1, SigWrapper &sig2) {
            return SigWrapper(sigLT(sig1, sig2));
          },
          arg("sig1"), arg("sig2"), returnPolicy)
      .def(
          "sigGE",
          [](SigWrapper &sig1, SigWrapper &sig2) {
            return SigWrapper(sigGE(sig1, sig2));
          },
          arg("sig1"), arg("sig2"), returnPolicy)
      .def(
          "sigLE",
          [](SigWrapper &sig1, SigWrapper &sig2) {
            return SigWrapper(sigLE(sig1, sig2));
          },
          arg("sig1"), arg("sig2"), returnPolicy)
      .def(
          "sigEQ",
          [](SigWrapper &sig1, SigWrapper &sig2) {
            return SigWrapper(sigEQ(sig1, sig2));
          },
          arg("sig1"), arg("sig2"), returnPolicy)
      .def(
          "sigNE",
          [](SigWrapper &sig1, SigWrapper &sig2) {
            return SigWrapper(sigNE(sig1, sig2));
          },
          arg("sig1"), arg("sig2"), returnPolicy)

      .def(
          "sigAND",
          [](SigWrapper &sig1, SigWrapper &sig2) {
            return SigWrapper(sigAND(sig1, sig2));
          },
          arg("sig1"), arg("sig2"), returnPolicy)
      .def(
          "sigOR",
          [](SigWrapper &sig1, SigWrapper &sig2) {
            return SigWrapper(sigOR(sig1, sig2));
          },
          arg("sig1"), arg("sig2"), returnPolicy)
      .def(
          "sigXOR",
          [](SigWrapper &sig1, SigWrapper &sig2) {
            return SigWrapper(sigXOR(sig1, sig2));
          },
          arg("sig1"), arg("sig2"), returnPolicy)

      .def(
          "sigAbs", [](SigWrapper &sig1) { return SigWrapper(sigAbs(sig1)); },
          arg("sig1"), returnPolicy)
      .def(
          "sigAcos", [](SigWrapper &sig1) { return SigWrapper(sigAcos(sig1)); },
          arg("sig1"), returnPolicy)
      .def(
          "sigTan", [](SigWrapper &sig1) { return SigWrapper(sigTan(sig1)); },
          arg("sig1"), returnPolicy)
      .def(
          "sigSqrt", [](SigWrapper &sig1) { return SigWrapper(sigSqrt(sig1)); },
          arg("sig1"), returnPolicy)
      .def(
          "sigSin", [](SigWrapper &sig1) { return SigWrapper(sigSin(sig1)); },
          arg("sig1"), returnPolicy)
      .def(
          "sigRint", [](SigWrapper &sig1) { return SigWrapper(sigRint(sig1)); },
          arg("sig1"), returnPolicy)
      .def(
          "sigLog", [](SigWrapper &sig1) { return SigWrapper(sigLog(sig1)); },
          arg("sig1"), returnPolicy)
      .def(
          "sigLog10",
          [](SigWrapper &sig1) { return SigWrapper(sigLog10(sig1)); },
          arg("sig1"), returnPolicy)
      .def(
          "sigFloor",
          [](SigWrapper &sig1) { return SigWrapper(sigFloor(sig1)); },
          arg("sig1"), returnPolicy)
      .def(
          "sigExp", [](SigWrapper &sig1) { return SigWrapper(sigExp(sig1)); },
          arg("sig1"), returnPolicy)
      .def(
          "sigExp10",
          [](SigWrapper &sig1) { return SigWrapper(sigExp10(sig1)); },
          arg("sig1"), returnPolicy)
      .def(
          "sigCos", [](SigWrapper &sig1) { return SigWrapper(sigCos(sig1)); },
          arg("sig1"), returnPolicy)
      .def(
          "sigCeil", [](SigWrapper &sig1) { return SigWrapper(sigCeil(sig1)); },
          arg("sig1"), returnPolicy)
      .def(
          "sigAtan", [](SigWrapper &sig1) { return SigWrapper(sigAtan(sig1)); },
          arg("sig1"), returnPolicy)
      .def(
          "sigAsin", [](SigWrapper &sig1) { return SigWrapper(sigAsin(sig1)); },
          arg("sig1"), returnPolicy)

      .def(
          "sigRemainder",
          [](SigWrapper &sig1, SigWrapper &sig2) {
            return SigWrapper(sigRemainder(sig1, sig2));
          },
          arg("sig1"), arg("sig2"), returnPolicy)
      .def(
          "sigPow",
          [](SigWrapper &sig1, SigWrapper &sig2) {
            return SigWrapper(sigPow(sig1, sig2));
          },
          arg("sig1"), arg("sig2"), returnPolicy)
      .def(
          "sigMin",
          [](SigWrapper &sig1, SigWrapper &sig2) {
            return SigWrapper(sigMin(sig1, sig2));
          },
          arg("sig1"), arg("sig2"), returnPolicy)
      .def(
          "sigMax",
          [](SigWrapper &sig1, SigWrapper &sig2) {
            return SigWrapper(sigMax(sig1, sig2));
          },
          arg("sig1"), arg("sig2"), returnPolicy)
      .def(
          "sigFmod",
          [](SigWrapper &sig1, SigWrapper &sig2) {
            return SigWrapper(sigFmod(sig1, sig2));
          },
          arg("sig1"), arg("sig2"), returnPolicy)
      .def(
          "sigAtan2",
          [](SigWrapper &sig1, SigWrapper &sig2) {
            return SigWrapper(sigAtan2(sig1, sig2));
          },
          arg("sig1"), arg("sig2"), returnPolicy)

      .def(
          "sigSelf", []() { return SigWrapper(sigSelf()); }, returnPolicy)
      .def(
          "sigRecursion",
          [](SigWrapper &sig1) { return SigWrapper(sigRecursion(sig1)); },
          arg("sig"), returnPolicy)

      .def(
          "sigButton",
          [](std::string &label) { return SigWrapper(sigButton(label)); },
          arg("label"), returnPolicy)
      .def(
          "sigCheckbox",
          [](std::string &label) { return SigWrapper(sigCheckbox(label)); },
          arg("label"), returnPolicy)

      .def(
          "sigVSlider",
          [](std::string &label, SigWrapper &sigInit, SigWrapper &sigMin,
             SigWrapper &sigMax, SigWrapper &sigStep) {
            return SigWrapper(
                sigVSlider(label, sigInit, sigMin, sigMax, sigStep));
          },
          arg("label"), arg("init"), arg("min"), arg("max"), arg("step"),
          returnPolicy)
      .def(
          "sigHSlider",
          [](std::string &label, SigWrapper &sigInit, SigWrapper &sigMin,
             SigWrapper &sigMax, SigWrapper &sigStep) {
            return SigWrapper(
                sigHSlider(label, sigInit, sigMin, sigMax, sigStep));
          },
          arg("label"), arg("init"), arg("min"), arg("max"), arg("step"),
          returnPolicy)

      .def(
          "sigNumEntry",
          [](std::string &label, SigWrapper &sigInit, SigWrapper &sigMin,
             SigWrapper &sigMax, SigWrapper &sigStep) {
            return SigWrapper(
                sigNumEntry(label, sigInit, sigMin, sigMax, sigStep));
          },
          arg("label"), arg("init"), arg("min"), arg("max"), arg("step"),
          returnPolicy)

      .def(
          "sigVBargraph",
          [](std::string &label, SigWrapper &sigMin, SigWrapper &sigMax,
             SigWrapper &sig) {
            return SigWrapper(sigVBargraph(label, sigMin, sigMax, sig));
          },
          arg("label"), arg("min"), arg("max"), arg("step"), returnPolicy)
      .def(
          "sigHBargraph",
          [](std::string &label, SigWrapper &sigMin, SigWrapper &sigMax,
             SigWrapper &sig) {
            return SigWrapper(sigHBargraph(label, sigMin, sigMax, sig));
          },
          arg("label"), arg("min"), arg("max"), arg("step"), returnPolicy)

      .def(
          "sigAttach",
          [](SigWrapper &s1, SigWrapper &s2) {
            return SigWrapper(sigAttach(s1, s2));
          },
          arg("sig1"), arg("sig2"), returnPolicy)

      .def(
          "sigSampleRate",
          []() {
            return SigWrapper(sigMin(
                sigReal(192000.0),
                sigMax(sigReal(1.0),
                       sigFConst(SType::kSInt, "fSamplingFreq", "<math.h>"))));
          },
          returnPolicy)
      .def(
          "sigBufferSize",
          []() {
            return SigWrapper(sigFVar(SType::kSInt, "count", "<math.h>"));
          },
          returnPolicy)

      .def(
          "isSigNil", [](SigWrapper &s1) { return isNil(s1); }, arg("sig"),
          returnPolicy)

      .def(
          "isSigInt",
          [](SigWrapper &s1) {
            int i = 0;
            bool res = isSigInt(s1, &i);
            return py::make_tuple<py::return_value_policy::take_ownership>(res,
                                                                           i);
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigReal",
          [](SigWrapper &s1) {
            double r;
            bool res = isSigReal(s1, &r);
            return py::make_tuple<py::return_value_policy::take_ownership>(res,
                                                                           r);
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigWaveform", [](SigWrapper &s1) { return isSigWaveform(s1); },
          arg("sig"), returnPolicy)

      .def(
          "isSigInput",
          [](SigWrapper &s1) {
            int i;
            bool res = isSigInput(s1, &i);
            return py::make_tuple<py::return_value_policy::take_ownership>(res,
                                                                           i);
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigOutput",
          [](SigWrapper &s1) {
            int i;
            Signal s2;
            bool res = isSigOutput(s1, &i, s2);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, i, SigWrapper(s2));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigDelay1",
          [](SigWrapper &s1) {
            Signal s2;
            bool res = isSigDelay1(s1, s2);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, SigWrapper(s2));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigDelay",
          [](SigWrapper &s1) {
            Signal s2;
            Signal s3;
            bool res = isSigDelay(s1, s2, s3);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, SigWrapper(s2), SigWrapper(s3));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigPrefix",
          [](SigWrapper &s1) {
            Signal s2;
            Signal s3;
            bool res = isSigPrefix(s1, s2, s3);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, SigWrapper(s2), SigWrapper(s3));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigIntCast",
          [](SigWrapper &s1) {
            Signal s2;
            bool res = isSigIntCast(s1, s2);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, SigWrapper(s2));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigFloatCast",
          [](SigWrapper &s1) {
            Signal s2;
            bool res = isSigFloatCast(s1, s2);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, SigWrapper(s2));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigRDTbl",
          [](SigWrapper &s) {
            Signal t;
            Signal i;
            bool res = isSigRDTbl(s, t, i);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, SigWrapper(t), SigWrapper(i));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigWRTbl",
          [](SigWrapper &u) {
            Signal id, t, i, s;
            bool res = isSigWRTbl(u, id, t, i, s);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, SigWrapper(id), SigWrapper(t), SigWrapper(i),
                SigWrapper(s));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigTable",
          [](SigWrapper &t) {
            Signal id, n, sig;
            bool res = isSigTable(t, id, n, sig);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, SigWrapper(id), SigWrapper(n), SigWrapper(sig));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigDocConstantTbl",
          [](SigWrapper &s) {
            Signal n, init;
            bool res = isSigDocConstantTbl(s, n, init);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, SigWrapper(n), SigWrapper(init));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigDocWriteTbl",
          [](SigWrapper &s) {
            Signal n, init, widx, wsig;
            bool res = isSigDocWriteTbl(s, n, init, widx, wsig);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, SigWrapper(n), SigWrapper(init), SigWrapper(widx),
                SigWrapper(wsig));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigDocAccessTbl",
          [](SigWrapper &s) {
            Signal doctbl, ridx;
            bool res = isSigDocConstantTbl(s, doctbl, ridx);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, SigWrapper(doctbl), SigWrapper(ridx));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigSelect2",
          [](SigWrapper &s) {
            Signal selector, s1, s2;
            bool res = isSigSelect2(s, selector, s1, s2);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, SigWrapper(selector), SigWrapper(s1), SigWrapper(s2));
          },
          arg("sig"), returnPolicy)

      .def(
          "isProj",
          [](SigWrapper &s) {
            int i;
            Signal rgroup;
            bool res = isProj(s.ptr, &i, rgroup);

            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, i, SigWrapper(rgroup));
          },
          arg("sig"), returnPolicy)

      .def(
          "isRec",
          [](SigWrapper &s) {
            Signal var, body;
            bool res = isRec(s, var, body);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, SigWrapper(var), SigWrapper(body));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigBinOp",
          [](SigWrapper &s) {
            Signal x, y;
            int i;
            bool res = isSigBinOp(s, &i, x, y);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, i, SigWrapper(x), SigWrapper(y));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigButton",
          [](SigWrapper &s) {
            Signal label;
            bool res = isSigButton(s, label);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, TREE2STR(res, label));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigCheckbox",
          [](SigWrapper &s) {
            Signal label;
            bool res = isSigCheckbox(s, label);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, TREE2STR(res, label));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigVSlider",
          [](SigWrapper &s) {
            Signal label, init, theMin, theMax, step;
            bool res = isSigVSlider(s, label, init, theMin, theMax, step);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, TREE2STR(res, label), SigWrapper(init),
                SigWrapper(theMin), SigWrapper(theMax), SigWrapper(step));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigHSlider",
          [](SigWrapper &s) {
            Signal label, init, theMin, theMax, step;
            bool res = isSigHSlider(s, label, init, theMin, theMax, step);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, TREE2STR(res, label), SigWrapper(init),
                SigWrapper(theMin), SigWrapper(theMax), SigWrapper(step));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigNumEntry",
          [](SigWrapper &s) {
            Signal label, init, theMin, theMax, step;
            bool res = isSigNumEntry(s, label, init, theMin, theMax, step);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, TREE2STR(res, label), SigWrapper(init),
                SigWrapper(theMin), SigWrapper(theMax), SigWrapper(step));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigVBargraph",
          [](SigWrapper &s) {
            Signal label, theMin, theMax, t0;
            bool res = isSigVBargraph(s, label, theMin, theMax, t0);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, TREE2STR(res, label), SigWrapper(theMin),
                SigWrapper(theMax), SigWrapper(t0));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigHBargraph",
          [](SigWrapper &s) {
            Signal label, theMin, theMax, t0;
            bool res = isSigHBargraph(s, label, theMin, theMax, t0);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, TREE2STR(res, label), SigWrapper(theMin),
                SigWrapper(theMax), SigWrapper(t0));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigAttach",
          [](SigWrapper &s) {
            Signal x, y;
            bool res = isSigAttach(s, x, y);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, SigWrapper(x), SigWrapper(y));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigEnable",
          [](SigWrapper &s) {
            Signal x, y;
            bool res = isSigEnable(s, x, y);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, SigWrapper(x), SigWrapper(y));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigControl",
          [](SigWrapper &s) {
            Signal x, y;
            bool res = isSigControl(s, x, y);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, SigWrapper(x), SigWrapper(y));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigSoundfile",
          [](SigWrapper &s) {
            Signal label;
            bool res = isSigSoundfile(s, label);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, TREE2STR(res, label));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigSoundfileLength",
          [](SigWrapper &s) {
            Signal sf, part;
            bool res = isSigSoundfileLength(s, sf, part);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, SigWrapper(sf), SigWrapper(part));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigSoundfileRate",
          [](SigWrapper &s) {
            Signal sf, part;
            bool res = isSigSoundfileRate(s, sf, part);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, SigWrapper(sf), SigWrapper(part));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigSoundfileBuffer",
          [](SigWrapper &s) {
            Signal sf, chan, part, ridx;
            bool res = isSigSoundfileBuffer(s, sf, chan, part, ridx);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, SigWrapper(sf), SigWrapper(chan), SigWrapper(part),
                SigWrapper(ridx));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigLowest",
          [](SigWrapper &s) {
            Signal x;
            bool res = isSigLowest(s, x);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, SigWrapper(x));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigHighest",
          [](SigWrapper &s) {
            Signal x;
            bool res = isSigHighest(s, x);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, SigWrapper(x));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigGen",
          [](SigWrapper &s) {
            Signal x;
            bool res = isSigGen(s, x);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, SigWrapper(x));
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigFFun",
          [](SigWrapper &s) {
            Signal ff, largs;
            bool res = isSigFFun(s, ff, largs);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, res ? tree2str(ff->branch(1)) : "",
                SigWrapper(largs));
          },
          arg("sig"), returnPolicy)
      .def(
          "isSigFVar",
          [](SigWrapper &s) {
            Signal type, name, file;
            bool res = isSigFVar(s, type, name, file);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, SigWrapper(type), res ? tree2str(name) : "",
                res ? tree2str(file) : "");
          },
          arg("sig"), returnPolicy)

      .def(
          "isSigFConst",
          [](SigWrapper &s) {
            Signal type, name, file;
            bool res = isSigFConst(s, type, name, file);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, SigWrapper(type), res ? tree2str(name) : "",
                res ? tree2str(file) : "");
          },
          arg("sig"), returnPolicy)

      .def(
          "getUserData",
          [](SigWrapper &s) {
            return bool(getUserData(s));
          },
          arg("sig"), returnPolicy)
			  
			  ;
}