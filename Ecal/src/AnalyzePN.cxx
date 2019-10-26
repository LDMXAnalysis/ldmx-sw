/**
 * @file AnalyzePN.cxx
 * @brief Energy histograms to analyze how PN interactions affect showers in ECAL
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Ecal/AnalyzePN.h"

namespace ldmx {

    void AnalyzePN::configure(const ldmx::ParameterSet& ps) {

        simParticlesCollName_ = ps.getString( "simParticlesCollName" );
        simParticlesPassName_ = ps.getString( "simParticlesPassName" );

        ecalDigiCollName_ = ps.getString( "ecalDigiCollName" );
        ecalDigiPassName_ = ps.getString( "ecalDigiPassName" );

        taggerSimHitsCollName_ = ps.getString( "taggerSimHitsCollName" );
        taggerSimHitsPassName_ = ps.getString( "taggerSimHitsPassName" );

        hcalVetoCollName_ = ps.getString( "hcalVetoCollName" );
        hcalVetoPassName_ = ps.getString( "hcalVetoPassName" );

        minPrimaryPhotonEnergy_ = ps.getDouble( "minPrimaryPhotonEnergy" );

        energyCut_ = ps.getDouble( "energyCut" );
        pTCut_     = ps.getDouble( "pTCut"     );

        //constants to determine if event is saved
        lowReconEnergy_ = ps.getDouble( "lowReconEnergy" );
        lowPNEnergy_    = ps.getDouble( "lowPNEnergy"    );

        return;
    }

    void AnalyzePN::analyze(const ldmx::Event& event) {

        //check for Hcal Veto
        //  if veto exists and event fails to pass it ==> skip event
        if ( event.exists( hcalVetoCollName_ , hcalVetoPassName_ ) ) {
            const TClonesArray *hcalVeto = event.getCollection( hcalVetoCollName_ , hcalVetoPassName_ );
            HcalVetoResult *result = (HcalVetoResult *)(hcalVeto->At(0));
            if ( ! result->passesVeto() ) {
                //event failed hcal veto
                //  leave out of analysis
                vetoedEvents_++;
                return;
            }
        }


        const TClonesArray *taggerSimHits = event.getCollection( taggerSimHitsCollName_ , taggerSimHitsPassName_ );
        double preTargetElectronEnergy, preTargetElectronPT;
        bool taggerWouldVeto = electronTaggerEnergy( taggerSimHits , preTargetElectronEnergy , preTargetElectronPT );

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
            } 
       
            if ( (simParticle->getTrackID() == 1 and
                 simParticle->getVertex().at(2) < -500.0 and
                 taggerWouldVeto ) or unphysicalWideAngleBrem( simParticle )
               ) { 
                //something funky happened upstream and primary electron was generated upstream of tagger
                //or particle was an unphysical wide angle brem
                // SKIP EVENT
                skippedEvents_++;
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

            if ( goesPN( simParticle ) ) {

                totalEnergyPN += energy;

                if ( energy > energyHardestPN ) { energyHardestPN = energy; }

            } //particle goes PN and is inside ECAL region

        } //loop through all sim particles

        if ( energyHardestPN < 0.0 ) {
            //no PNs this event
            h_ReconE_NoPN->Fill( ecalReconEnergy );
            energyHardestPN = 0.0; //reset for the histograms with all events in them
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

        if ( totalEnergyPN < lowPNEnergy_ ) {
            
            if ( taggerSimHits->GetEntriesFast() > 0 ) {
                h_ReconE_TaggerElecE->Fill( ecalReconEnergy , preTargetElectronEnergy );
                h_ReconE_TaggerElecPT->Fill( ecalReconEnergy , preTargetElectronPT );
            }

            if ( ecalReconEnergy < lowReconEnergy_ ) {
                //signal region low pn shower - worrisome
                setStorageHint( hint_shouldKeep );
                lowReconLowPN_++;
            }
        }

        return;
    }

    void AnalyzePN::onProcessStart() {

        lowReconLowPN_ = 0;
        skippedEvents_ = 0;
        vetoedEvents_  = 0;

        TDirectory* baseDirectory = getHistoDirectory();

        h_ReconE_TaggerElecE = new TH2F(
                "ReconE_TaggerElecE",
                ";Reconstruced Energy in ECAL [MeV];Energy of Electron in Last Layer of Tagger [MeV]",
                800,0,8000,
                400,0,4000);

        h_ReconE_TaggerElecPT = new TH2F(
                "ReconE_TaggerElecPT",
                ";Reconstruced Energy in ECAL [MeV];p_{T} of Electron in Last Layer of Tagger [MeV/c]",
                800,0,8000,
                400,0,4000);

        TString title;
        title.Form( "Events with Total PN Energy < %.1fMeV" , lowPNEnergy_ );
        h_ReconE_TaggerElecE->SetTitle( title );
        h_ReconE_TaggerElecPT->SetTitle( title );

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
        printf( "| Low PN Events with Recon E < %2.1fGeV : %6d |\n" , lowReconEnergy_/1000.0 , lowReconLowPN_ );
        printf( "| N Events Skipped for Upstream Loss :  %6d |\n" , skippedEvents_ );
        printf( "| N Events Vetoed by HCal            :  %6d |\n" , vetoedEvents_ );
        printf( "================================================\n" );

        return;
    }

    bool AnalyzePN::electronTaggerEnergy( const TClonesArray *taggerSimHits , double &electronE, double &electronPT ) const {

        int nTaggerHits = taggerSimHits->GetEntriesFast();
        electronE = 0.0;
        electronPT = 4000.0;
        for ( int iHit = 0; iHit < nTaggerHits; iHit++ ) {
            
            SimTrackerHit *taggerHit = (SimTrackerHit *)(taggerSimHits->At( iHit ));

            //skip hits that aren't in the last layer
            if ( taggerHit->getLayerID() < 14 ) { continue; }

            SimParticle *particle = taggerHit->getSimParticle();

            //skip hits by not electrons
            if ( particle->getPdgID() != 11 ) { continue; }

            //calculate energy at this hit in the event
            std::vector<double> momentum = taggerHit->getMomentum();
            double pT = sqrt( momentum.at(0)*momentum.at(0) + momentum.at(1)*momentum.at(1) ); 
            double momentum2 = momentum.at(0)*momentum.at(0) + momentum.at(1)*momentum.at(1) + momentum.at(2)*momentum.at(2);
            double energy = sqrt( momentum2 + particle->getMass()*particle->getMass() );

            //check if found primary electron
            if ( energy > electronE ) { electronE = energy; electronPT = pT; }

        } //loop through tagger hits

        return ( electronE < energyCut_ or electronPT > pTCut_ );
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

    bool AnalyzePN::unphysicalWideAngleBrem( const SimParticle *particle ) const {
        
        //only considering photons for this cut
        if ( particle->getPdgID() != 22 ) { return false; }

        //only photons originating in target
        //  target dimensions pulled from v9 gdml file
        std::vector<double> vertex = particle->getVertex();
        if ( abs(vertex.at(2)) > 0.3504/2 or abs(vertex.at(1)) > 95./2. or abs(vertex.at(0)) > 35./2. ) { return false; }

        //check photons in target minimum momentum transfer
        double energy = particle->getEnergy();
        std::vector<double> momentum = particle->getMomentum();

        //theta = arctan( pT / pz )
        double theta = atan( sqrt( momentum.at(0)*momentum.at(0) + momentum.at(1)*momentum.at(1) ) / momentum.at(2) );

        double E0 = 4000.0; //energy of originating electron (assumed to be beam)
        
        //minimum possible momentum transfer for this target photon
        double Qmin = (E0 * energy * theta*theta ) / ( 4 * ( E0 - energy ) );

        //momentum transfer greater than ~1/fermi are considered unphysical
        return ( Qmin > 139.531 );

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

}

DECLARE_ANALYZER_NS(ldmx, AnalyzePN);
