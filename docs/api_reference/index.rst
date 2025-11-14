API Reference
=============

This section provides detailed API documentation for all DawDreamer classes and functions.

Core Classes
------------

RenderEngine
~~~~~~~~~~~~

.. autoclass:: dawdreamer.RenderEngine
   :members:
   :undoc-members:
   :show-inheritance:

ProcessorBase
~~~~~~~~~~~~~

.. autoclass:: dawdreamer.ProcessorBase
   :members:
   :undoc-members:
   :show-inheritance:

Processors
----------

FaustProcessor
~~~~~~~~~~~~~~

.. autoclass:: dawdreamer.FaustProcessor
   :members:
   :undoc-members:
   :show-inheritance:

PluginProcessor
~~~~~~~~~~~~~~~

.. autoclass:: dawdreamer.PluginProcessor
   :members:
   :undoc-members:
   :show-inheritance:

PlaybackProcessor
~~~~~~~~~~~~~~~~~

.. autoclass:: dawdreamer.PlaybackProcessor
   :members:
   :undoc-members:
   :show-inheritance:

PlaybackWarpProcessor
~~~~~~~~~~~~~~~~~~~~~

.. autoclass:: dawdreamer.PlaybackWarpProcessor
   :members:
   :undoc-members:
   :show-inheritance:

FilterProcessor
~~~~~~~~~~~~~~~

.. autoclass:: dawdreamer.FilterProcessor
   :members:
   :undoc-members:
   :show-inheritance:

CompressorProcessor
~~~~~~~~~~~~~~~~~~~

.. autoclass:: dawdreamer.CompressorProcessor
   :members:
   :undoc-members:
   :show-inheritance:

ReverbProcessor
~~~~~~~~~~~~~~~

.. autoclass:: dawdreamer.ReverbProcessor
   :members:
   :undoc-members:
   :show-inheritance:

PannerProcessor
~~~~~~~~~~~~~~~

.. autoclass:: dawdreamer.PannerProcessor
   :members:
   :undoc-members:
   :show-inheritance:

DelayProcessor
~~~~~~~~~~~~~~

.. autoclass:: dawdreamer.DelayProcessor
   :members:
   :undoc-members:
   :show-inheritance:

AddProcessor
~~~~~~~~~~~~

.. autoclass:: dawdreamer.AddProcessor
   :members:
   :undoc-members:
   :show-inheritance:

SamplerProcessor
~~~~~~~~~~~~~~~~

.. autoclass:: dawdreamer.SamplerProcessor
   :members:
   :undoc-members:
   :show-inheritance:

OscillatorProcessor
~~~~~~~~~~~~~~~~~~~

.. autoclass:: dawdreamer.OscillatorProcessor
   :members:
   :undoc-members:
   :show-inheritance:

Faust API
---------

DawDreamer includes Python bindings for Faust's Box and Signal APIs, allowing programmatic construction of DSP graphs.

The Faust APIs provide two complementary approaches to building audio processors:

* **Box API**: High-level, component-based approach for building audio processors
* **Signal API**: Low-level, signal-processing approach for fine-grained control

See the `Faust Box API Example <https://github.com/DBraun/DawDreamer/tree/main/examples/Box_API>`_ for practical usage.

Faust Module
~~~~~~~~~~~~

The main ``dawdreamer.faust`` module provides:

.. py:class:: dawdreamer.faust.FaustContext

   Context manager for Faust compilation and execution.

.. py:function:: dawdreamer.faust.createLibContext()

   Create a Faust library context.

   :return: Faust library context
   :rtype: FaustContext

.. py:function:: dawdreamer.faust.destroyLibContext(context)

   Destroy a Faust library context.

   :param context: The context to destroy
   :type context: FaustContext

.. py:function:: dawdreamer.faust.boxToSignals(box)

   Convert a Box to Signal representation.

   :param box: A Faust Box object
   :type box: Box
   :return: List of Signal objects
   :rtype: list[Signal]

Faust Box API
~~~~~~~~~~~~~

The Box API provides a functional, component-based approach to building audio processors.

**Core Types:**

.. py:class:: dawdreamer.faust.box.Box

   Base class for all Faust Box objects. Boxes can be combined using standard operators.

   **Operators:**

   * ``box1 + box2`` - Parallel composition
   * ``box1 * box2`` - Sequential composition
   * ``box1 , box2`` - Split composition
   * ``box1 : box2`` - Sequential composition (alternative syntax)

.. py:class:: dawdreamer.faust.box.SType

   Signal type enumeration for Faust boxes.

**Box Construction Functions:**

Audio Primitives
^^^^^^^^^^^^^^^^

.. py:function:: dawdreamer.faust.box.boxInt(n)

   Create an integer constant box.

.. py:function:: dawdreamer.faust.box.boxReal(x)

   Create a real (float) constant box.

.. py:function:: dawdreamer.faust.box.boxWire()

   Create a wire (identity) box that passes input to output unchanged.

.. py:function:: dawdreamer.faust.box.boxCut()

   Create a cut box that terminates a signal.

Math Operations
^^^^^^^^^^^^^^^

.. py:function:: dawdreamer.faust.box.boxAdd(box1, box2)

   Addition: box1 + box2

.. py:function:: dawdreamer.faust.box.boxSub(box1, box2)

   Subtraction: box1 - box2

.. py:function:: dawdreamer.faust.box.boxMul(box1, box2)

   Multiplication: box1 * box2

.. py:function:: dawdreamer.faust.box.boxDiv(box1, box2)

   Division: box1 / box2

.. py:function:: dawdreamer.faust.box.boxAbs(box)

   Absolute value: abs(box)

Trigonometric Functions
^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: dawdreamer.faust.box.boxSin(box)

   Sine function

.. py:function:: dawdreamer.faust.box.boxCos(box)

   Cosine function

.. py:function:: dawdreamer.faust.box.boxTan(box)

   Tangent function

.. py:function:: dawdreamer.faust.box.boxAsin(box)

   Arcsine function

.. py:function:: dawdreamer.faust.box.boxAcos(box)

   Arccosine function

.. py:function:: dawdreamer.faust.box.boxAtan(box)

   Arctangent function

.. py:function:: dawdreamer.faust.box.boxAtan2(box1, box2)

   Two-argument arctangent

Delay and Memory
^^^^^^^^^^^^^^^^

.. py:function:: dawdreamer.faust.box.boxDelay()

   Create a one-sample delay

.. py:function:: dawdreamer.faust.box.boxIntCast(box)

   Cast to integer

.. py:function:: dawdreamer.faust.box.boxFloatCast(box)

   Cast to float

User Interface Elements
^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: dawdreamer.faust.box.boxButton(label)

   Create a button UI element

.. py:function:: dawdreamer.faust.box.boxCheckbox(label)

   Create a checkbox UI element

.. py:function:: dawdreamer.faust.box.boxVSlider(label, init, min, max, step)

   Create a vertical slider

.. py:function:: dawdreamer.faust.box.boxHSlider(label, init, min, max, step)

   Create a horizontal slider

.. py:function:: dawdreamer.faust.box.boxNumEntry(label, init, min, max, step)

   Create a numeric entry field

.. py:function:: dawdreamer.faust.box.boxVBargraph(label, min, max)

   Create a vertical bargraph (output display)

.. py:function:: dawdreamer.faust.box.boxHBargraph(label, min, max)

   Create a horizontal bargraph (output display)

**Example:**

.. code-block:: python

   import dawdreamer as daw
   from dawdreamer.faust import box

   # Create a simple oscillator with volume control
   freq = box.boxHSlider("frequency", 440, 20, 20000, 1)
   gain = box.boxHSlider("volume", 0.5, 0, 1, 0.01)

   # Build the signal chain
   osc = box.boxSin(freq)
   output = box.boxMul(osc, gain)

   # Convert to Faust DSP string and use in processor
   # (See examples/Box_API for complete usage)

Faust Signal API
~~~~~~~~~~~~~~~~

The Signal API provides low-level signal processing primitives.

.. py:class:: dawdreamer.faust.signal.Signal

   Base class for Faust Signal objects representing audio signals.

   **Operators:**

   * ``sig1 + sig2`` - Signal addition
   * ``sig1 * sig2`` - Signal multiplication
   * ``sig1 - sig2`` - Signal subtraction
   * ``sig1 / sig2`` - Signal division

**Signal Construction Functions:**

.. py:function:: dawdreamer.faust.signal.sigInt(n)

   Create an integer constant signal

.. py:function:: dawdreamer.faust.signal.sigReal(x)

   Create a real (float) constant signal

.. py:function:: dawdreamer.faust.signal.sigInput(n)

   Reference the nth input signal

.. py:function:: dawdreamer.faust.signal.sigDelay(sig, delay)

   Delay a signal by specified samples

.. py:function:: dawdreamer.faust.signal.sigIntCast(sig)

   Cast signal to integer

.. py:function:: dawdreamer.faust.signal.sigFloatCast(sig)

   Cast signal to float

**Example:**

.. code-block:: python

   import dawdreamer as daw
   from dawdreamer.faust import signal as sig

   # Create a simple delay effect
   input_sig = sig.sigInput(0)
   delay_time = sig.sigInt(4410)  # 100ms at 44.1kHz
   delayed = sig.sigDelay(input_sig, delay_time)

   # Mix dry and wet
   dry_gain = sig.sigReal(0.7)
   wet_gain = sig.sigReal(0.3)
   output = sig.sigAdd(
       sig.sigMul(input_sig, dry_gain),
       sig.sigMul(delayed, wet_gain)
   )

Further Reading
~~~~~~~~~~~~~~~

* `Faust Box API Tutorial <https://github.com/DBraun/DawDreamer/tree/main/examples/Box_API>`_
* `Faust Official Documentation <https://faustdoc.grame.fr/>`_
* `Faust Signal API Reference <https://faustdoc.grame.fr/manual/syntax/#signals>`_
* :doc:`../user_guide/faust_processor` - Using Faust DSP in DawDreamer

Indices and Tables
------------------

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
