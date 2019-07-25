/**
 * @file HcalHitMatcher.cxx
 * @brief The purpose of this analyzer is to study vetoes caused by activity in the Hcal using 
 *        Monte Carlo simulations.
 *        It extracts an array of sim particles and then matches each sim particle to a hit in the Hcal.
 *        Hcal hits and sim particles are matched spatially by finding the closest distance from a 
 *        sim particle's trajectory to a reconstructed Hcal hit.
 *        Plots and results are then tabulated in ROOT based on the sim particle and Hcal hit matches.
 *
 * @author Matthew Forsman , Tom Eichlersmith
 */

#include "Hcal/HcalHitMatcher.h"
#include "Event/HcalHit.h"
#include "Event/SimParticle.h"

#include "TDirectoryFile.h" //directories to organize histograms

namespace ldmx {

    void HcalHitMatcher::configure(const ldmx::ParameterSet& ps) { 
        
        EcalHitColl_ = ps.getString( "EcalHitCollectionName" , "ecalDigis" );
        HcalHitColl_ = ps.getString( "HcalHitCollectionName" , "hcalDigis" );
        EcalScoringPlane_ = ps.getString( "EcalScoringPlaneHitsName" , "EcalScoringPlaneHits" ); 
        HcalScoringPlane_ = ps.getString( "HcalScoringPlaneHitsName" , "HcalScoringPlaneHits" );

        maxMatchDist_ = ps.getDouble( "MaximumMatchDistance" , 150.0 );

        return;
    }

    void HcalHitMatcher::analyze(const ldmx::Event& event) {

        numEvents_++;

        //---------- This section obtains a list of sim particles that cross the ecal scoring plane --------->
        const TClonesArray* ecalScoringPlaneHits = event.getCollection( EcalScoringPlane_ );
        const TClonesArray* simParticles = event.getCollection("SimParticles"); // side-effect is to make TRefs all valid

        std::vector<ldmx::SimTrackerHit*> simVec;
    
        for (int i = 0; i < ecalScoringPlaneHits->GetEntriesFast(); i++ ) {
            ldmx::SimTrackerHit* ecalSPH = (ldmx::SimTrackerHit*)(ecalScoringPlaneHits->At(i));
            simVec.push_back(ecalSPH);
        }

        std::sort(simVec.begin(), simVec.end(), compSims);

        //SimParticles that cross Ecal Scoring Planes
        std::vector< ldmx::SimParticle* > simParticleCrossEcalSP;

        ldmx::SimParticle* lastP = 0; //sometimes multiple SP hits from same particle
        for ( std::vector<ldmx::SimTrackerHit*>::iterator it_simVec = simVec.begin();
              it_simVec != simVec.end(); ++it_simVec ) {

            ldmx::SimParticle* sP = (*it_simVec)->getSimParticle();
            if ( sP != lastP ) {
                //make sure there aren't any repeats
                lastP = sP;
                simParticleCrossEcalSP.push_back( sP );
            }
        }

        //---------- This section obtains a list of sim particles that cross the hcal scoring plane -------->
        //Currently it is NOT being used to obtain any information
//        
//        const TClonesArray* scoringPlaneHits_h=event.getCollection(scoringPlane_h);
//        const TClonesArray* simParticles_h=event.getCollection("SimParticles"); // side-effect is to make TRefs all valid
//    
//        ldmx::SimTrackerHit* hcalSPP;
//        std::vector<SimTrackerHit*> simVec_h;
//        std::vector<SimTrackerHit*> filteredSimVec_h;//Sim particles are organized from highest to lowest momentum
//    
//        if ( scoringPlaneHits_h->GetEntriesFast() > 0 ) {
//            for (TIter next(scoringPlaneHits_h); hcalSPP = (ldmx::SimTrackerHit*)next();) {
//                simVec_h.push_back(hcalSPP);
//            }
//        }
//    
//        std::sort(simVec_h.begin(), simVec_h.end(), compSims);
//
//        SimParticle* lastP_h = 0; // sometimes multiple SP hits from same particle
//        for (int j = 0; j < simVec_h.size(); j++) {
//            SimParticle* sP_h = simVec_h[j]->getSimParticle();
//            if (sP_h == lastP_h) continue;
//            lastP = sP_h;
//            filteredSimVec_h.push_back(simVec_h[j]);
//        }
//        
//        std::sort(filteredSimVec_h.begin(), filteredSimVec_h.end(), compSimsP);
    
    
        //---------- This section calculates the energy in the ECAL ---------------------------------------->
        //Then uses this energy to set standard deviation range

        const TClonesArray* ecalHitColl = event.getCollection( EcalHitColl_ ); 

        double ecalTotalEnergy = 0;
        for(int i=0; i < ecalHitColl->GetEntriesFast(); i++) {
            ldmx::EcalHit* ecalhit = (ldmx::EcalHit*)(ecalHitColl->At(i));
            if ( ! ecalhit->isNoise() ) { //Only add non-noise hits
                ecalTotalEnergy += ecalhit->getEnergy();
            }
        }

        //Bin event information
        h_Ecal_SummedEnergy->Fill( ecalTotalEnergy );
    
        h_NumParticles->Fill( ecalTotalEnergy , simParticleCrossEcalSP.size() );

        //Go through all SimParticles that crossed ECAL Scoring Plane
        for ( const ldmx::SimParticle* simPart : simParticleCrossEcalSP ) {
            int pdgID = simPart->getPdgID();
            double energy = simPart->getEnergy();

            h_Particle_PDGID_All->Fill( ecalTotalEnergy , pdgID );

            h_Particle_Energy_All->Fill( ecalTotalEnergy , energy );
        }

        //----This section matches HCal hits to sim particles and records results----->
        const TClonesArray* hcalHitColl = event.getCollection( HcalHitColl_ );

        float max_PE_of_event=0;
        for(int i=0; i < hcalHitColl->GetEntriesFast(); i++) { //Begin loop over hcalhits array
            ldmx::HcalHit* hcalhit = (ldmx::HcalHit*)(hcalHitColl->At(i));
            
            if ( ! hcalhit->getNoise() ) { //Only analyze non-noise hits

                numNonNoiseHits_++;

                //---- Bin HcalHit information that does not depend on mathcin -------------------->
                
                double hcalhit_radialdist2 = pow(hcalhit->getX(), 2) + pow(hcalhit->getY(), 2);
                double hcalhit_radialdist = 0;
                //check to avoid a floating point error
                if(abs(hcalhit_radialdist2) > 1e-5) {
                    hcalhit_radialdist = sqrt(hcalhit_radialdist2);
                }

                h_HcalHit_ZbyR_All->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist);
                h_HcalHit_Z->Fill( ecalTotalEnergy , hcalhit->getZ());
                h_HcalHit_Time_All->Fill( ecalTotalEnergy , hcalhit->getTime());
                h_HcalHit_PE_All->Fill( ecalTotalEnergy , hcalhit->getPE());
                
                
                if(hcalhit->getTime() < 15.0)  {
                    h_HcalHit_PE_TimeLess15->Fill( ecalTotalEnergy , hcalhit->getPE());
                    h_HcalHit_ZbyR_TimeLess15->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist);
                } else if(hcalhit->getTime() > 40.0)  {
                    h_HcalHit_PE_TimeGreat40->Fill( ecalTotalEnergy , hcalhit->getPE());
                    h_HcalHit_ZbyR_TimeGreat40->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist);
                }
                
                if(hcalhit->getPE() > max_PE_of_event)
                    max_PE_of_event=hcalhit->getPE();

                //---- Attempt to match this HcalHit to a SimParticle that cross the Ecal SP ----->

                //Iterate over all SimParticles that cross Ecal Scoring Plane to try to find a match
                //  for this HcalHit
                double new_dist=9999, dist=9998; //initializing distance variables
                const ldmx::SimParticle* matchedParticle = nullptr;
                for ( const ldmx::SimParticle* simPart : simParticleCrossEcalSP ) {

                    std::vector<double> simStart = simPart->getVertex();
                    std::vector<double> simEnd = simPart->getEndPoint();
        
                    TVector3 simStartT = TVector3(simStart[0], simStart[1], simStart[2]);
                    TVector3 simEndT = TVector3(simEnd[0], simEnd[1], simEnd[2]);
                    TVector3 hCalPoint = TVector3(hcalhit->getX(), hcalhit->getY(), hcalhit->getZ());
                    
                    new_dist = point_line_distance(simStartT, simEndT, hCalPoint);
                    
                    h_Particle_HitDistance_All->Fill( ecalTotalEnergy , new_dist);
        
                    if(simStart[2]<10.0 and simPart->getEnergy()>3000.0) {
                        //discarding original electron
                        //DO NOTHING (skip this sim particle)
                    } else if(new_dist < dist) {
                        dist = new_dist; //Distance to matched particle
                        matchedParticle = simPart;
                    }
                    
                } //iterate over sim particles to match one to current hcal hit

                //---- Bin HcalHit/SimParticle information for successfully matched hits --------->

                if( matchedParticle and dist <= maxMatchDist_ ) {
                
                    numMatchedHits_++;

                    h_Particle_HitDistance_Matched->Fill( ecalTotalEnergy ,  dist );
        
                    h_HcalHit_Time_Matched_All->Fill( ecalTotalEnergy , hcalhit->getTime());
                    
                    int pdgID = matchedParticle->getPdgID();

                    h_Particle_PDGID_Matched->Fill( ecalTotalEnergy , pdgID);
        
                    double part_hcalhit_timeDiff = (hcalhit->getTime()) - (matchedParticle->getTime());
                    
                    h_HcalHit_Time_Matched_Tdif->Fill( ecalTotalEnergy , part_hcalhit_timeDiff);

                    if(part_hcalhit_timeDiff < 15.0)  {
                        h_HcalHit_PE_Matched_TdifLess15->Fill( ecalTotalEnergy , hcalhit->getPE());
                        h_HcalHit_ZbyR_Matched_TdifLess15->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist);
                    } else if(part_hcalhit_timeDiff > 40.0)  {
                        h_HcalHit_PE_Matched_TdifGreat40->Fill( ecalTotalEnergy , hcalhit->getPE());
                        h_HcalHit_ZbyR_Matched_TdifGreat40->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist);
                    }
                    
                    //protons or neutrons (nucleons) 
                    if( pdgID==2112 or pdgID==2212 ) { 
                        h_HcalHit_Time_Matched_Nucleons->Fill( ecalTotalEnergy , matchedParticle->getTime());
                    }
                    
                    h_Particle_Energy_Matched->Fill( ecalTotalEnergy , matchedParticle->getEnergy());
        
                    switch(pdgID) {
                        case 11:
                            h_HcalHit_ZbyR_Matched_Electron->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist); 
                            break;
                        case 22:
                            h_HcalHit_ZbyR_Matched_Photon->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist); 
                            break;
                        case 2112:
                            h_HcalHit_ZbyR_Matched_Neutron->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist); 
                            break;
                        default:
                            h_HcalHit_ZbyR_Matched_Other->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist); 
                            break;
                    }

                } else {

                    h_HcalHit_ZbyR_Unmatched->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist);

                } //matched or unmatched
    
            } // if not a noise hit

        }//End loop over hcalhits array

        // maximum PE in hcal hits for the event
        h_EventMaxPE->Fill( ecalTotalEnergy , max_PE_of_event );

        return;
    } //analyze

   
    bool HcalHitMatcher::compSimsP(const SimTrackerHit* a, const SimTrackerHit* b) {
        std::vector<double> paVec = a->getMomentum();
        std::vector<double> pbVec = b->getMomentum();
    
        double pa2 = pow(paVec[0],2)+pow(paVec[1],2)+pow(paVec[2],2);
        double pb2 = pow(pbVec[0],2)+pow(pbVec[1],2)+pow(pbVec[2],2);
    
        return pa2 > pb2;
    }

    bool HcalHitMatcher::compSims(const SimTrackerHit* a, const SimTrackerHit* b) {

        if (a->getSimParticle() == b->getSimParticle()) {
             return compSimsP(a,b);
        } else {
             return a->getSimParticle() < b->getSimParticle();
        }
    }

    //This function finds the minimum distance between a line segment and a point in 3D space
    double HcalHitMatcher::point_line_distance(TVector3 v, TVector3 w, TVector3 p)  {
        
        //Define 2 line segments from point v to w and from point v to p
        TVector3 vw = TVector3(w.x()-v.x(), w.y()-v.y(), w.z()-v.z());
        TVector3 vp = TVector3(v.x()-p.x(), v.y()-p.y(), v.z()-p.z());
    
        //Define both line segment's magnitude squared
        const double L2vw = pow(vw.Mag(), 2);
        const double L2vp = pow(vp.Mag(), 2);
    
        //Define a parameter t clamped between 0 and 1 so that the distance is never measured from 
        //beyond the endpoints of the line segment
        const double t = std::max(0.0, -1*vp.Dot(vw)/L2vw);//std::max(0.0, std::min(1.0, -1*vp.Dot(vw)/L2vw));
    
        //Calculate the minimum distance squared
        double d2 = L2vp + 2.0*t*vp.Dot(vw) + pow(t,2)*L2vw;
    
        //Return minimum distance and catch floating point errors near zero
        if(abs(d2) < 1e-5)
            return 0;
        else
            return sqrt(d2);
    }

    void HcalHitMatcher::onProcessStart() {
        
        numNonNoiseHits_ = 0;
        numMatchedHits_ = 0;
        numEvents_ = 0;

        // Make directory tree to organize histograms in
        TDirectory* d_base = getHistoDirectory();

        //////////// Directory Tree
        //  Ecal_SummedEnergy
        //  NumParticles
        //  EventMaxPE
        //  Particle
        //      PDGID
        //          All
        //          Matched
        //      HitDistance
        //          All
        //          Matched
        //      Energy
        //          All
        //          Matched
        //  HcalHit
        //      Z
        //      ZbyR
        //          All
        //          Unmatched
        //          TimeLess15
        //          TimeGreat40
        //          Matched
        //              Photon
        //              Electron
        //              Neutron
        //              Other
        //              TdifLess15
        //              TdifGreat40
        //      PE
        //          All
        //          TimeLess15
        //          TimeGreat40
        //          Matched
        //              TdifLess15
        //              TdifGreat40
        //      Time
        //          All
        //          Matched
        //              All
        //              Nucleons
        //              Tdif

        TDirectory* d_Ecal_SummedEnergy                 = d_base->mkdir( "Ecal_SummedEnergy" );
        TDirectory* d_NumParticles                      = d_base->mkdir( "NumParticles" );
        TDirectory* d_EventMaxPE                        = d_base->mkdir( "EventMaxPE" );

        TDirectory* d_Particle                          = d_base->mkdir( "Particle" );
        TDirectory* d_Particle_PDGID                    = d_Particle->mkdir( "PDGID" );
        TDirectory* d_Particle_PDGID_All                = d_Particle_PDGID->mkdir( "All" );
        TDirectory* d_Particle_PDGID_Matched            = d_Particle_PDGID->mkdir( "Matched" );
        TDirectory* d_Particle_HitDistance              = d_Particle->mkdir( "HitDistance" );
        TDirectory* d_Particle_HitDistance_All          = d_Particle_HitDistance->mkdir( "All" );
        TDirectory* d_Particle_HitDistance_Matched      = d_Particle_HitDistance->mkdir( "Matched" );
        TDirectory* d_Particle_Energy                   = d_Particle->mkdir( "Energy" );
        TDirectory* d_Particle_Energy_All               = d_Particle_Energy->mkdir( "All" );
        TDirectory* d_Particle_Energy_Matched           = d_Particle_Energy->mkdir( "Matched" );

        TDirectory* d_HcalHit                           = d_base->mkdir( "HcalHit" );
        TDirectory* d_HcalHit_Z                         = d_HcalHit->mkdir( "Z" );
        TDirectory* d_HcalHit_ZbyR                      = d_HcalHit->mkdir( "ZbyR" );
        TDirectory* d_HcalHit_ZbyR_All                  = d_HcalHit_ZbyR->mkdir( "All" );
        TDirectory* d_HcalHit_ZbyR_Unmatched            = d_HcalHit_ZbyR->mkdir( "Unmatched" );
        TDirectory* d_HcalHit_ZbyR_TimeLess15           = d_HcalHit_ZbyR->mkdir( "TimeLess15" );
        TDirectory* d_HcalHit_ZbyR_TimeGreat40          = d_HcalHit_ZbyR->mkdir( "TimeGreat40" );
        TDirectory* d_HcalHit_ZbyR_Matched              = d_HcalHit_ZbyR->mkdir( "Matched" );
        TDirectory* d_HcalHit_ZbyR_Matched_Photon       = d_HcalHit_ZbyR_Matched->mkdir( "Photon" );
        TDirectory* d_HcalHit_ZbyR_Matched_Electron     = d_HcalHit_ZbyR_Matched->mkdir( "Electron" );
        TDirectory* d_HcalHit_ZbyR_Matched_Neutron      = d_HcalHit_ZbyR_Matched->mkdir( "Neutron" );
        TDirectory* d_HcalHit_ZbyR_Matched_Other        = d_HcalHit_ZbyR_Matched->mkdir( "Other" );
        TDirectory* d_HcalHit_ZbyR_Matched_TdifLess15   = d_HcalHit_ZbyR_Matched->mkdir( "TdifLess15" );
        TDirectory* d_HcalHit_ZbyR_Matched_TdifGreat40  = d_HcalHit_ZbyR_Matched->mkdir( "TdifGreat40" );
        TDirectory* d_HcalHit_PE                        = d_HcalHit->mkdir( "PE" );
        TDirectory* d_HcalHit_PE_All                    = d_HcalHit_PE->mkdir( "All" );
        TDirectory* d_HcalHit_PE_TimeLess15             = d_HcalHit_PE->mkdir( "TimeLess15" );
        TDirectory* d_HcalHit_PE_TimeGreat40            = d_HcalHit_PE->mkdir( "TimeGreat40" );
        TDirectory* d_HcalHit_PE_Matched                = d_HcalHit_PE->mkdir( "Matched" );
        TDirectory* d_HcalHit_PE_Matched_TdifLess15     = d_HcalHit_PE_Matched->mkdir( "TdifLess15" );
        TDirectory* d_HcalHit_PE_Matched_TdifGreat40    = d_HcalHit_PE_Matched->mkdir( "TdifGreat40" );
        TDirectory* d_HcalHit_Time                      = d_HcalHit->mkdir( "Time" );
        TDirectory* d_HcalHit_Time_All                  = d_HcalHit_Time->mkdir( "All" );
        TDirectory* d_HcalHit_Time_Matched              = d_HcalHit_Time->mkdir( "Matched" );
        TDirectory* d_HcalHit_Time_Matched_All          = d_HcalHit_Time_Matched->mkdir( "All" );
        TDirectory* d_HcalHit_Time_Matched_Nucleons     = d_HcalHit_Time_Matched->mkdir( "Nucleons" );
        TDirectory* d_HcalHit_Time_Matched_Tdif         = d_HcalHit_Time_Matched->mkdir( "Tdif" );

        d_Ecal_SummedEnergy->cd();
        h_Ecal_SummedEnergy = new TH1D(
                "Ecal_SummedEnergy",
                "Ecal Summed Energy;Energy [MeV] (10MeV bin width);Count",
                800,0,8000);//10MeV bins

        d_NumParticles->cd();
        h_NumParticles = new TH2D(
                "NumParticles" ,
                "Num Particles in Event;X;Number of SimParticles that Crossed the ECAL Scoring Plane;Count",
                800,0,8000,
                50,0,50);

        d_EventMaxPE->cd();
        h_EventMaxPE = new TH2D(
                "EventMaxPE",
                "Maximum PE for all Hits in Event;X;Maximum PE for all HcalHits in Event;Count",
                800,0,8000,
                500,0,500);

        d_Particle_PDGID_All->cd();
        h_Particle_PDGID_All = new TH2D(
                "Particle_PDGID_All",
                "PDG IDs;X;PDG ID of SimParticle Crossing ECAL Scoring Plane;Count",
                800,0,8000,
                10000,-5000,5000);

        d_Particle_PDGID_Matched->cd();
        h_Particle_PDGID_Matched = new TH2D(
                "Particle_PDGID_Matched",
                "PDG IDs;X;PDG ID of SimParticle Matched to HcalHit;Count",
                800,0,8000,
                10000,-5000,5000);

       d_Particle_HitDistance_All->cd();
       h_Particle_HitDistance_All = new TH2D(
               "Particle_HitDistance_All",
               "Any pair of HcalHit and SimParticle crossing ECAL Scoring Plane;X;Distance between SimParticle and HcalHit;Count",
               800,0,8000,
               400, 0, 2000);
       
       d_Particle_HitDistance_Matched->cd();
       h_Particle_HitDistance_Matched = new TH2D(
               "Particle_HitDistance_Matched", 
               "Distance between SimParticle and HcalHit when matched (5mm bins)", 
               800,0,8000,
               400, 0, 2000);
       
       d_Particle_Energy_All->cd();
       h_Particle_Energy_All = new TH2D(
               "Particle_Energy_All",
               "All Particle Energies;X;Energy(MeV)(5MeV bin width);Count",
               800,0,8000,
               800,0,4000);

       d_Particle_Energy_Matched->cd();
       h_Particle_Energy_Matched = new TH2D(
               "Particle_Energy_Matched",
               "Matched Particle Energies;X;Energy(MeV)(5MeV bin width);Count",
               800,0,8000,
               800,0,4000);

       d_HcalHit_Z->cd();
       h_HcalHit_Z = new TH2D(
               "HcalHit_Z",
               "Z depth of HCal hits (10mm bins)",
               800,0,8000,
               320, 0, 3200);

       d_HcalHit_ZbyR_All->cd();
       h_HcalHit_ZbyR_All = new TH3D(
               "HcalHit_ZbyR_All", 
               "All HcalHit locations;X;Z depth (mm);radial distance from z-axis (mm)",
               800,0,8000,
               80,0,3200,112,0,4500);

       d_HcalHit_ZbyR_Unmatched->cd();
       h_HcalHit_ZbyR_Unmatched = new TH3D(
               "HcalHit_ZbyR_Unmatched", 
               "Hcal unmatched hit locations;X;Z depth(mm);radial distance from z-axis(mm)",
               800,0,8000,
               80,0,3200,112,0,4500);

       d_HcalHit_ZbyR_TimeLess15->cd();
       h_HcalHit_ZbyR_TimeLess15 = new TH3D(
               "HcalHit_ZbyR_TimeLess15",
               "HcalHits with Time < 15ns locations;X;Z depth(mm);radial distance from z-axis(mm)",
               800,0,8000,
               80,0,3200,112,0,4500);

       d_HcalHit_ZbyR_TimeGreat40->cd();
       h_HcalHit_ZbyR_TimeGreat40 = new TH3D(
               "HcalHit_ZbyR_TimeGreat40",
               "HcalHits with Time > 40ns locations;X;Z depth(mm);radial distance from z-axis(mm)",
               800,0,8000,
               80,0,3200,112,0,4500);

       d_HcalHit_ZbyR_Matched_Photon->cd();
       h_HcalHit_ZbyR_Matched_Photon = new TH3D(
               "HcalHit_ZbyR_Matched_Photon", 
               "Hcal photon hit locations;X;Z depth(mm);radial distance from z-axis(mm)",
               800,0,8000,
               80,0,3200,112,0,4500);

       d_HcalHit_ZbyR_Matched_Electron->cd();
       h_HcalHit_ZbyR_Matched_Electron = new TH3D(
               "HcalHit_ZbyR_Matched_Electron", 
               "Hcal electron hit locations;X;Z depth(mm);radial distance from z-axis(mm)",
               800,0,8000,
               80,0,3200,112,0,4500);

       d_HcalHit_ZbyR_Matched_Neutron->cd();
       h_HcalHit_ZbyR_Matched_Neutron = new TH3D(
               "HcalHit_ZbyR_Matched_Neutron", 
               "Hcal neutron hit locations;X;Z depth(mm);radial distance from z-axis(mm)",
               800,0,8000,
               80,0,3200,112,0,4500);

       d_HcalHit_ZbyR_Matched_Other->cd();
       h_HcalHit_ZbyR_Matched_Other = new TH3D(
               "HcalHit_ZbyR_Matched_Other", 
               "Hcal other particle hit locations;X;Z depth(mm);radial distance from z-axis(mm)",
               800,0,8000,
               80,0,3200,112,0,4500);

       d_HcalHit_ZbyR_Matched_TdifLess15->cd();
       h_HcalHit_ZbyR_Matched_TdifLess15 = new TH3D(
               "HcalHit_ZbyR_Matched_TdifLess15",
               "Matched HcalHit location with time dif < 15ns"
                ";X;Z depth(mm);radial distance from z-axis(mm)",
               800,0,8000,
               80,0,3200,112,0,4500);

       d_HcalHit_ZbyR_Matched_TdifGreat40->cd();
       h_HcalHit_ZbyR_Matched_TdifGreat40 = new TH3D(
               "HcalHit_ZbyR_Matched_TdifGreat40",
               "Matched HcalHit location with time dif > 40ns"
                ";X;Z depth(mm);radial distance from z-axis(mm)",
               800,0,8000,
               80,0,3200,112,0,4500);

       d_HcalHit_PE_All->cd();
       h_HcalHit_PE_All = new TH2D(
               "HcalHit_PE_All",
               "PEs of all HcalHits;X;Photoelectrons(PEs);Count",
               800,0,8000,
               200,0,200);

       d_HcalHit_PE_TimeLess15->cd();
       h_HcalHit_PE_TimeLess15 = new TH2D(
               "HcalHit_PE_TimeLess15",
              "HcalHits with Time < 15ns;X;Photoelectrons(PEs);Count",
               800,0,8000,
               200,0,200);

       d_HcalHit_PE_TimeGreat40->cd();
       h_HcalHit_PE_TimeGreat40 = new TH2D(
               "HcalHit_PE_TimeGreat40",
               "HcalHits with Time > 40ns;X;Photoelectrons(PEs);Count",
               800,0,8000,
               200,0,200);

       d_HcalHit_PE_Matched_TdifLess15->cd();
       h_HcalHit_PE_Matched_TdifLess15 = new TH2D(
               "HcalHit_PE_Matched_TdifLess15",
               "Matched HcalHit with Time dif < 15ns;X;Photoelectrons(PEs);Count",
               800,0,8000,
               200,0,200);

       d_HcalHit_PE_Matched_TdifGreat40->cd();
       h_HcalHit_PE_Matched_TdifGreat40 = new TH2D(
               "HcalHit_PE_Matched_TdifGreat40",
               "Matched HcalHit with Time dif > 40ns;X;Photoelectrons(PEs);Count",
               800,0,8000,
               200,0,200);

       d_HcalHit_Time_All->cd();
       h_HcalHit_Time_All = new TH2D(
               "HcalHit_Time_All",
               "Time of All HcalHits;X;time(ns)(5ns bin width);Count",
               800,0,8000,
               100,0,500);

       d_HcalHit_Time_Matched_All->cd();
       h_HcalHit_Time_Matched_All = new TH2D(
               "HcalHit_Time_Matched_All", 
               "Time of Matched HcalHits;X;Time(ns);Number of particles created", 
               800,0,8000,
               500, 0, 500);

       d_HcalHit_Time_Matched_Nucleons->cd();
       h_HcalHit_Time_Matched_Nucleons = new TH2D(
               "HcalHit_Time_Matched_Nucleons",
               "Time of HcalHits Matched to Nucleons;X;Time(ns);Number of Nucleons created", 
               800,0,8000,
               500, 0, 500);
       
       d_HcalHit_Time_Matched_Tdif->cd();
       h_HcalHit_Time_Matched_Tdif = new TH2D(
               "HcalHit_Time_Matched_Tdif",
               ";X;Time Difference Between SimParticle and matched HcalHit [ns] (2ns bin width);Count",
               800,0,8000,
               100,0,200);

        //go back to base histogram directory
        d_base->cd();
    
        return;
    } //onProcessStart
    
    void HcalHitMatcher::onProcessEnd() {
        
        double hitRate, matchRate;
        {
            //temporary variables for calculating rates
            double numerator = numNonNoiseHits_;
            double denominator = numEvents_;
            hitRate = numerator/denominator;

            numerator = numMatchedHits_;
            denominator = numNonNoiseHits_;
            matchRate = numerator / denominator;
        }

        printf( "Number of Events:          %i\n" , numEvents_ );
        printf( "Number of Non Noise Hits:  %i\n" , numNonNoiseHits_ );
        printf( "Number of Matched Hits:    %i\n" , numMatchedHits_ );
        printf( "Hit Rate (hits/events):    %f\n" , hitRate );
        printf( "Match Rate (matches/hits): %f\n" , matchRate );

        return;
    }

} //ldmx namespace

DECLARE_ANALYZER_NS(ldmx, HcalHitMatcher);
