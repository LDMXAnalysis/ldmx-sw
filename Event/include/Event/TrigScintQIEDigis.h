#ifndef EVENT_TRIGSCINTQIEDIGIS_H
#define EVENT_TRIGSCINTQIEDIGIS_H

#include"SimQIE.h"
#include "TObject.h" //For ClassDef

namespace ldmx {
  class TrigScintQIEDigis
  {
  public:
    TrigScintQIEDigis(int maxTS_,Pulse* pl, float pd, float ns);
    TrigScintQIEDigis(int maxTS_,Pulse* pl, SimQIE* sm);
    TrigScintQIEDigis();
    ~TrigScintQIEDigis(){};

    int* GetADC(){return(ADCs);}
    int* GetTDC(){return(TDCs);}
    int* GetCID(){return(CIDs);}

    void Print(Option_t *option = "") const; // required by Event/include/Event/EventDef.h
    void Clear(Option_t *option = ""); // required by Event/include/Event/EventDef.h
  
    // private:
    int maxTS;			// no. of time samples stored
    int chanID;			// channel ID

    int* ADCs;			// analog to digital counts
    int* TDCs;			// Time to Digital counts
    int* CIDs;			// capacitor IDs
    ClassDef(TrigScintQIEDigis,1);
  };
}
#endif
