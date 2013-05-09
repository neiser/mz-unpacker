#ifndef _T_TRB_EVENTDATA_H
#define _T_TRB_EVENTDATA_H
// +++ include header files +++
#include <algorithm>
#include <bitset>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "TClonesArray.h"
#include "TObject.h"

#include "TrbStructs.h"
#include "myUtilities.h"
#include "TTrbHit.h"
#include "THldEvent.h"
#include "THldSubevent.h"

class THldEvent;

// +++ class definitions +++
class TTrbEventData : public TObject{
private:
	void Init();
protected:
	// HLD Event Header Data
	UInt_t nEvtSize; // size of event in 4-byte words
	UInt_t nEvtDecoding;
	UInt_t nEvtId;
	UInt_t nEvtSeqNr; // sequential trigger number, use to identfy event
	UInt_t nEvtDate;
	UInt_t nEvtTime;
	UInt_t nEvtRun;
	UInt_t nEvtPad;
	// HLD Subevent information
	UInt_t nSubEvtSize; // subevent size in bytes
	UInt_t nSubEvtDecoding; // subevent decoding settings
	UInt_t nSubEvtId; // subevent ID (should be 0x8c00 for TRBv3)
	UInt_t nSubEvtTrigger; // subevent trigger number
	UInt_t nSebErrCode; // subevent builder error code
	UInt_t nTrbs; // number of TRB boards in subevent
	UInt_t nTdcs; // number of TDCs in subevent
	UInt_t nCTSExtTrigger; // central trigger system (CTS) external trigger module (ETM)
	UInt_t nCTSExtTriggerStatus; // status of CTS ETM
	// Unpacker information
	UInt_t nSubEvtDecError; // error bit pattern from subevent decoding
	// remaining data
	TClonesArray *Hits; //array with all hits
public:
	TTrbEventData(); // empty constructor needed for TTree stuff
	TTrbEventData(TClonesArray& UserArray);
	//TTrbEventData(THldEvent& UserHldEvent, TClonesArray* UserHitArray);
	~TTrbEventData();
	void AddEvtHeader(HLD_HEADER& UserEvtHeader);
	void AddSubEvt(THldEvent& UserHldEvent);
	void AddSubEvt(SUB_HEADER& UserSubEvtHeader,
	               UInt_t nUserCTSExtTrigger,
	               UInt_t nUserCTSExtTriggerStatus,             
	               UInt_t nUserSebErrCode, UInt_t nUserTrbs, UInt_t nUserTdcs, UInt_t nUserSubEvtDecError);
	void Fill(THldEvent& UserHldEvent);
	/* some magic ROOT stuff... */
	ClassDef(TTrbEventData,1);
};


#endif
