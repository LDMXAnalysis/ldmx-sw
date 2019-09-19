/**
 * @file AnalyzePN.cxx
 * @brief Energy histograms to analyze how PN interactions affect showers in ECAL
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Ecal/AnalyzePN.h"

namespace ldmx {

    void AnalyzePN::configure(const ldmx::ParameterSet& ps) {

        simParticlesCollName_ = ps.getString( "simParticlesCollName" , "SimParticles" );
        simParticlesPassName_ = ps.getString( "simParticlesPassName" , "sim" );

        ecalDigiCollName_ = ps.getString( "ecalDigiCollName" , "ecalDigis" );
        ecalDigiPassName_ = ps.getString( "ecalDigiPassName" , "" );

        minPrimaryPhotonEnergy_ = ps.getDouble( "minPrimaryPhotonEnergy" );
        upstreamLossThresh_ = ps.getDouble( "upstreamLossThresh" , 0.95 );

        return;
    }

    void AnalyzePN::analyze(const ldmx::Event& event) {

        const TClonesArray *ecalDigiHits = event.getCollection( ecalDigiCollName_ , ecalDigiPassName_ );
        double ecalReconEnergy = calculateReconEnergy( ecalDigiHits );

        const TClonesArray *allSimParticles = event.getCollection( simParticlesCollName_ , simParticlesPassName_ );

        int nSimParticles = allSimParticles->GetEntriesFast();
        double energyHardestPN = -5.0; //start with negative so if there are no PNs, it goes in the pure EM bin
        double totalEnergyPN = 0.0;
        SimParticle *primaryPhoton = nullptr; //photon with highest energy
        for ( int iSP = 0; iSP < nSimParticles; iSP++ ) {
            SimParticle *simParticle = static_cast<SimParticle *>(allSimParticles->At( iSP ));

            if ( !simParticle ) {
                std::cerr << "OOPS! Loaded a nullptr as the sim particle!" << std::endl;
                continue;
            } else if ( isUpstreamLoss( simParticle ) ) { 
                //primary electron lost too much energy
                //==> ECAL missed a lot of energy BUT tagger would veto easily
                // SKIP EVENT
                return;
            }

            double energy = simParticle->getEnergy();

            if ( simParticle->getPdgID() == 22 ) {
                if ( (!primaryPhoton and energy > minPrimaryPhotonEnergy_ ) or
                     (primaryPhoton and energy > primaryPhoton->getEnergy())
                    ) {
                    primaryPhoton = simParticle;
                }
            }

            std::vector<double> startPoint = simParticle->getVertex();
            if ( goesPN( simParticle ) ) {

                totalEnergyPN += energy;

                if ( energy > energyHardestPN ) { energyHardestPN = energy; }

            } //particle goes PN and is inside ECAL region

        } //loop through all sim particles

        if ( energyHardestPN < 0.0 ) {
            //no PNs this event
            h_ReconE_NoPN->Fill( ecalReconEnergy );
            energyHardestPN = 0.0; //reset for the histograms with all events in them
            if ( ecalReconEnergy < 2000. ) {
                //signal region pure em shower - worrisome
                setStorageHint( hint_shouldKeep );
                lowReconPureEM_++;
            }
        } else if ( primaryPhoton and goesPN( primaryPhoton ) ) {
            //primary photon went PN
            h_ReconE_PrimPhoton->Fill( ecalReconEnergy );
        } else {
            //nothing interesting - primary photon did not go PN but something did go PN
            h_ReconE_HardestPN_NotSpecial->Fill( ecalReconEnergy , energyHardestPN );
            h_ReconE_TotalPN_NotSpecial  ->Fill( ecalReconEnergy , totalEnergyPN );
        }

        h_ReconE_HardestPN_All->Fill( ecalReconEnergy , energyHardestPN );
        h_ReconE_TotalPN_All  ->Fill( ecalReconEnergy , totalEnergyPN );

        return;
    }

    void AnalyzePN::onProcessStart() {

        lowReconPureEM_ = 0;

        TDirectory* baseDirectory = getHistoDirectory();

        h_ReconE_HardestPN_All = new TH2F(
                "ReconE_HardestPN_All",
                ";Reconstructed Energy in ECAL [MeV];Energy of Hardest Photon Going PN [MeV]",
                800,0,8000,
                400,0,4000);

        h_ReconE_TotalPN_All = new TH2F(
                "ReconE_TotalPN_All",
                ";Reconstructed Energy in ECAL [MeV];Total Energy of Photons Going PN [MeV]",
                800,0,8000,
                400,0,4000);

        h_ReconE_HardestPN_NotSpecial = new TH2F(
                "ReconE_HardestPN_NotSpecial",
                "Excluding NoPN and PrimaryPhoton Events;Reconstructed Energy in ECAL [MeV];Energy of Hardest Photon Going PN [MeV]",
                800,0,8000,
                400,0,4000);

        h_ReconE_TotalPN_NotSpecial = new TH2F(
                "ReconE_TotalPN_NotSpecial",
                "Excluding NoPN and PrimaryPhoton Events;Reconstructed Energy in ECAL [MeV];Total Energy of Photons Going PN [MeV]",
                800,0,8000,
                400,0,4000);

        h_ReconE_NoPN = new TH1F(
                "ReconE_NoPN",
                "Only Events without any PN interactions;Reconstructed Energy in ECAL [MeV]",
                800,0,8000);

        h_ReconE_PrimPhoton = new TH1F( 
                "ReconE_PrimPhoton",
                "Only Events with primary photon going PN;Reconstructed Energy in ECAL [MeV]",
                800,0,8000);

        return;
    }

    void AnalyzePN::onProcessEnd() {

        printf( "================================================\n" );
        printf( "| Mid-Shower PN Analyzer                       |\n" );
        printf( "|----------------------------------------------|\n" );
        printf( "| Pure EM Events with Recon E < 2.0GeV : %5d |\n" , lowReconPureEM_ );
        printf( "================================================\n" );

        return;
    }

    double AnalyzePN::calculateReconEnergy( const TClonesArray *ecalHitColl ) const {

        double ecalTotalEnergy = 0;
        for(int i=0; i < ecalHitColl->GetEntriesFast(); i++) {
            EcalHit* ecalhit = (EcalHit*)(ecalHitColl->At(i));
            if ( ! ecalhit->isNoise() ) { //Only add non-noise hits
                ecalTotalEnergy += ecalhit->getEnergy();
            }
        }
        
        return ecalTotalEnergy;
    }

    bool AnalyzePN::goesPN( const SimParticle *particle ) const {
        
        int nChildren = particle->getDaughterCount();
        for ( int iChild = 0; iChild < nChildren; iChild++ ) {
            SimParticle *child = particle->getDaughter( iChild );

            //need to check if child pointer is not NULL
            //  pointer to child is a TRef so it will be NULL unless the child was also saved and loaded in
            if ( child and child->getProcessType() == SimParticle::ProcessType::photonNuclear ) {
                return true;
            }
        }

        return false;
    }

    bool AnalyzePN::isUpstreamLoss( const SimParticle *particle ) const {

        //should only care about checking primary electron
        if ( particle->getTrackID() > 1 ) { return false; }
        
        std::vector<double> endPoint = particle->getEndPoint();

        //check that endPoint is BEFORE target
        //  target is centered at z=0 and has thickness < 1mm
        //  Geant4 also seems to have the point z=-5000 mean something special because it comes up a lot
        //      I'm excluding that too because I don't understand it ==> maybe means escaped to edge of world volume?
        if ( endPoint.at(2) < -1.0 and endPoint.at(2) > -4999.9 ) {
            //primary electron "ends" before target
            //check if there is an electron daughter with energy high enough
            //    to take over title of primary
            int nChildren = particle->getDaughterCount();
            SimParticle *inheritPrimary = nullptr;
            for ( int iChild = 0; iChild < nChildren; iChild++ ) {
                SimParticle *child = particle->getDaughter( iChild );

                if ( child and child->getPdgID() == 11 ) {
                    //electron child
                    if ( ! inheritPrimary or inheritPrimary->getEnergy() < child->getEnergy() ) {
                        //electron child has larger energy than previous inheriter
                        inheritPrimary = child;
                    }
                }
            }//loop through children

            if ( inheritPrimary ) { 
                //check if inheriting primary electron is below threshhold
                return inheritPrimary->getEnergy() < upstreamLossThresh_*particle->getEnergy(); 
            } else { 
                //no inheriter but ended before target ==> upstream loss
                return true; 
            }

        }//ending before target

        //high enough energy and ends after target ==> no upstream loss
        return false;
    }
}

DECLARE_ANALYZER_NS(ldmx, AnalyzePN);
