"""Configuration for the packing processor"""

from LDMX.Framework import ldmxcfg

class Processor(ldmxcfg.Producer) :
    """The base packing processor configuration class.

    Do not interact with this directly.

    The instance name of the processor is determined
    by the decode parameter.

    Parameters
    ----------
    decode : bool
        True if we are decoding, false if encoding
    translators : list[Translator]
        List of translators for encoding/decoding
    """
    def __init__(self, decode, translators = []) :
        if decode :
            name = 'unpacker'
        else :
            name = 'packer'

        super().__init__(name,'packing::Processor','Packing')

        self.decode = decode
        self.translators = translators

class Unpack(Processor) :
    """Processor to use to unpack from raw to digi.

    Parameters
    ----------
    translators : list[Translator]
        List of translators for decoding
    """

    def __init__(self, translators = []) :
        super().__init__(True, translators)

class Pack(Processor) :
    """Processor to use to pack digi into raw.

    Parameters
    ----------
    translators : list[Translator]
        List of translators for encoding
    """

    def __init__(self, translators = []) :
        super().__init__(False, translators)
