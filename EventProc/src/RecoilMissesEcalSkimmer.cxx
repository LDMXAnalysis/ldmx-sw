/**
 * @file RecoilMissesEcalSkimmer.cxx
 * @brief Processor used to select events where the recoil electron misses the
 *        Ecal. 
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "EventProc/RecoilMissesEcalSkimmer.h"

namespace ldmx { 
    
    RecoilMissesEcalSkimmer::RecoilMissesEcalSkimmer(const std::string &name, Process &process):
        Producer(name, process) { 
    }

    RecoilMissesEcalSkimmer::~RecoilMissesEcalSkimmer() { 
    }

    void RecoilMissesEcalSkimmer::configure(const ParameterSet &pset) { 
    }

    void RecoilMissesEcalSkimmer::produce(Event &event) { 
        
        // Get the collection of sim particles from the event 
        const std::map<int,SimParticle> simParticleMap = event.getObject<std::map<int,SimParticle>>("SimParticles");
        if (simParticleMap.size() == 0) return; 

        const SimParticle *recoilElectron = Analysis::getRecoil( simParticleMap );

        // Get the collection of simulated Ecal hits from the event. 
        const std::vector<SimCalorimeterHit> ecalSimHits = event.getCollection<SimCalorimeterHit>(EventConstants::ECAL_SIM_HITS);
       
        // Loop through the Ecal hits and check if the recoil electron is 
        // associated with any of them.  If there are any recoil electron hits
        // in the Ecal, drop the event.
        bool hasRecoilElectronHits = false; 
        for (const SimCalorimeterHit &simHit : ecalSimHits ) {
            
            /*std::cout << "[ RecoilMissesEcalSkimmer ]: "  
                      << "Number of hit contributions: "  
                      << simHit->getNumberOfContribs() << std::endl;*/

            for (int iContrib = 0; iContrib < simHit.getNumberOfContribs(); ++iContrib) {

                SimCalorimeterHit::Contrib contrib = simHit.getContrib(iContrib);

                if (contrib.trackID == recoilElectron->getTrackID()) { 
                    /*std::cout << "[ RecoilMissesEcalSkimmer ]: " 
                              << "Ecal hit associated with recoil electron." << std::endl; */
                    
                    hasRecoilElectronHits = true;
                } 
            } 
        }
       
        // Tell the skimmer to keep or drop the event based on whether there
        // were recoil electron hits found in the Ecal. 
        if (hasRecoilElectronHits) { 
            setStorageHint(hint_shouldDrop); 
        } else { 
            setStorageHint(hint_shouldKeep); 
        }
    }
}

DECLARE_PRODUCER_NS(ldmx, RecoilMissesEcalSkimmer);
