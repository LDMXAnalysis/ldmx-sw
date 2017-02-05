/**
 * @file EcalDetectorID.h
 * @brief Class that defines an ECal detector ID with a cell number
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef DETDESCR_ECALDETECTORID_H_
#define DETDESCR_ECALDETECTORID_H_

// LDMX
#include "DetDescr/DefaultDetectorID.h"
#include "DetDescr/EcalHexReadout.h"

namespace ldmx {

/**
 * @class EcalDetectorID
 * @brief Extension of DefaultDetectorID providing access to ECal cell number in a hex grid
 */
class EcalDetectorID : public DefaultDetectorID {

    private:
        EcalHexReadout* hexReadout_{nullptr};

    public:

        /**
         * Adds a cell field and re-initializes the ID.
         */
        EcalDetectorID() {
            hexReadout_ = new EcalHexReadout();
            this->getFieldList()->push_back(new IDField("cell", 2, 12, 31));
            init();
        }

        /**
         * Get the value of the cell field from the ID.
         * @return The value of the cell field.
         */
        int getCellID() {
            return this->getFieldValue(2);
        }

        std::map<int,float> getMap();
};

}

#endif
