#ifndef _T_HLD_EVENT_H
#define _T_HLD_EVENT_H
// +++ include header files +++
#include <algorithm>
#include <bitset>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "TObject.h"

#include "TrbStructs.h"
//#include "TTrbEventData.h"
#include "THldSubevent.h"
#include "myUtilities.h"

// +++ class definition +++
class THldEvent : public TObject{
	friend class TTrbEventData;
	friend class TTrbUnpacker;
private:
	ifstream* HldFile;
	size_t nDataBytes;
	size_t nDataWords;
	Bool_t CheckErrState();
	void DecodeBaseEventSize();
	void Init();
	Bool_t ReadHeader();
	size_t SkipPaddingBytes(size_t nWordsRead);
	//Bool_t ReadSubEvent();
	//std::bitset<NO_ERR_BITS> IgnoreErrCode;
protected:
	HLD_HEADER EventHeader;
	THldSubEvent* SubEventData;
	Bool_t bVerboseMode;
	Bool_t bIgnoreEvent;
	Bool_t bSkipSubEvent;
	Bool_t bHasSubEvent;
	size_t nBaseEventSize;
	TClonesArray* Hits;
	const TRB_SETUP* TrbSettings;
	const std::bitset<NO_ERR_BITS>* SubEvtErrCode;
public:
	THldEvent(ifstream* UserHldFile, const TRB_SETUP* UserTrbSettings, TClonesArray* UserArray, Bool_t bUserVerboseMode=kFALSE, Bool_t bUserSkipSubEvent=kFALSE); // constructor
	//THldEvent(ifstream* UserHldFile, UInt_t nUserSubEventId, Bool_t bUserVerboseMode=kFALSE, Bool_t bUserSkipSubEvent=kFALSE); // constructor
	~THldEvent(); // destructor
	const HLD_HEADER* GetEvtHeader() const { return (&EventHeader); };
	size_t GetHeaderSize() const { return (sizeof(HLD_HEADER)); };
	void PrintHeader();
	Bool_t ReadIt(Bool_t bApplyPadding=kTRUE);
	//void SetErrorState(std::bitset<NO_ERR_BITS> UserErrState);
	/* some magic ROOT stuff... */
	ClassDef(THldEvent,1);
};
#endif
