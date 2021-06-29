"""Module to re-form data to our input format

The output of the hexactrl-sw we are using to test an HGC ROC
is a certain style of ROOT TTree. The "raw" file output by
this software is not really raw - it has been interpreted 
and reserialized by Boost.Serialization so it is not close
to what we actually expect to come off the detector.
"""

import ROOT

def reformat(root_hexactrl_sw, out_name = 'LDMX_RawData.root') :
    infile = ROOT.TFile(root_hexactrl_sw)
    intree = infile.Get('unpacker_data/hgcroc')

    current_event = 0
    for entry in intree :
        if entry.event != current_event :
            outtree.Fill()
            raw_data.clear()

        current_event = entry.event
        eid = chip + half + channel
        word = 0 << 31 + 0 << 30 + adc << 20 + tot << 10 + toa


