#include "Event/EcalDigiCollection.h"

ClassImp(ldmx::EcalDigiCollection)

namespace ldmx {

    void EcalDigiCollection::Copy( TObject &copy ) const {

        DigiCollection::Copy( copy );

        return;
    }

    std::vector< EcalDigiSample > EcalDigiCollection::getDigi( unsigned int digiIndex ) const {
        
        std::vector< EcalDigiSample > digi;
        for ( unsigned int sampleIndex = 0; sampleIndex < this->getNumSamplesPerChannel(); sampleIndex++ ) {
            digi.push_back( this->getSample( digiIndex , sampleIndex ) );
        }

        return digi;
    }

    void EcalDigiCollection::addDigi( std::vector< EcalDigiSample > newSamples ) {
        
        std::vector< EcalDigiSample >::iterator it_newSamples;
        int channelID = newSamples.at(0).rawID_;
        std::vector< int32_t > words;
        for ( it_newSamples = newSamples.begin(); it_newSamples != newSamples.end(); ++it_newSamples ) {
            
            int32_t word;

            //this is where the measurements --> word translation occurs
            int adc_t = it_newSamples->adc_t_;
            int tot   = it_newSamples->tot_;
            int toa   = it_newSamples->toa_;

            //the chip returns flags that determine what the three measurements are
            //  I (Tom E) don't know right now what that mapping is, so I will not use them.
            //  Just hard-coding ADCt, TOT, and TOA right now

            word = (1 << FIRSTFLAG_POS) 
                 + (1 << SECONFLAG_POS) 
                 + ( (adc_t & TEN_BIT_MASK) << FIRSTMEAS_POS ) 
                 + ( (tot   & TEN_BIT_MASK) << SECONMEAS_POS ) 
                 + ( toa & TEN_BIT_MASK );
            
            words.push_back( word );
        }

        this->addDigiWords( channelID , words );

        return;
    }

    EcalDigiSample EcalDigiCollection::getSample( unsigned int digiIndex , unsigned int sampleIndex ) const {

        EcalDigiSample sample;

        sample.rawID_ = this->getChannelID( digiIndex );

        int32_t word = this->getSampleWord( digiIndex , sampleIndex  );

        //this is where the word --> measurements translation occurs

        bool firstFlag = ONE_BIT_MASK & ( word >> FIRSTFLAG_POS );
        bool seconFlag = ONE_BIT_MASK & ( word >> SECONFLAG_POS );
        int  firstMeas = TEN_BIT_MASK & ( word >> FIRSTMEAS_POS );
        int  seconMeas = TEN_BIT_MASK & ( word >> SECONMEAS_POS );
        int  lastMeas  = TEN_BIT_MASK & ( word );

        //the chip returns flags that determine what the three measurements are
        //  I (Tom E) don't know right now what that mapping is, so I will not use them.

        sample.adc_t_ = firstMeas;
        sample.tot_   = seconMeas;
        sample.toa_   = lastMeas;
        sample.adc_tm1_ = -99;

        return sample;
    }
}