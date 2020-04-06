/**
 * @file RunManager.cxx
 * @brief Class providing a Geant4 run manager implementation.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimApplication/RunManager.h"

//-------------//
//   ldmx-sw   //
//-------------//
#include "SimApplication/APrimePhysics.h"
#include "SimApplication/DetectorConstruction.h"
#include "SimApplication/GammaPhysics.h"
#include "SimApplication/ParallelWorld.h"
#include "SimApplication/PrimaryGeneratorAction.h"
#include "SimApplication/USteppingAction.h"
#include "SimApplication/UserActionManager.h"
#include "SimApplication/UserEventAction.h"
#include "SimApplication/UserRunAction.h"
#include "SimApplication/UserStackingAction.h"
#include "SimApplication/UserTrackingAction.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/FrameworkDef.h" 

//------------//
//   Geant4   //
//------------//
#include "FTFP_BERT.hh"
#include "G4GDMLParser.hh"
#include "G4GenericBiasingPhysics.hh"
#include "G4VModularPhysicsList.hh"
#include "G4ParallelWorldPhysics.hh"
#include "G4ProcessTable.hh"

namespace ldmx {

    RunManager::RunManager(Parameters& parameters) {  

        parameters_ = parameters; 

        // Set whether the ROOT primary generator should use the persisted seed.
        auto rootPrimaryGenUseSeed{parameters.getParameter< bool >("rootPrimaryGenUseSeed")}; 
        setUseRootSeed(rootPrimaryGenUseSeed); 
    
    }

    RunManager::~RunManager() {
    }

    void RunManager::setupPhysics() {

        auto pList{physicsListFactory_.GetReferencePhysList("FTFP_BERT")};
        
        parallelWorldPath_ = parameters_.getParameter<std::string>("scoringPlanes");
        isPWEnabled_ = (not parallelWorldPath_.empty());
        if ( isPWEnabled_ ) {
            ldmx_log(info) << "Parallel worlds physics list has been registered.";
            pList->RegisterPhysics(new G4ParallelWorldPhysics("ldmxParallelWorld"));
        }

        pList->RegisterPhysics(new GammaPhysics);
        pList->RegisterPhysics(new APrimePhysics( parameters_ ));
       
        auto biasingEnabled{parameters_.getParameter< bool >("biasing.enabled")}; 
        if (biasingEnabled) {

            auto biasedParticle{parameters_.getParameter< std::string >("biasing.particle")}; 
            ldmx_log(info) << "Enabling biasing of particle type " << biasedParticle;

            // Instantiate the constructor used when biasing
            G4GenericBiasingPhysics* biasingPhysics = new G4GenericBiasingPhysics();

            // Specify what particles are being biased
            biasingPhysics->Bias(biasedParticle);

            // Register the physics constructor to the physics list:
            pList->RegisterPhysics(biasingPhysics);
        }

        this->SetUserInitialization(pList);
    }

    void RunManager::Initialize() {
        
        setupPhysics();

        // The parallel world needs to be registered before the mass world is
        // constructed i.e. before G4RunManager::Initialize() is called. 
        if (isPWEnabled_) {
            ldmx_log(info) << "Parallel worlds have been enabled.";

            G4GDMLParser* pwParser = new G4GDMLParser();
            pwParser->Read(parallelWorldPath_);
            this->getDetectorConstruction()->RegisterParallelWorld(new ParallelWorld(pwParser, "ldmxParallelWorld"));
        }

        G4RunManager::Initialize();

        // Instantiate the primary generator action
        auto primaryGeneratorAction{ new PrimaryGeneratorAction(parameters_) };
        SetUserAction( primaryGeneratorAction );

        // Instantiate action manager
        auto actionManager{UserActionManager::getInstance()}; 

        // Get instances of all G4 actions
        auto actions{actionManager.getActions()};
       
        // Create all user actions
        auto userActions{parameters_.getParameter< std::vector< Class > >("actions")}; 
        std::for_each(userActions.begin(), userActions.end(), 
                [&actionManager](auto& userAction) { 
                    actionManager.createAction(userAction.className_, userAction.instanceName_, userAction.params_); 
                }
        );

        // Register all actions with the G4 engine
        for (const auto& [key, act] : actions) {
            std::visit([this](auto&& arg) { this->SetUserAction(arg); }, act); 
        }
    }

    void RunManager::TerminateOneEvent() {
        
        //have geant4 do its own thing
        G4RunManager::TerminateOneEvent();

        //reset dark brem process (if needed)
        G4ProcessTable* ptable = G4ProcessTable::GetProcessTable();
        G4int verbosity = ptable->GetVerboseLevel();
        ptable->SetVerboseLevel(0); //silent ptable while searching for process that may/may not exist
        G4String pname = "biasWrapper(eDBrem)"; //TODO allow eDBrem to be biased or unbiased
        bool active = true;
        ptable->SetProcessActivation(pname,active);    
        if ( this->GetVerboseLevel() > 1 ) {
            ldmx_log(debug) << "Reset the dark brem process (if it was activated).";
        }
        ptable->SetVerboseLevel(verbosity);

    }

    DetectorConstruction* RunManager::getDetectorConstruction() {
        return static_cast<DetectorConstruction*>(this->userDetector); 
    }

} // ldmx 
