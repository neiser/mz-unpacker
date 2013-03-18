#ifndef _T_TRB_HIT
#define _T_TRB_HIT
// +++ include header files +++
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "TObject.h"

//#include "TrbStructs.h"
//#include "THldSubevent.h"
#include "myUtilities.h"

// +++ class definition +++
class TTrbHit : public TObject{
	friend class TTrbEventClass;
private:
	void ComputeTime() { fTime = nCoarseTime-nFineTime; };
	void Init(); // initialise variables etc
protected:
	UInt_t nTrbAddress;
	UInt_t nTdcChannel;
	UInt_t nSubEvtId;
	UInt_t nTdcErrCode;
	UInt_t nSignalEdge;
	UInt_t nEpochCounter;
	UInt_t nCoarseTime;
	UInt_t nFineTime;
	Double_t fTime; // this needs to be a Double_t
	Bool_t bIsCalibrated;
	Bool_t bIsRefChannel;
	Bool_t bVerboseMode;
public:
	TTrbHit() { Init(); };
	//TTrbHit(UInt_t nUserTrbAddress, UInt_t nUserTdcChannel, UInt_t nUserSubEvtId, UInt_t nUserEdge, UInt_t nUserCoarseTime, UInt_t nUserFineTime, Bool_t bUserIsRefChannel, Bool_t bUserVerboseMode=kFALSE); // constructor
	~TTrbHit(); // destructor
	void Clear(Option_t* option);
	virtual void Print(Option_t* option=NULL) const; // print hit information to terminal
	void Set(UInt_t nUserTrbAddress, UInt_t nUserTdcChannel, UInt_t nUserSubEvtId, UInt_t nUserTdcErrCode, UInt_t nUserEdge,
	         UInt_t nUserEpochCounter, UInt_t nUserCoarseTime, UInt_t nUserFineTime, Bool_t bUserIsRefChannel); // set hit information
	void SetCalibratedTime(Double_t fUserTime) { fTime = fUserTime; bIsCalibrated = kTRUE; };
	void SetVerboseMode(Bool_t bUserVerboseMode) { bVerboseMode = bUserVerboseMode; };

	/* some magic ROOT stuff... */
	ClassDef(TTrbHit,1);
};

#endif
