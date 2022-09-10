#include "FaustProcessor.h"

inline void create_bindings_for_faust_box(py::module &faust_module) {
  using arg = py::arg;
  using kw_only = py::kw_only;

  auto box_module = faust_module.def_submodule("box");

  box_module.doc() = R"pbdoc(
        dawdreamer
        -----------------------
    
		For reference: https://faustdoc.grame.fr/tutorials/box-api/

        .. currentmodule:: dawdreamer.faust.box
      
        .. autosummary::
           :toctree: _generate
    )pbdoc";

  auto returnPolicy = py::return_value_policy::take_ownership;

  py::class_<BoxWrapper>(box_module, "Box")
      .def(py::init<float>(), arg("val"), "Init with a float", returnPolicy)
      .def(py::init<int>(), arg("val"), "Init with an int", returnPolicy)
      .def(
          "__add__",
          [](const BoxWrapper &box1, BoxWrapper &box2) {
            return BoxWrapper(boxAdd((BoxWrapper &)box1, box2));
          },
          returnPolicy)
      .def(
          "__add__",
          [](const BoxWrapper &box1, float other) {
            return BoxWrapper(boxAdd((BoxWrapper &)box1, boxReal(other)));
          },
          returnPolicy)
      .def(
          "__radd__",
          [](const BoxWrapper &box1, float other) {
            return BoxWrapper(boxAdd((BoxWrapper &)box1, boxReal(other)));
          },
          returnPolicy)
      .def(
          "__add__",
          [](const BoxWrapper &box1, int other) {
            return BoxWrapper(boxAdd((BoxWrapper &)box1, boxInt(other)));
          },
          returnPolicy)
      .def(
          "__radd__",
          [](const BoxWrapper &box1, int other) {
            return BoxWrapper(boxAdd((BoxWrapper &)box1, boxInt(other)));
          },
          returnPolicy)

      .def(
          "__sub__",
          [](const BoxWrapper &box1, BoxWrapper &box2) {
            return BoxWrapper(boxSub((BoxWrapper &)box1, box2));
          },
          returnPolicy)
      .def(
          "__sub__",
          [](const BoxWrapper &box1, float other) {
            return BoxWrapper(boxSub((BoxWrapper &)box1, boxReal(other)));
          },
          returnPolicy)
      .def(
          "__rsub__",
          [](const BoxWrapper &box1, float other) {
            return BoxWrapper(boxSub((BoxWrapper &)box1, boxReal(other)));
          },
          returnPolicy)
      .def(
          "__sub__",
          [](const BoxWrapper &box1, int other) {
            return BoxWrapper(boxSub((BoxWrapper &)box1, boxInt(other)));
          },
          returnPolicy)
      .def(
          "__rsub__",
          [](const BoxWrapper &box1, int other) {
            return BoxWrapper(boxSub((BoxWrapper &)box1, boxInt(other)));
          },
          returnPolicy)

      .def(
          "__mul__",
          [](const BoxWrapper &box1, BoxWrapper &box2) {
            return BoxWrapper(boxMul((BoxWrapper &)box1, box2));
          },
          returnPolicy)
      .def(
          "__mul__",
          [](const BoxWrapper &box1, float other) {
            return BoxWrapper(boxMul((BoxWrapper &)box1, boxReal(other)));
          },
          returnPolicy)
      .def(
          "__rmul__",
          [](const BoxWrapper &box1, float other) {
            return BoxWrapper(boxMul((BoxWrapper &)box1, boxReal(other)));
          },
          returnPolicy)
      .def(
          "__mul__",
          [](const BoxWrapper &box1, int other) {
            return BoxWrapper(boxMul((BoxWrapper &)box1, boxInt(other)));
          },
          returnPolicy)
      .def(
          "__rmul__",
          [](const BoxWrapper &box1, int other) {
            return BoxWrapper(boxMul((BoxWrapper &)box1, boxInt(other)));
          },
          returnPolicy)

      .def(
          "__truediv__",
          [](const BoxWrapper &box1, BoxWrapper &box2) {
            return BoxWrapper(boxDiv((BoxWrapper &)box1, box2));
          },
          returnPolicy)
      .def(
          "__truediv__",
          [](const BoxWrapper &box1, float other) {
            return BoxWrapper(boxDiv((BoxWrapper &)box1, boxReal(other)));
          },
          returnPolicy)
      .def(
          "__rtruediv__",
          [](const BoxWrapper &box1, float other) {
            return BoxWrapper(boxDiv((BoxWrapper &)box1, boxReal(other)));
          },
          returnPolicy)
      .def(
          "__truediv__",
          [](const BoxWrapper &box1, int other) {
            return BoxWrapper(boxDiv((BoxWrapper &)box1, boxInt(other)));
          },
          returnPolicy)
      .def(
          "__rtruediv__",
          [](const BoxWrapper &box1, int other) {
            return BoxWrapper(boxDiv((BoxWrapper &)box1, boxInt(other)));
          },
          returnPolicy)

      .def(
          "__mod__",
          [](const BoxWrapper &box1, BoxWrapper &box2) {
            return BoxWrapper(boxFmod((BoxWrapper &)box1, box2));
          },
          returnPolicy)
      .def(
          "__mod__",
          [](const BoxWrapper &box1, float other) {
            return BoxWrapper(boxFmod((BoxWrapper &)box1, boxReal(other)));
          },
          returnPolicy)
      .def(
          "__mod__",
          [](const BoxWrapper &box1, int other) {
            return BoxWrapper(boxFmod((BoxWrapper &)box1, boxInt(other)));
          },
          returnPolicy);

  box_module
      .def(
          "boxInt", [](int val) { return BoxWrapper(boxInt(val)); }, arg("val"),
          returnPolicy)
      .def(
          "boxReal", [](double val) { return BoxWrapper(boxReal(val)); },
          arg("val"), returnPolicy)
      .def(
          "boxWire", []() { return BoxWrapper(boxWire()); }, returnPolicy)
      .def(
          "boxCut", []() { return BoxWrapper(boxCut()); }, returnPolicy)

      .def(
          "boxSeq",
          [](BoxWrapper &box1, BoxWrapper &box2) {
            return BoxWrapper(boxSeq(box1, box2));
          },
          arg("box1"), arg("box2"),
          "The sequential composition of two blocks (e.g., A:B) expects: "
          "outputs(A)=inputs(B)",
          returnPolicy)

      .def(
          "boxPar",
          [](BoxWrapper &box1, BoxWrapper &box2) {
            return BoxWrapper(boxPar(box1, box2));
          },
          arg("box1"), arg("box2"),
          "The parallel composition of two blocks (e.g., A,B). It places the "
          "two block-diagrams one on top of the other, without connections.",
          returnPolicy)
      .def(
          "boxPar3",
          [](BoxWrapper &box1, BoxWrapper &box2, BoxWrapper &box3) {
            return BoxWrapper(boxPar3(box1, box2, box3));
          },
          arg("box1"), arg("box2"), arg("box3"),
          "The parallel composition of three blocks (e.g., A,B,C).",
          returnPolicy)
      .def(
          "boxPar4",
          [](BoxWrapper &box1, BoxWrapper &box2, BoxWrapper &box3,
             BoxWrapper &box4) {
            return BoxWrapper(boxPar4(box1, box2, box3, box4));
          },
          arg("box1"), arg("box2"), arg("box3"), arg("box4"),
          "The parallel composition of four blocks (e.g., A,B,C,D).",
          returnPolicy)
      .def(
          "boxPar5",
          [](BoxWrapper &box1, BoxWrapper &box2, BoxWrapper &box3,
             BoxWrapper &box4, BoxWrapper &box5) {
            return BoxWrapper(boxPar5(box1, box2, box3, box4, box5));
          },
          arg("box1"), arg("box2"), arg("box3"), arg("box4"), arg("box5"),
          "The parallel composition of five blocks (e.g., A,B,C,D,E).",
          returnPolicy)

      .def(
          "boxSplit",
          [](BoxWrapper &box1, BoxWrapper &box2) {
            return BoxWrapper(boxSplit(box1, box2));
          },
          arg("box1"), arg("box2"),
          "The split composition (e.g., A<:B) operator is used to distribute "
          "the outputs of A to the inputs of B. The number of inputs of B must "
          "be a multiple of the number of outputs of A: outputs(A).k=inputs(B)",
          returnPolicy)
      .def(
          "boxMerge",
          [](BoxWrapper &box1, BoxWrapper &box2) {
            return BoxWrapper(boxMerge(box1, box2));
          },
          arg("box1"), arg("box2"),
          "The merge composition (e.g., A:>B) is the dual of the split "
          "composition. The number of outputs of A must be a multiple of the "
          "number of inputs of B: outputs(A)=k.inputs(B)",
          returnPolicy)

      .def(
          "boxRoute",
          [](BoxWrapper &box1, BoxWrapper &box2, BoxWrapper &box3) {
            return BoxWrapper(boxRoute(box1, box2, box3));
          },
          arg("box_n"), arg("box_m"), arg("box_r"), returnPolicy)

      .def(
          "boxDelay",
          [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
            if (box1.has_value() && box2.has_value()) {
              return BoxWrapper(boxDelay(*box1, *box2));
            } else {
              return BoxWrapper(boxDelay());
            }
          },
          arg("box1") = py::none(), arg("box2") = py::none(), returnPolicy)

      .def(
          "boxIntCast",
          [](std::optional<BoxWrapper> box1) {
            if (box1.has_value()) {
              return BoxWrapper(boxIntCast(*box1));
            } else {
              return BoxWrapper(boxIntCast());
            }
          },
          arg("box1") = py::none(), returnPolicy)

      .def(
          "boxFloatCast",
          [](std::optional<BoxWrapper> box1) {
            if (box1.has_value()) {
              return BoxWrapper(boxFloatCast(*box1));
            } else {
              return BoxWrapper(boxFloatCast());
            }
          },
          arg("box1") = py::none(), returnPolicy)

      .def(
          "boxReadOnlyTable",
          [](std::optional<BoxWrapper> n, std::optional<BoxWrapper> init,
             std::optional<BoxWrapper> ridx) {
            if (n.has_value() && init.has_value() && ridx.has_value()) {
              return BoxWrapper(
                  boxReadOnlyTable(boxIntCast(*n), *init, boxIntCast(*ridx)));
            } else {
              return BoxWrapper(boxReadOnlyTable());
            }
          },
          arg("n") = py::none(), arg("init") = py::none(),
          arg("ridx") = py::none(), returnPolicy)

      .def(
          "boxWriteReadTable",
          [](std::optional<BoxWrapper> n, std::optional<BoxWrapper> init,
             std::optional<BoxWrapper> widx, std::optional<BoxWrapper> wsig,
             std::optional<BoxWrapper> ridx) {
            if (n.has_value() && init.has_value() && widx.has_value() &&
                wsig.has_value() && ridx.has_value()) {
              return BoxWrapper(
                  boxWriteReadTable(boxIntCast(*n), *init, boxIntCast(*widx),
                                    boxIntCast(*wsig), boxIntCast(*ridx)));
            } else {
              return BoxWrapper(boxWriteReadTable());
            }
          },
          arg("n") = py::none(), arg("init") = py::none(),
          arg("widx") = py::none(), arg("wsig") = py::none(),
          arg("ridx") = py::none(), returnPolicy)

      .def(
          "boxWaveform",
          [](std::vector<float> vals) {
            tvec waveform;
            for (auto &val : vals) {
              waveform.push_back(boxReal(val));
            }
            return BoxWrapper(boxWaveform(waveform));
          },
          arg("vals"), returnPolicy)
      .def(
          "boxSoundfile",
          [](std::string &label, BoxWrapper &chan,
             std::optional<BoxWrapper> part, std::optional<BoxWrapper> rdx) {
            if (part.has_value() && rdx.has_value()) {
              return BoxWrapper(boxSoundfile(label, boxIntCast(chan),
                                             boxIntCast(*part),
                                             boxIntCast(*rdx)));
            } else {
              return BoxWrapper(boxSoundfile(label, chan));
            }
          },
          arg("filepath"), arg("chan"), arg("part") = py::none(),
          arg("ridx") = py::none(), returnPolicy)

      .def(
          "boxSelect2",
          [](std::optional<BoxWrapper> selector, std::optional<BoxWrapper> box1,
             std::optional<BoxWrapper> box2) {
            if (selector.has_value() && box1.has_value() && box2.has_value()) {
              return BoxWrapper(boxSelect2(*selector, *box1, *box2));
            } else {
              return BoxWrapper(boxSelect2());
            }
          },
          arg("selector") = py::none(), arg("box1") = py::none(),
          arg("box2") = py::none(), returnPolicy)

      .def(
          "boxSelect3",
          [](std::optional<BoxWrapper> selector, std::optional<BoxWrapper> box1,
             std::optional<BoxWrapper> box2, std::optional<BoxWrapper> box3) {
            if (selector.has_value() && box1.has_value() && box2.has_value()) {
              return BoxWrapper(boxSelect3(*selector, *box1, *box2, *box3));
            } else {
              return BoxWrapper(boxSelect3());
            }
          },
          arg("selector") = py::none(), arg("box1") = py::none(),
          arg("box2") = py::none(), arg("box3") = py::none(), returnPolicy)

      .def(
          "boxFConst",
          [](SType type, const std::string &name, const std::string &file) {
            return BoxWrapper(boxFConst(type, name, file));
          },
          arg("type"), arg("name"), arg("file"), returnPolicy)
      .def(
          "boxFVar",
          [](SType type, const std::string &name, const std::string &file) {
            return BoxWrapper(boxFVar(type, name, file));
          },
          arg("type"), arg("name"), arg("file"), returnPolicy)

      .def(
          "boxBinOp",
          [](SOperator op, std::optional<BoxWrapper> box1,
             std::optional<BoxWrapper> box2) {
            if (box1.has_value() && box2.has_value()) {
              return BoxWrapper(boxBinOp(op, *box1, *box2));
            } else {
              return BoxWrapper(boxBinOp(op));
            }
          },
          arg("op"), arg("x") = py::none(), arg("y") = py::none(), returnPolicy)

      .def(
          "boxAdd",
          [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
            return box1.has_value() && box2.has_value()
                       ? BoxWrapper(boxAdd(*box1, *box2))
                       : BoxWrapper(boxAdd());
          },
          arg("box1") = py::none(), arg("box2") = py::none(), returnPolicy)
      .def(
          "boxSub",
          [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
            return box1.has_value() && box2.has_value()
                       ? BoxWrapper(boxSub(*box1, *box2))
                       : BoxWrapper(boxSub());
          },
          arg("box1") = py::none(), arg("box2") = py::none(), returnPolicy)
      .def(
          "boxMul",
          [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
            return box1.has_value() && box2.has_value()
                       ? BoxWrapper(boxMul(*box1, *box2))
                       : BoxWrapper(boxMul());
          },
          arg("box1") = py::none(), arg("box2") = py::none(), returnPolicy)
      .def(
          "boxDiv",
          [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
            return box1.has_value() && box2.has_value()
                       ? BoxWrapper(boxDiv(*box1, *box2))
                       : BoxWrapper(boxDiv());
          },
          arg("box1") = py::none(), arg("box2") = py::none(), returnPolicy)
      .def(
          "boxRem",
          [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
            return box1.has_value() && box2.has_value()
                       ? BoxWrapper(boxRem(*box1, *box2))
                       : BoxWrapper(boxRem());
          },
          arg("box1") = py::none(), arg("box2") = py::none(), returnPolicy)

      .def(
          "boxLeftShift",
          [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
            return box1.has_value() && box2.has_value()
                       ? BoxWrapper(boxLeftShift(*box1, *box2))
                       : BoxWrapper(boxLeftShift());
          },
          arg("box1") = py::none(), arg("box2") = py::none(), returnPolicy)
      .def(
          "boxLRightShift",
          [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
            return box1.has_value() && box2.has_value()
                       ? BoxWrapper(boxLRightShift(*box1, *box2))
                       : BoxWrapper(boxLRightShift());
          },
          arg("box1") = py::none(), arg("box2") = py::none(), returnPolicy)
      .def(
          "boxARightShift",
          [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
            return box1.has_value() && box2.has_value()
                       ? BoxWrapper(boxARightShift(*box1, *box2))
                       : BoxWrapper(boxARightShift());
          },
          arg("box1") = py::none(), arg("box2") = py::none(), returnPolicy)

      .def(
          "boxGT",
          [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
            return box1.has_value() && box2.has_value()
                       ? BoxWrapper(boxGT(*box1, *box2))
                       : BoxWrapper(boxGT());
          },
          arg("box1") = py::none(), arg("box2") = py::none(), returnPolicy)
      .def(
          "boxLT",
          [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
            return box1.has_value() && box2.has_value()
                       ? BoxWrapper(boxLT(*box1, *box2))
                       : BoxWrapper(boxLT());
          },
          arg("box1") = py::none(), arg("box2") = py::none(), returnPolicy)
      .def(
          "boxGE",
          [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
            return box1.has_value() && box2.has_value()
                       ? BoxWrapper(boxGE(*box1, *box2))
                       : BoxWrapper(boxGE());
          },
          arg("box1") = py::none(), arg("box2") = py::none(), returnPolicy)
      .def(
          "boxLE",
          [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
            return box1.has_value() && box2.has_value()
                       ? BoxWrapper(boxLE(*box1, *box2))
                       : BoxWrapper(boxLE());
          },
          arg("box1") = py::none(), arg("box2") = py::none(), returnPolicy)
      .def(
          "boxEQ",
          [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
            return box1.has_value() && box2.has_value()
                       ? BoxWrapper(boxEQ(*box1, *box2))
                       : BoxWrapper(boxEQ());
          },
          arg("box1") = py::none(), arg("box2") = py::none(), returnPolicy)
      .def(
          "boxNE",
          [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
            return box1.has_value() && box2.has_value()
                       ? BoxWrapper(boxNE(*box1, *box2))
                       : BoxWrapper(boxNE());
          },
          arg("box1") = py::none(), arg("box2") = py::none(), returnPolicy)

      .def(
          "boxAND",
          [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
            return box1.has_value() && box2.has_value()
                       ? BoxWrapper(boxAND(*box1, *box2))
                       : BoxWrapper(boxAND());
          },
          arg("box1") = py::none(), arg("box2") = py::none(), returnPolicy)
      .def(
          "boxOR",
          [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
            return box1.has_value() && box2.has_value()
                       ? BoxWrapper(boxOR(*box1, *box2))
                       : BoxWrapper(boxOR());
          },
          arg("box1") = py::none(), arg("box2") = py::none(), returnPolicy)
      .def(
          "boxXOR",
          [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
            return box1.has_value() && box2.has_value()
                       ? BoxWrapper(boxXOR(*box1, *box2))
                       : BoxWrapper(boxXOR());
          },
          arg("box1") = py::none(), arg("box2") = py::none(), returnPolicy)

      .def(
          "boxAbs",
          [](std::optional<BoxWrapper> box1) {
            return BoxWrapper(box1.has_value() ? boxAbs(*box1) : boxAbs());
          },
          arg("box1") = py::none(), returnPolicy)
      .def(
          "boxAcos",
          [](std::optional<BoxWrapper> box1) {
            return BoxWrapper(box1.has_value() ? boxAcos(*box1) : boxAcos());
          },
          arg("box1") = py::none(), returnPolicy)
      .def(
          "boxTan",
          [](std::optional<BoxWrapper> box1) {
            return BoxWrapper(box1.has_value() ? boxTan(*box1) : boxTan());
          },
          arg("box1") = py::none(), returnPolicy)
      .def(
          "boxSqrt",
          [](std::optional<BoxWrapper> box1) {
            return BoxWrapper(box1.has_value() ? boxSqrt(*box1) : boxSqrt());
          },
          arg("box1") = py::none(), returnPolicy)
      .def(
          "boxSin",
          [](std::optional<BoxWrapper> box1) {
            return BoxWrapper(box1.has_value() ? boxSin(*box1) : boxSin());
          },
          arg("box1") = py::none(), returnPolicy)
      .def(
          "boxRint",
          [](std::optional<BoxWrapper> box1) {
            return BoxWrapper(box1.has_value() ? boxRint(*box1) : boxRint());
          },
          arg("box1") = py::none(), returnPolicy)
      .def(
          "boxLog",
          [](std::optional<BoxWrapper> box1) {
            return BoxWrapper(box1.has_value() ? boxLog(*box1) : boxLog());
          },
          arg("box1") = py::none(), returnPolicy)
      .def(
          "boxLog10",
          [](std::optional<BoxWrapper> box1) {
            return BoxWrapper(box1.has_value() ? boxLog10(*box1) : boxLog10());
          },
          arg("box1") = py::none(), returnPolicy)
      .def(
          "boxFloor",
          [](std::optional<BoxWrapper> box1) {
            return BoxWrapper(box1.has_value() ? boxFloor(*box1) : boxFloor());
          },
          arg("box1") = py::none(), returnPolicy)
      .def(
          "boxExp",
          [](std::optional<BoxWrapper> box1) {
            return BoxWrapper(box1.has_value() ? boxExp(*box1) : boxExp());
          },
          arg("box1") = py::none(), returnPolicy)
      .def(
          "boxExp10",
          [](std::optional<BoxWrapper> box1) {
            return BoxWrapper(box1.has_value() ? boxExp10(*box1) : boxExp10());
          },
          arg("box1") = py::none(), returnPolicy)
      .def(
          "boxCos",
          [](std::optional<BoxWrapper> box1) {
            return BoxWrapper(box1.has_value() ? boxCos(*box1) : boxCos());
          },
          arg("box1") = py::none(), returnPolicy)
      .def(
          "boxCeil",
          [](std::optional<BoxWrapper> box1) {
            return BoxWrapper(box1.has_value() ? boxCeil(*box1) : boxCeil());
          },
          arg("box1") = py::none(), returnPolicy)
      .def(
          "boxAtan",
          [](std::optional<BoxWrapper> box1) {
            return BoxWrapper(box1.has_value() ? boxAtan(*box1) : boxAtan());
          },
          arg("box1") = py::none(), returnPolicy)
      .def(
          "boxAsin",
          [](std::optional<BoxWrapper> box1) {
            return BoxWrapper(box1.has_value() ? boxAsin(*box1) : boxAsin());
          },
          arg("box1") = py::none(), returnPolicy)

      .def(
          "boxRemainder",
          [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
            return box1.has_value() && box2.has_value()
                       ? BoxWrapper(boxRemainder(*box1, *box2))
                       : BoxWrapper(boxRemainder());
          },
          arg("box1") = py::none(), arg("box2") = py::none(), returnPolicy)
      .def(
          "boxPow",
          [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
            return box1.has_value() && box2.has_value()
                       ? BoxWrapper(boxPow(*box1, *box2))
                       : BoxWrapper(boxPow());
          },
          arg("box1") = py::none(), arg("box2") = py::none(), returnPolicy)
      .def(
          "boxMin",
          [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
            return box1.has_value() && box2.has_value()
                       ? BoxWrapper(boxMin(*box1, *box2))
                       : BoxWrapper(boxMin());
          },
          arg("box1") = py::none(), arg("box2") = py::none(), returnPolicy)
      .def(
          "boxMax",
          [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
            return box1.has_value() && box2.has_value()
                       ? BoxWrapper(boxMax(*box1, *box2))
                       : BoxWrapper(boxMax());
          },
          arg("box1") = py::none(), arg("box2") = py::none(), returnPolicy)
      .def(
          "boxFmod",
          [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
            return box1.has_value() && box2.has_value()
                       ? BoxWrapper(boxFmod(*box1, *box2))
                       : BoxWrapper(boxFmod());
          },
          arg("box1") = py::none(), arg("box2") = py::none(), returnPolicy)
      .def(
          "boxAtan2",
          [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
            return box1.has_value() && box2.has_value()
                       ? BoxWrapper(boxAtan2(*box1, *box2))
                       : BoxWrapper(boxAtan2());
          },
          arg("box1") = py::none(), arg("box2") = py::none(), returnPolicy)

      .def(
          "boxRec",
          [](BoxWrapper &box1, BoxWrapper &box2) {
            return BoxWrapper(boxRec(box1, box2));
          },
          arg("box1"), arg("box2"),
          "The recursive composition (e.g., A~B) is used to create cycles in "
          "the block-diagram in order to express recursive computations. It is "
          "the most complex operation in terms of connections: "
          "outputs(A)≥inputs(B) and inputs(A)≥outputs(B)",
          returnPolicy)

      .def(
          "boxButton",
          [](std::string &label) { return BoxWrapper(boxButton(label)); },
          arg("label"), returnPolicy)
      .def(
          "boxCheckbox",
          [](std::string &label) { return BoxWrapper(boxCheckbox(label)); },
          arg("label"), returnPolicy)

      .def(
          "boxVSlider",
          [](std::string &label, BoxWrapper &boxInit, BoxWrapper &boxMin,
             BoxWrapper &boxMax, BoxWrapper &boxStep) {
            return BoxWrapper(
                boxVSlider(label, boxInit, boxMin, boxMax, boxStep));
          },
          arg("label"), arg("init"), arg("min"), arg("max"), arg("step"),
          "Create a vertical slider. All the args except the first are "
          "Boxes.",
          returnPolicy)
      .def(
          "boxVSlider",
          [](std::string &label, float init, float minVal, float maxVal,
             float step) {
            return BoxWrapper(boxVSlider(label, boxReal(init), boxReal(minVal),
                                         boxReal(maxVal), boxReal(step)));
          },
          arg("label"), arg("init"), arg("min"), arg("max"), arg("step"),
          "Create a vertical slider. All the args except the first are "
          "floats.",
          returnPolicy)
      .def(
          "boxHSlider",
          [](std::string &label, BoxWrapper &boxInit, BoxWrapper &boxMin,
             BoxWrapper &boxMax, BoxWrapper &boxStep) {
            return BoxWrapper(
                boxHSlider(label, boxInit, boxMin, boxMax, boxStep));
          },
          arg("label"), arg("init"), arg("min"), arg("max"), arg("step"),
          "Create a horizontal slider. All the args except the first are "
          "Boxes.",
          returnPolicy)
      .def(
          "boxHSlider",
          [](std::string &label, float init, float minVal, float maxVal,
             float step) {
            return BoxWrapper(boxHSlider(label, boxReal(init), boxReal(minVal),
                                         boxReal(maxVal), boxReal(step)));
          },
          arg("label"), arg("init"), arg("min"), arg("max"), arg("step"),
          "Create a horizontal slider. All the args except the first "
          "are floats.",
          returnPolicy)

      .def(
          "boxNumEntry",
          [](std::string &label, BoxWrapper &boxInit, BoxWrapper &boxMin,
             BoxWrapper &boxMax, BoxWrapper &boxStep) {
            return BoxWrapper(
                boxNumEntry(label, boxInit, boxMin, boxMax, boxStep));
          },
          arg("label"), arg("init"), arg("min"), arg("max"), arg("step"),
          "Create a num entry. All the args except the first are "
          "Boxes.",
          returnPolicy)
      .def(
          "boxNumEntry",
          [](std::string &label, float init, float minVal, float maxVal,
             float step) {
            return BoxWrapper(boxNumEntry(label, boxReal(init), boxReal(minVal),
                                          boxReal(maxVal), boxReal(step)));
          },
          arg("label"), arg("init"), arg("min"), arg("max"), arg("step"),
          "Create a num entry. All the args except the first are "
          "floats.",
          returnPolicy)

      .def(
          "boxVBargraph",
          [](std::string &label, BoxWrapper &boxMin, BoxWrapper &boxMax,
             BoxWrapper &box) {
            return BoxWrapper(boxVBargraph(label, boxMin, boxMax, box));
          },
          arg("label"), arg("min"), arg("max"), arg("step"), returnPolicy)

      .def(
          "boxHBargraph",
          [](std::string &label, BoxWrapper &boxMin, BoxWrapper &boxMax,
             BoxWrapper &box) {
            return BoxWrapper(boxHBargraph(label, boxMin, boxMax, box));
          },
          arg("label"), arg("min"), arg("max"), arg("step"), returnPolicy)

      .def(
          "boxAttach",
          [](std::optional<BoxWrapper> s1, std::optional<BoxWrapper> s2) {
            return BoxWrapper((s1.has_value() && s2.has_value())
                                  ? boxAttach(*s1, *s2)
                                  : boxAttach());
          },
          arg("box1") = py::none(), arg("box2") = py::none(), returnPolicy)

      .def(
          "boxSampleRate",
          []() {
            return BoxWrapper(boxMin(
                boxReal(192000.0),
                boxMax(boxReal(1.0),
                       boxFConst(SType::kSInt, "fSamplingFreq", "<math.h>"))));
          },
          "Return a box representing the constant sample rate, such as 44100.",
          returnPolicy)
      .def(
          "boxBufferSize",
          []() {
            return BoxWrapper(boxFVar(SType::kSInt, "count", "<math.h>"));
          },
          "Return a box representing the buffer size, such as 1, 2, 4, 8, etc.",
          returnPolicy)
      .def(
          "boxFromDSP",
          [](const std::string &dsp_content) {
            int inputs = 0;
            int outputs = 0;
            std::string error_msg = "";
            const std::string dsp_content2 =
                std::string("import(\"stdfaust.lib\");\n") + dsp_content;
            Box box = DSPToBoxes("dawdreamer", dsp_content2, &inputs, &outputs,
                                 error_msg);
            if (error_msg != "") {
              throw std::runtime_error(error_msg);
            }

            return BoxWrapper(box);
          },
          arg("dsp_code"), "Convert Faust DSP code to a Box.", returnPolicy)

      .def(
          "isBoxNil", [](BoxWrapper &b) { return isNil(b); }, arg("box"))

      .def(
          "isBoxAbstr", [](BoxWrapper &b) { return isBoxAbstr(b); }, arg("box"))

      .def(
          "isBoxAccess",
          [](BoxWrapper &box) {
            Box b2, b3;
            bool res = isBoxAccess(box, b2, b3);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(b2), BoxWrapper(b3));
          },
          arg("box_t"))

      .def(
          "isBoxAppl",
          [](BoxWrapper &box) {
            Box b2, b3;
            bool res = isBoxAppl(box, b2, b3);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(b2), BoxWrapper(b3));
          },
          arg("box_t"))

      .def(
          "isBoxButton",
          [](BoxWrapper &b) {
            Box label;
            bool res = isBoxButton(b, label);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(label));
          },
          arg("box"))

      .def(
          "isBoxCase",
          [](BoxWrapper &b) {
            Box rules;
            bool res = isBoxCase(b, rules);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(rules));
          },
          arg("box"))

      .def(
          "isBoxCheckbox",
          [](BoxWrapper &b) {
            Box label;
            bool res = isBoxCheckbox(b, label);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(label));
          },
          arg("box"))

      .def(
          "isBoxComponent",
          [](BoxWrapper &b) {
            Box filename;
            bool res = isBoxComponent(b, filename);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(filename));
          },
          arg("box"))

      .def(
          "isBoxCut", [](BoxWrapper &b) { return isBoxCut(b); }, arg("box"))

      .def(
          "isBoxEnvironment", [](BoxWrapper &b) { return isBoxEnvironment(b); },
          arg("box"))

      .def(
          "isBoxError", [](BoxWrapper &b) { return isBoxError(b); }, arg("box"))

      .def(
          "isBoxFConst",
          [](BoxWrapper &b) {
            Box type, name, file;
            bool res = isBoxFConst(b, type, name, file);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(type), BoxWrapper(name), BoxWrapper(file));
          },
          arg("box"))

      .def(
          "isBoxFFun",
          [](BoxWrapper &b) {
            Box ffun;
            bool res = isBoxFFun(b, ffun);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(ffun));
          },
          arg("box"))

      .def(
          "isBoxFVar",
          [](BoxWrapper &b) {
            Box type, name, file;
            bool res = isBoxFVar(b, type, name, file);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(type), BoxWrapper(name), BoxWrapper(file));
          },
          arg("box"))

      .def(
          "isBoxHBarGraph",
          [](BoxWrapper &b) {
            Box label, a_min, a_max;
            bool res = isBoxHBargraph(b, label, a_min, a_max);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(label), BoxWrapper(a_min), BoxWrapper(a_max));
          },
          arg("box"))

      .def(
          "isBoxHGroup",
          [](BoxWrapper &b) {
            Box label, x;
            bool res = isBoxHGroup(b, label, x);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(label), BoxWrapper(x));
          },
          arg("box"))

      .def(
          "isBoxHSlider",
          [](BoxWrapper &b) {
            Box label, init, a_min, a_max, step;
            bool res = isBoxHSlider(b, label, init, a_min, a_max, step);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(label), BoxWrapper(init), BoxWrapper(init),
                BoxWrapper(a_min), BoxWrapper(a_max), BoxWrapper(step));
          },
          arg("box"))

      .def(
          "isBoxIdent",
          [](BoxWrapper &b) {
            const char **blah;  // todo: check
            bool res = isBoxIdent(b, blah);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, *blah);
          },
          arg("box"))

      .def(
          "isBoxInputs",
          [](BoxWrapper &b) {
            Box x;
            bool res = isBoxInputs(b, x);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(x));
          },
          arg("box"))

      .def(
          "isBoxInt",
          [](BoxWrapper &b) {
            int i;
            bool res = isBoxInt(b, &i);
            return py::make_tuple<py::return_value_policy::take_ownership>(res,
                                                                           i);
          },
          arg("box"))

      .def(
          "isBoxIPar",
          [](BoxWrapper &b) {
            Box x, y, z;
            bool res = isBoxIPar(b, x, y, z);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(x), BoxWrapper(y), BoxWrapper(z));
          },
          arg("box"))

      .def(
          "isBoxIProd",
          [](BoxWrapper &b) {
            Box x, y, z;
            bool res = isBoxIProd(b, x, y, z);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(x), BoxWrapper(y), BoxWrapper(z));
          },
          arg("box"))

      .def(
          "isBoxISeq",
          [](BoxWrapper &b) {
            Box x, y, z;
            bool res = isBoxISeq(b, x, y, z);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(x), BoxWrapper(y), BoxWrapper(z));
          },
          arg("box"))

      .def(
          "isBoxISum",
          [](BoxWrapper &b) {
            Box x, y, z;
            bool res = isBoxISum(b, x, y, z);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(x), BoxWrapper(y), BoxWrapper(z));
          },
          arg("box"))

      .def(
          "isBoxLibrary",
          [](BoxWrapper &b) {
            Box filename;
            bool res = isBoxLibrary(b, filename);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(filename));
          },
          arg("box"))

      .def(
          "isBoxMerge",
          [](BoxWrapper &b) {
            Box x, y;
            bool res = isBoxMerge(b, x, y);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(x), BoxWrapper(y));
          },
          arg("box"))

      .def(
          "isBoxMetadata",
          [](BoxWrapper &b) {
            Box x, y;
            bool res = isBoxMetadata(b, x, y);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(x), BoxWrapper(y));
          },
          arg("box"))

      .def(
          "isBoxNumEntry",
          [](BoxWrapper &b) {
            Box label, init, a_min, a_max, step;
            bool res = isBoxNumEntry(b, label, init, a_min, a_max, step);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(label), BoxWrapper(init), BoxWrapper(init),
                BoxWrapper(a_min), BoxWrapper(a_max), BoxWrapper(step));
          },
          arg("box"))

      .def(
          "isBoxOutputs",
          [](BoxWrapper &b) {
            Box x;
            bool res = isBoxOutputs(b, x);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(x));
          },
          arg("box"))

      .def(
          "isBoxPar",
          [](BoxWrapper &b) {
            Box x, y;
            bool res = isBoxPar(b, x, y);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(x), BoxWrapper(y));
          },
          arg("box"))

      .def(
          "isBoxPrim0", [](BoxWrapper &b) { return isBoxPrim0(b); }, arg("box"))

      .def(
          "isBoxPrim1", [](BoxWrapper &b) { return isBoxPrim1(b); }, arg("box"))

      .def(
          "isBoxPrim2", [](BoxWrapper &b) { return isBoxPrim2(b); }, arg("box"))

      .def(
          "isBoxPrim3", [](BoxWrapper &b) { return isBoxPrim3(b); }, arg("box"))

      .def(
          "isBoxPrim4", [](BoxWrapper &b) { return isBoxPrim4(b); }, arg("box"))

      .def(
          "isBoxPrim5", [](BoxWrapper &b) { return isBoxPrim5(b); }, arg("box"))

      .def(
          "isBoxReal",
          [](BoxWrapper &b) {
            double r;
            bool res = isBoxReal(b, &r);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, r);
          },
          arg("box"))

      .def(
          "isBoxRec",
          [](BoxWrapper &b) {
            Box x, y;
            bool res = isBoxRec(b, x, y);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(x), BoxWrapper(y));
          },
          arg("box"))

      .def(
          "isBoxRoute",
          [](BoxWrapper &b) {
            Box n, m, r;
            bool res = isBoxRoute(b, n, m, r);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(n), BoxWrapper(m), BoxWrapper(r));
          },
          arg("box"))

      .def(
          "isBoxSeq",
          [](BoxWrapper &b) {
            Box x, y;
            bool res = isBoxSeq(b, x, y);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(x), BoxWrapper(y));
          },
          arg("box"))

      .def(
          "isBoxSlot", [](BoxWrapper &b) { return isBoxSlot(b); }, arg("box"))

      .def(
          "isBoxSoundfile",
          [](BoxWrapper &b) {
            Box label, chan;
            bool res = isBoxSoundfile(b, label, chan);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(label), BoxWrapper(chan));
          },
          arg("box"))

      .def(
          "isBoxSplit",
          [](BoxWrapper &b) {
            Box x, y;
            bool res = isBoxSplit(b, x, y);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(x), BoxWrapper(y));
          },
          arg("box"))

      .def(
          "isBoxSymbolic",
          [](BoxWrapper &b) {
            Box slot, body;
            bool res = isBoxSymbolic(b, slot, body);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(slot), BoxWrapper(body));
          },
          arg("box"))

      .def(
          "isBoxTGroup",
          [](BoxWrapper &b) {
            Box label, x;
            bool res = isBoxSymbolic(b, label, x);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(label), BoxWrapper(x));
          },
          arg("box"))

      .def(
          "boxVBargraph",
          [](std::string &label, BoxWrapper &boxMin, BoxWrapper &boxMax,
             BoxWrapper &box) {
            return BoxWrapper(boxVBargraph(label, boxMin, boxMax, box));
          },
          arg("label"), arg("min"), arg("max"), arg("step"), returnPolicy)

      .def(
          "isBoxVGroup",
          [](BoxWrapper &b) {
            Box label, x;
            bool res = isBoxVGroup(b, label, x);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(label), BoxWrapper(x));
          },
          arg("box"))

      .def(
          "isBoxVSlider",
          [](BoxWrapper &b) {
            Box label, init, a_min, a_max, step;
            bool res = isBoxVSlider(b, label, init, a_min, a_max, step);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(label), BoxWrapper(init), BoxWrapper(init),
                BoxWrapper(a_min), BoxWrapper(a_max), BoxWrapper(step));
          },
          arg("box"))

      .def(
          "isBoxWaveform", [](BoxWrapper &b) { return isBoxWaveform(b); },
          arg("box"))

      .def(
          "isBoxWithLocalDef",
          [](BoxWrapper &b) {
            Box body, ldef;
            bool res = isBoxWithLocalDef(b, body, ldef);
            return py::make_tuple<py::return_value_policy::take_ownership>(
                res, BoxWrapper(body), BoxWrapper(ldef));
          },
          arg("box"))

      .def(
          "getBoxType",
          [](BoxWrapper s1) {
            int inputs, outputs;
            bool result = getBoxType(s1, &inputs, &outputs);
            return py::make_tuple(result, inputs, outputs);
          },
          arg("box"),
          "Return a tuple of (whether the type is valid, number of inputs, "
          "number of outputs) of a box.",
          returnPolicy);
}