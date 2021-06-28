"""Python module to merge simultaneous raw files
into a single ROOT file with many branches of encoded
data."""

import ROOT

LDMXBufferType = ROOT.std.vector('unsigned char')
LDMXEventEndWord = 0b00000000

class RawDataStream(file) :
    """Each 'stream' of raw data is housed in an instance
    of this class. We pretend streams/files of raw data are
    stacks in order of events and each stream/file has exactly
    one block of data per event that can be uniquely identified.

    Parameters
    ----------
    data : str
        Name of file with binary data inside of it
    name : str
        Unique identification name for this data stream
    """

    def __init__(self, data, name) :
        super().__init__(data, 'rb')
        self.__name = name

    def name(self) :
        """Get the name of this data stream"""
        return self.__name

    def pop(self) :
        """Get the next chunk of data to be inserted into the map.

        We keep reading 8-bit words until we reach a stopping condition.
        """  

        cpp_obj_buffer = LDMXBufferType()

        while True:
            [word] = self.read(1)
            if word == LDMXEventEndWord :
                break
            cpp_obj_buffer.push_back(word)

        return cpp_obj_buffer

class EventBuilder() :
    """Here we hold multiple raw data streams and merge them
    event-by-event while writing out a *correctly formatted*
    event file to be given to ldmx-sw."""

    def __init__(self, output_file = 'raw_events.root') :
        self.streams = {}

        self.tfile = ROOT.TFile(output_file, 'recreate')
        self.tree = ROOT.TTree('LDMX_Events','LDMX_Events')

        self.raw_data = ROOT.std.map(ROOT.std.string,LDMXBufferType)()
        self.tree.Branch('RawData_raw', ROOT.addressof(raw_data))

    def next(self) :
        """Iterate one more event, grabbing one more event from each stream"""

        self.raw_data.clear()

        for key, val in self.streams :
            self.raw_data[key] = val.pop()

        self.tree.Fill()
