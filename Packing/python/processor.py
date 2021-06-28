"""Configuration for the packing processor"""

from LDMX.Framework import ldmxcfg

class Unpacker(Processor) :
    """Processor to use to unpack from raw to digi.

    Parameters
    ----------
    translators : list[Translator]
        List of translators for decoding
    """

    def __init__(self, raw_name, translators = []) :
        super().__init__('unpacker','packing::Unpacker','Packing')
        self.translators = translators
        self.raw_name = raw_name
        self.raw_pass = ''
