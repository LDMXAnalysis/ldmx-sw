"""Analyze the dark brem events passed to use on the command line."""

def get_dark_brem_products(event) :
    """Get the dark brem participating particles from the event bus

    We simply loop through all the particles stored in the event bus
    and pick out the two particles of importance.
    1. A' - Particle with PDG ID 622
    2. Recoil Electron - Particle with PDG ID 11 and Process Type 11

    Parameters
    ----------
    event : iter(EventTree)
        The current event bus returned as we iterate through event tree
    """
    dark_brem_process_type = 11 # looked up in code
    recoil = None
    aprime = None
    for id_p in e.SimParticles :
        if id_p.second.getProcessType() == dark_brem_process_type :
            if id_p.second.getPdgID() == 622 :
                if aprime is not None :
                    raise Exception(f'More than one A\' have been found in event {e.EventHeader.getEventNumber()}.')
                aprime = id_p.second
            elif id_p.second.getPdgID() == 11 :
                recoil = id_p.second
            else :
                raise Exception(f'Particle {id_p.second.getPdgID()} produced by DB process.')
    if aprime is None :
        raise Exception(f'No A\' found in {e.EventHeader.getEventNumber()}')
    elif recoil is None :
        raise Exception(f'No recoil electron found in {e.EventHeader.getEventNumber()}')

    return recoil, aprime


from LDMX.Framework import EventTree
import sys

tree = EventTree.EventTree(sys.argv[1])

for e in tree :
    recoil, aprime = get_dark_brem_products(e)
    e.EventHeader.Print()
    e.SimParticles.at(1).Print()
    recoil.Print()
    aprime.Print()
    print()
