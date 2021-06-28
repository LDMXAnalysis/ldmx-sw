"""Python module to merge simultaneous raw files
into a single ROOT file with many branches of encoded
data."""

import ROOT

LDMXBufferType = ROOT.std.vector('unsigned char')
LDMXEventEndWord = 0b00000000

def build(self, streams, output_file = 'raw_events.root', output_name = 'RawData', output_pass = 'raw') :
    
    # convert file names to binary files being read
    for name in streams :
        streams[name] = open(streams[name], 'rb')
    
    # start reading in data
    tfile = ROOT.TFile(output_file, 'recreate')
    tree  = ROOT.TTree('LDMX_Events','LDMX_Events')
 
    raw_data = ROOT.std.map(ROOT.std.string,LDMXBufferType)()
    tree.Branch('RawData_raw', ROOT.addressof(raw_data))
 
    while True :
        raw_data.clear()
        done = True
        for name, buff in streams.items() :
            cpp_obj_buffer = LDMXBufferType()
            byte = buff.read(1)
            while byte :
                if byte[0] == LDMXEventEndWord :
                    break
                cpp_obj_buffer.push_back(byte[0])

            if not cpp_obj_buffer.empty() :
                done = False
 
            raw_data[name] = cpp_obj_buffer

        if done :
            break
 
        tree.Fill()
 
    tfile.Write()
    tfile.Close()

    for name in streams :
        streams[name].close()
