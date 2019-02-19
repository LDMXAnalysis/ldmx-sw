/**
 * @file AnalysisUtils.h
 * @brief Collection of utility functions useful for analysis
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _ANALYSIS_UTILS_H_
#define _ANALYSIS_UTILS_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <vector>

// Forward declaration
class TClonesArray;  

namespace ldmx { 

    // Forward declaration for classes inside ldmx namespace
    class SimParticle;

    /**
     * Find and return the the sim particle associated with the recoil 
     * electron.
     *
     * @param particles Collection of sim particles
     * @param index Position along the TClonesArray to start the search for 
     *              the recoil.  The default is to start at the 
     *              beginning of the array. 
     */
    const SimParticle* searchForRecoil(const TClonesArray* particles, const int index = 0);

    /**
     * Find and return the sim particle associated with the gamma that 
     * underwent a photo-nuclear reaction.
     *
     * @param particle Sim particle that will be used to retrieve the list of 
     *                 particles to search through.
     * @param index Position along the TClonesArray to start the search for the
     *              photo-nuclear gamma.  The default is to start at the 
     *              beginning of the array.
     */
    const SimParticle* searchForPNGamma(const SimParticle* particle, const int index = 0);  

    /**
     * Calculate the magnitude of a vector.
     *
     * @param v C++ std::vector
     * @return Magnitude of the given vector.
     */
    template <class T>
    double vectorMagnitude(std::vector<T> v) { 
        return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]); 
    } 

} // ldmx

#endif // _ANALYSIS_UTILS_H_
