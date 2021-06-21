"""Python module for configuring translators"""

from LDMX.Framework import ldmxcfg

class Translator() :
    """Base class for all translator configuration classes

    Does the bare-minimum of including the library the translators
    are compiled into and defining the class name that is necessary
    to determine which C++ translator to configure.
    """

    def __init__(self, class_name) :
        ldmxcfg.Process.addModule('Packing::Translators')
        self.class_name = f'packing::translators::{class_name}'

class Hgcroc(Translator) :
    """Configuration for Hgcroc Translator configuration

    Parameters
    ----------
    pass_regex : str
        Regular expression for pass name of objects to decode/encode
    object_regex : str
        Regular expression for names of objects to decode/encode
    """

    def __init__(self, pass_regex, object_regex) :
        super().__init__('Hgcroc')
        
        self.pass_regex = pass_regex
        self.object_regex = object_regex
