/**
 * @file HcalHitMatcher.cxx
 * @brief The purpose of this analyzer is to study vetoes caused by activity in the Hcal using 
 *        Monte Carlo simulations.
 * @author Matthew Forsman , Tom Eichlersmith
 */

#include "Hcal/HcalHitMatcher.h"

namespace ldmx {

    void HcalHitMatcher::configure(const ldmx::ParameterSet& ps) { 
        
        EcalHitColl_ = ps.getString( "EcalHitCollectionName" , "ecalDigis" );
        HcalHitColl_ = ps.getString( "HcalHitCollectionName" , "hcalDigis" );
        EcalScoringPlane_ = ps.getString( "EcalScoringPlaneHitsName" , "EcalScoringPlaneHits" ); 
        HcalScoringPlane_ = ps.getString( "HcalScoringPlaneHitsName" , "HcalScoringPlaneHits" );

        minLayerEventMaxPE_ = ps.getInteger( "minLayerEventMaxPE" );

        ecalFrontZ_ = ps.getDouble( "ecalFrontZ" );

        return;
    }

    void HcalHitMatcher::analyze(const ldmx::Event& event) {

        numEvents_++;

        //Measure total energy in ECAL
        const TClonesArray* ecalHitColl = event.getCollection( EcalHitColl_ ); 
        double ecalTotalEnergy = calculateEcalSummedEnergy( ecalHitColl );

        //Scoring plane information
        const TClonesArray* ecalScoringPlaneHits = event.getCollection( EcalScoringPlane_ );
        std::vector<SimTrackerHit *> simTrackerHits_LeavingScoringPlane;
        getParticlesLeavingEcalScoringPlane( ecalScoringPlaneHits , ecalTotalEnergy , simTrackerHits_LeavingScoringPlane );

        //Map HcalHits to pdgIDs
        std::map< int , std::vector<SimCalorimeterHit *> > rawID_simHits;
        const TClonesArray* hcalSimHits = event.getCollection( EventConstants::HCAL_SIM_HITS , "sim" );
        for( int iHit = 0; iHit < hcalSimHits->GetEntriesFast(); iHit++ ) {
            SimCalorimeterHit* simHit = (SimCalorimeterHit *)(hcalSimHits->At(iHit));

            if ( simHit->getNumberOfContribs() != 1 ) {
                std::cout << "I didn't think this would happen! There are " << simHit->getNumberOfContribs()
                    << " which isn't equal to 1!" << std::endl;
                continue;
            }

            int pdgID = simHit->getContrib(0).pdgCode;

            if ( abs(pdgID) == 12 or abs(pdgID) == 14 ) continue; //skip neutrinos

            int rawID = simHit->getID();

            rawID_simHits[ rawID ].push_back( simHit );
        }

        //HcalHit information
        const TClonesArray* hcalHitColl = event.getCollection( HcalHitColl_ );

        float max_PE_of_event=0;
        float max_PE_of_event_excluded=0;
        int nBackHcalHits = 0;
        int nSideHcalHits = 0;
        for(int i=0; i < hcalHitColl->GetEntriesFast(); i++) { //Begin loop over hcalhits array
            ldmx::HcalHit* hcalhit = (ldmx::HcalHit*)(hcalHitColl->At(i));
            
            if ( ! hcalhit->getNoise() ) { //Only analyze non-noise hits
                
                numNonNoiseHits_++;

                //---- Bin HcalHit information that does not depend on mathcin -------------------->
                
                double hcalhit_radialdist = sqrt( pow(hcalhit->getX(), 2) + pow(hcalhit->getY(), 2) );

                h_HcalHit_ZbyR_All->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist);

                int section = hcalhit->getSection();
                int layer = hcalhit->getLayer();
                if ( section == 0 ) {
                    h_HcalHit_Depth_Back->Fill( ecalTotalEnergy , layer );
                    nBackHcalHits++;
                } else if ( section > 0 and section < 5 ) {
                    h_HcalHit_Z_Side->Fill( ecalTotalEnergy , hcalhit->getZ()-ecalFrontZ_ );
                    h_HcalHit_Depth_Side->Fill( ecalTotalEnergy , layer );
                    nSideHcalHits++;
                } else {
                    std::cerr << "[ Warning ] : HcalHitMatcher::analyze - found HcalSection " << section
                        << " that is not in the correct range." << std::endl;
                }

                float pe = hcalhit->getPE();
                h_HcalHit_PE_All->Fill( ecalTotalEnergy , pe );
                if( pe > max_PE_of_event ) {
                    max_PE_of_event = pe;
                    if ( layer > minLayerEventMaxPE_ ) {
                        max_PE_of_event_excluded = pe;
                    }
                }

                int rawID = hcalhit->getID();
                if ( rawID_simHits.find( rawID ) != rawID_simHits.end() ) {
                    h_HcalHit_NContribs->Fill( ecalTotalEnergy , rawID_simHits.at( rawID ).size() );
                    for ( SimCalorimeterHit *simHit : rawID_simHits.at( rawID ) ) {
                        
                        int pdgID = simHit->getContrib(0).pdgCode;
                        TString pdgStr( "Nuclei" );
                        if ( pdgID < 1000000000 ) {
                            pdgStr.Form( "%d" , pdgID );
                        }

                        //weight by energy deposition
                        h_HcalHit_IDs->Fill( ecalTotalEnergy , pdgStr , 
                            simHit->getEdep()/hcalhit->getEnergy() );

                        if ( section > 0 and section < 5 ) {
                            h_HcalHit_Depth_Side_byID->Fill( pdgStr , layer ,
                                    simHit->getEdep() / hcalhit->getEnergy() );
                        }
                    }//simHits contributing to this hcalhit
                } else {
                    std::cout << "Yikes! Found an HcalHit that doesn't have a corresponding SimCalorimeterHit!" << std::endl;
                    numUnMatchedHits_++;
                }
    
            } // if not a noise hit

        }//End loop over hcalhits array

        // maximum PE in hcal hits for the event
        h_EventMaxPE_All     ->Fill( ecalTotalEnergy , max_PE_of_event );
        h_EventMaxPE_Excluded->Fill( ecalTotalEnergy , max_PE_of_event_excluded );

        // number of hcal hits for the event
        h_NumHcalHits     ->Fill( ecalTotalEnergy , nBackHcalHits+nSideHcalHits );
        h_NumHcalHits_Back->Fill( ecalTotalEnergy , nBackHcalHits );
        h_NumHcalHits_Side->Fill( ecalTotalEnergy , nSideHcalHits );

        return;
    } //analyze

    void HcalHitMatcher::onProcessStart() {
        
        //Readin default database installed with ROOTSYS
        databasePDG_.ReadPDGTable();

        numNonNoiseHits_ = 0;
        numUnMatchedHits_ = 0;
        numEvents_ = 0;

        // Make directory tree to organize histograms in
        TDirectory* d_base = getHistoDirectory();

        h_EcalSummedEnergy = new TH1F(
                "EcalSummedEnergy",
                "Ecal Summed Energy;Total Measured Energy in ECAL [MeV];Count",
                800,0,8000); //10 MeV Bins

        h_NumHcalHits = new TH2F(
                "NumHcalHits",
                ";EcalSummedEnergy;Number of HcalHits per Event;Count",
                800,0,8000,
                50,0,50);

        h_NumHcalHits_Back = new TH2F(
                "NumHcalHits_Back",
                ";EcalSummedEnergy;Number of Hits in Back HCAL per Event;Count",
                800,0,8000,
                50,0,50);

        h_NumHcalHits_Side = new TH2F(
                "NumHcalHits_Side",
                ";EcalSummedEnergy;Number of Hits in Side HCAL per Event;Count",
                800,0,8000,
                50,0,50);

        h_NumParticles = new TH2F(
                "NumParticles" ,
                ";EcalSummedEnergy;Number of Particles that Crossed the ECAL Scoring Plane;Count",
                800,0,8000,
                10,0,10);

        h_EventMaxPE_All = new TH2F(
                "EventMaxPE_All",
                ";EcalSummedEnergy;Maximum PE for all HcalHits in Event;Count",
                800,0,8000,
                500,0,500);

        TString title;
        title.Form( ";EcalSummedEnergy;Maximum PE for Hits with Layer Index > %d; Count" , minLayerEventMaxPE_ );
        h_EventMaxPE_Excluded = new TH2F(
                "EventMaxPE_Excluded",
                title,
                800,0,8000,
                500,0,500);
        
        //add in bins of known particles
        std::vector<std::string> knownPDGs = { "22" , "11" , "-11" , "13" , "-13", 
            "2112" , "2212" , "211", "-211" , "130", "321" , "Nuclei" };
        h_Particle_ID = new TH2F(
                "Particle_ID",
                ";EcalSummedEnergy;Particle Crossing ECAL Scoring Plane;Count",
                800,0,8000,
                knownPDGs.size(),0, knownPDGs.size() );

        h_Particle_Energy = new TH2F(
               "Particle_Energy",
               ";EcalSummedEnergy;Particle Energy [MeV];Count",
               800,0,8000,
               400,0,4000);

        h_Particle_Kinetic = new TH2F(
               "Particle_Kinetic",
               ";EcalSummedEnergy;Particle Kinetic Energy [MeV];Count",
               800,0,8000,
               400,0,4000);

        h_HcalHit_Depth_Side = new TH2F(
               "HcalHit_Depth_Side",
               ";EcalSummedEnergy;Depth of Hits in Side HCAL [layer index];Count",
               800,0.,8000.,
               35, 0, 35);

        h_HcalHit_Depth_Side_byID = new TH2F(
               "HcalHit_Depth_Side_byID",
               ";Particle Contributing to Hit;Depth of Hit in Side HCAL [layer index];Count",
               knownPDGs.size(),0,knownPDGs.size(),
               35, 0, 35);

        h_HcalHit_Depth_Back = new TH2F(
               "HcalHit_Depth_Back",
               ";EcalSummedEnergy;Depth of Hits in Back HCAL [layer index];Count",
               800,0,8000,
               100,0,100);

        h_HcalHit_Z_Side = new TH2F(
               "HcalHit_Z_Side",
               ";EcalSummedEnergy;Z Location of Hits in Side HCAL [mm];Count",
               800,0,8000,
               600,0,600);

        h_HcalHit_IDs = new TH2F(
                "HcalHit_IDs",
                ";EcalSummedEnergy;Particles Blamed by Simulation;Count",
                800,0,8000,
                knownPDGs.size(),0,knownPDGs.size() );

        h_HcalHit_NContribs = new TH2F(
                "HcalHit_NContribs",
                ";EcalSummedEnergy;Number of Contributors to the Hit;Count",
                800,0,8000,
                50,0.5,50.5);

        h_HcalHit_ZbyR_All = new TH3F(
               "HcalHit_ZbyR_All", 
               "All Hcal Hit Locations;EcalSummedEnergy;Z depth [mm];radial distance from z-axis [mm]",
               800,0,8000,
               500,0,5000,
               220,0,2200);

        h_HcalHit_PE_All = new TH2F(
               "HcalHit_PE_All",
               ";EcalSummedEnergy;PEs of all HcalHits;Count",
               800,0,8000,
               500,0,500);

        //label PDG bins
        for ( int ibin = 1; ibin < knownPDGs.size()+1; ibin++ ) {
            h_Particle_ID            ->GetYaxis()->SetBinLabel( ibin , knownPDGs.at(ibin-1).c_str() );
            h_HcalHit_IDs            ->GetYaxis()->SetBinLabel( ibin , knownPDGs.at(ibin-1).c_str() );
            h_HcalHit_Depth_Side_byID->GetXaxis()->SetBinLabel( ibin , knownPDGs.at(ibin-1).c_str() );
        }

        return;
    } //onProcessStart
    
    void HcalHitMatcher::onProcessEnd() {
        
        double hitRate;
        {
            //temporary variables for calculating rates
            double numerator = numNonNoiseHits_;
            double denominator = numEvents_;
            hitRate = numerator/denominator;
        }

        printf( "\n" );
        printf( "===================================\n" );
        printf( "=           HcalHitMatcher        =\n" );
        printf( "===================================\n" );
        printf( "Number of Events         : %8i\n" , numEvents_ );
        printf( "Number of Non Noise Hits : %8i\n" , numNonNoiseHits_ );
        printf( "Number of Unmatched Hits : %8i\n" , numUnMatchedHits_ );
        printf( "Hit Rate (hits/events)   : %8.6f\n" , hitRate );
        printf( "===================================\n" );
        printf( "      PDG ID |  Number : Event Rate\n" );
        for ( std::pair<const int,long int> &pdg_N : numParticles_ ) {
            printf( " %11i | %7ld : %10.6f\n" , pdg_N.first , pdg_N.second , ((double)pdg_N.second)/numEvents_ );
        }
        printf( "===================================\n" );

        return;
    }

    double HcalHitMatcher::calculateEcalSummedEnergy( const TClonesArray *ecalHitColl ) {

        double ecalTotalEnergy = 0;
        for(int i=0; i < ecalHitColl->GetEntriesFast(); i++) {
            ldmx::EcalHit* ecalhit = (ldmx::EcalHit*)(ecalHitColl->At(i));
            if ( ! ecalhit->isNoise() ) { //Only add non-noise hits
                ecalTotalEnergy += ecalhit->getEnergy();
            }
        }

        //Bin event information
        h_EcalSummedEnergy->Fill( ecalTotalEnergy );

        return ecalTotalEnergy;
    }
        
    void HcalHitMatcher::getParticlesLeavingEcalScoringPlane( const TClonesArray* ecalScoringPlaneHits , 
            double ecalTotalEnergy, std::vector<SimTrackerHit *> leavingScoringPlane ) {
        
        leavingScoringPlane.clear();
    
        for (int i = 0; i < ecalScoringPlaneHits->GetEntriesFast(); i++ ) {
            SimTrackerHit* ecalSPH = (SimTrackerHit*)(ecalScoringPlaneHits->At(i));

            int layerID = ecalSPH->getLayerID();
            std::vector<double> momentum = ecalSPH->getMomentum();

            //skip particles that are entering the ECAL (mostly incoming electron)
            bool isLeavingECAL = false;
            switch( layerID ) {
                case 1: //front - nothing - not near HCAL
                    isLeavingECAL = false;
                    break;
                case 2: //back - z needs to be positive
                    isLeavingECAL = (momentum[2] > 0);
                    break;
                case 3: //top - y needs to be postive
                    isLeavingECAL = (momentum[1] > 0);
                    break;
                case 4: //bottom - y needs to be negative
                    isLeavingECAL = (momentum[1] < 0);
                    break;
                case 5: //right - x needs to be negative
                    isLeavingECAL = (momentum[0] < 0);
                    break;
                case 6: //left - x needs to be positive
                    isLeavingECAL = (momentum[0] > 0);
                    break;
                default:
                    isLeavingECAL = false;
                    std::cerr << "[ Warning ] : HcalHitMatcher found a ECAL Scoring Plane Hit with layerID " << layerID << std::endl;
                    std::cerr << "    which is not one of the options (1 - 6)." << std::endl;
            }

            if ( isLeavingECAL ) {
                leavingScoringPlane.push_back(ecalSPH);
                int pdgID = ecalSPH->getPdgID();
                double mass = 0;
                if ( databasePDG_.GetParticle( pdgID ) ) { 
                    mass = databasePDG_.GetParticle( pdgID )->Mass() * 1000; //returns in GeV --> convert to MeV
                } else {
                    //Geant4 uses non-PDG IDs for composite particles (like nuclei and ions)
                    //  Need to look these up manually
                    if ( pdgID == 1000010020 ) {
                        //deuteron - proton and neutron
                        mass = 938.272 + 939.565;
                    }
                }

                double energy = ecalSPH->getEnergy();
                double kinetic = energy - mass;

                if ( numParticles_.find( pdgID ) == numParticles_.end() ) {
                    //no particles with this PDG found yet
                    numParticles_[ pdgID ] = 1;
                } else {
                    //particles with this PDG have been found
                    numParticles_[ pdgID ] ++;
                }

                if ( abs(pdgID) != 12 and abs(pdgID) != 14 ) {
                    //no neutrinos
                    
                    TString pdgStr;
                    pdgStr.Form( "%d" , pdgID );
                    if ( pdgID > 1000000000 ) {
                        pdgStr = "Nuclei";
                    }

                    h_Particle_ID->Fill( ecalTotalEnergy , pdgStr , 1. );

                }//is not a neutrino

                h_Particle_Energy ->Fill( ecalTotalEnergy , energy );
                h_Particle_Kinetic->Fill( ecalTotalEnergy , kinetic );

            }//isLeavingECAL
        }//loop through all scoring plane hits

        h_NumParticles->Fill( ecalTotalEnergy , leavingScoringPlane.size() );

        return;
    }

} //ldmx namespace

DECLARE_ANALYZER_NS(ldmx, HcalHitMatcher);
