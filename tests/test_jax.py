import numpy as np
from box_instruments import *
from dawdreamer_utils import *

from dawdreamer.faust import createLibContext, destroyLibContext
from dawdreamer.faust.box import *

SAMPLE_RATE = 44100

HAS_JAX = True
try:
    import jax
    import jax.numpy as jnp
    from flax import nnx
except ImportError:
    HAS_JAX = False


@pytest.mark.skipif(not HAS_JAX, reason="JAX not installed")
def test1():
    createLibContext()

    faust_code = """
    import("stdfaust.lib");
    cutoff = hslider("cutoff", 440., 20., 20000., .01);
    process = fi.lowpass(1, cutoff);
    """

    box = boxFromDSP(faust_code)

    module_name = "MyDSP"

    jax_code = boxToSource(box, "jax", module_name, ["-a", "jax/minimal.py"])

    destroyLibContext()

    custom_globals = {}

    exec(jax_code, custom_globals)  # security risk!

    MyDSP = custom_globals[module_name]

    rngs = nnx.Rngs(0)
    model = MyDSP(SAMPLE_RATE, rngs=rngs)

    CHANNELS_IN = model.num_inputs

    # T is the number of audio samples of input and output
    T = int(SAMPLE_RATE * 1.0)

    input_shape = (CHANNELS_IN, T)

    noise = -1.0 + 2.0 * jnp.array(np.random.random(input_shape))

    model.eval()

    @nnx.jit
    def forward(model, x):
        return model(x)

    audio = forward(model, noise)

    audio = np.array(audio)
    assert np.abs(audio).mean() > 0.001


if __name__ == "__main__":
    pass
