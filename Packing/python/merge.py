"""Python module to merge simultaneous raw files
into a single ROOT file with many branches of encoded
data.

Open Questions
--------------
- Is there a uniform word (or multi-word) condition that is used to
  separate events across all subsystems?
- Is it ok to assume that all input raw files have the same number
  of events?
- Where will the RunHeader/EventHeader information come from?
"""

import ROOT

LDMXBufferType = ROOT.std.vector('unsigned char')
LDMXEventEndWord = 0b00000000

def build(self, streams, output_file = 'raw_events.root') :
    
    # convert file names to binary files being read
    for name in streams :
        streams[name] = open(streams[name], 'rb')
    
    # start reading in data
    tfile = ROOT.TFile(output_file, 'recreate')
    event_tree = ROOT.TTree('LDMX_RawData','LDMX_RawData')
 
    raw_data = ROOT.std.map(ROOT.std.string,LDMXBufferType)()
    event_tree.Branch('data', ROOT.addressof(raw_data))

    while True :
        raw_data.clear()
        done = True
        for name, buff in streams.items() :
            cpp_obj_buffer = LDMXBufferType()
            while True :
                byte = buff.read(1)
                if not byte or byte[0] == LDMXEventEndWord :
                    break
                cpp_obj_buffer.push_back(byte[0])

            if not cpp_obj_buffer.empty() :
                done = False
 
            raw_data[name] = cpp_obj_buffer

        if done :
            break
 
        event_tree.Fill()

    run_tree = ROOT.TTree('LDMX_RawRun','LDMX_RawRun')

    # Run Data

    tfile.Write()
    tfile.Close()

    for f in streams.values() :
        f.close()
