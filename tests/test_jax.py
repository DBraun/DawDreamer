from dawdreamer_utils import *
from dawdreamer.faust.box import *
from box_instruments import *

from typing import List
import inspect
import warnings

from scipy import signal
import numpy as np

from dawdreamer.faust import createLibContext, destroyLibContext

from functools import partial
import itertools
import importlib

BUFFER_SIZE = 1
SAMPLE_RATE = 44100

importlib.util.find_spec("spam")

HAS_JAX = True
try:
    import jax
    import jax.numpy as jnp
    from jax import random
    from flax import linen as nn
    from flax.training import train_state  # Useful dataclass to keep train state
    import optax
except ImportError:
    HAS_JAX = False

def with_lib_context(func):

    """
    The safest way to use either the signal API or box API is to wrap
    the function in a call that creates the lib context and a call
    that destroys the lib context.
    """

    def wrapped(*args, **kwargs):
        createLibContext()
        result = func(*args, **kwargs)
        destroyLibContext()
        return result

    return wrapped

@pytest.mark.skipif(not HAS_JAX, reason="JAX not installed")
def test1():

    createLibContext()

    faust_code = """
    import("stdfaust.lib");
    cutoff = hslider("cutoff", 440., 20., 20000., .01);
    process = fi.lowpass(1, cutoff);
    """

    box, inputs, outputs = boxFromDSP(faust_code)

    module_name = "MyDSP"

    jax_code = boxToSource(box, 'jax', module_name, ['-a', 'jax/minimal.py'])

    destroyLibContext()

    custom_globals = {}

    exec(jax_code, custom_globals)  # security risk!

    MyDSP = custom_globals[module_name]
    # MyDSP is now a class definition which can be instantiated.

    model_single = MyDSP(SAMPLE_RATE)
    CHANNELS_IN = model_single.getNumInputs()

    # Before we bother to vmap the class, which would allow batch-processing,
    # let's jit compile the class for single-item inference:
    jit_inference_fn = jax.jit(partial(model_single.apply, mutable='intermediates'))

    # Note that MyDSP's "forward" method is the following:
    # @nn.compact
    # def __call__(self, x, T: int) -> jnp.array:

    # This takes two arguments, x (an input signal shaped (C, T)), and T the number of samples of the output.
    # In our vmap code, we specify in_axes=(0, None) to batch along x but not along T.
    MyDSP = nn.vmap(MyDSP, in_axes=(0, None), variable_axes={'params': None}, split_rngs={'params': False})

    # Now we can create a model that handles batches of input
    model = MyDSP(SAMPLE_RATE)

    # Use the single-item inferencer

    # T is the number of audio samples of input and output
    T = int(SAMPLE_RATE*1.0)

    input_shape = (CHANNELS_IN, T)

    noise = -1. + 2.*jnp.array(np.random.random(input_shape))

    params = model_single.init({'params': random.PRNGKey(0)}, noise, T)['params']
    audio, mod_vars = jit_inference_fn({'params': params}, noise, T)

    audio = np.array(audio)
    assert np.abs(audio).mean() > .001


if __name__ == "__main__":
    pass
