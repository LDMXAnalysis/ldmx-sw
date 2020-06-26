/**
 * @file G4eDarkBremsstrahlung.h
 * @brief Class providing the Dark Bremsstrahlung process class.
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef SIMCORE_G4EDARKBREMSSTRAHLUNG_H_
#define SIMCORE_G4EDARKBREMSSTRAHLUNG_H_

#include "SimCore/G4eDarkBremsstrahlungModel.h"

// Geant
#include "G4VEnergyLossProcess.hh"

class G4String;
class G4ParticleDefinition;

/**
 * @class G4eDarkBremsstrahlung
 *
 * Class that represents the dark brem process.
 * A electron or positron is allowed to brem a dark photon
 */
class G4eDarkBremsstrahlung : public G4VEnergyLossProcess {

    public:
  
        /**
         * Constructor
         *
         * Sets this process up
         */
        G4eDarkBremsstrahlung(const G4String& name = "eDBrem");
  
        /**
         * Destructor
         */
        virtual ~G4eDarkBremsstrahlung() { /*Nothing on purpose*/ }
  
        /** 
         * Checks if the passed particle should be able to do this process
         *
         * @return true if particle is electron
         */
        virtual G4bool IsApplicable(const G4ParticleDefinition& p);
  
        /**
         * Reports the file name and the method (in string form)
         */
        virtual void PrintInfo();
  
        /** Pass the method for this process to the model */
        void SetMethod(G4eDarkBremsstrahlungModel::DarkBremMethod method);

        /** Pass LHE library of dark brem events to the model */
        void SetMadGraphDataLibrary(std::string path);

        /** Set threshold for non-zero cross section [GeV] */
        void SetThreshold(double thresh) { threshold_ = thresh; }

        /** Set epsilon for dark brem cross section calculation */
        void SetEpsilon(double e) { epsilon_ = e; }
 
    protected:
  
        /** Setup this process to get ready for simulation */
        virtual void InitialiseEnergyLossProcess(const G4ParticleDefinition*,
                                                 const G4ParticleDefinition*);
  
        /** Has this process been setup yet? */
        G4bool isInitialised;
 
    private:
  
        /** remove ability to assign this object */
        G4eDarkBremsstrahlung & operator=(const G4eDarkBremsstrahlung &right);

        /** remove ability to copy construct */
        G4eDarkBremsstrahlung(const G4eDarkBremsstrahlung&);

        /** Method that was passed to the model */
        G4eDarkBremsstrahlungModel::DarkBremMethod method_{G4eDarkBremsstrahlungModel::DarkBremMethod::Undefined};

        /** Mad Graph library passed to model */
        std::string madGraphLibrary_;

        /** Threshold for non-zero xsec [GeV] */
        double threshold_;

        /** Epsilon for dark brem xsec calculation */
        double epsilon_;
  
};


#endif // SIMCORE_G4EDARKBREMSSTRAHLUNG_H_
