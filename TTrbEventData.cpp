#include "TTrbEventData.h"

ClassImp(TTrbEventData);

TTrbEventData::TTrbEventData(){
	Init();
}

TTrbEventData::TTrbEventData(TClonesArray& UserArray){
	Init();
	Hits = &UserArray;
}

TTrbEventData::~TTrbEventData(){
	//if(bVerboseMode)
		//cout << "Calling TRB Event Data destructor..." << endl;
	//delete Hits;
}

void TTrbEventData::AddEvtHeader(HLD_HEADER& UserEvtHeader){
	nEvtSize		= UserEvtHeader.nSize;
	nEvtDecoding	= UserEvtHeader.nDecoding;
	nEvtId			= UserEvtHeader.nId;
	nEvtSeqNr		= UserEvtHeader.nSeqNr;
	nEvtDate		= UserEvtHeader.nDate;
	nEvtTime		= UserEvtHeader.nTime;
	nEvtRun			= UserEvtHeader.nRun;
	nEvtPad			= UserEvtHeader.nPad;
}

void TTrbEventData::AddSubEvt(THldEvent& UserHldEvent){
	nSubEvtSize		= UserHldEvent.SubEventData->SubEventHeader.nSize; // subevent size in bytes
	nSubEvtDecoding = UserHldEvent.SubEventData->SubEventHeader.nDecoding; // subevent decoding settings
	nSubEvtId		= UserHldEvent.SubEventData->SubEventHeader.nEventId; // subevent ID (should be 0x8c00 for TRBv3)
	nSubEvtTrigger	= UserHldEvent.SubEventData->SubEventHeader.nTrigger; // subevent trigger number
	nCTSExtTrigger = UserHldEvent.SubEventData->nCTSExtTrigger;  // trigger number from CTS external trigger module (ETM)
	nCTSExtTriggerStatus = UserHldEvent.SubEventData->nCTSExtTriggerStatus; // some status for CTS ETM
	nSebErrCode		= UserHldEvent.SubEventData->SubEventTrailer.nSebError; // subevent error code
	nTrbs			= UserHldEvent.SubEventData->nNumberOfTrbsFound;
	nTdcs			= UserHldEvent.SubEventData->nNumberOfTdcsFound;
	nSubEvtDecError = (UInt_t)UserHldEvent.SubEventData->ErrorCode.to_ulong();
}

void TTrbEventData::AddSubEvt(SUB_HEADER& UserSubEvtHeader,
                              UInt_t nUserCTSExtTrigger,
                              UInt_t nUserCTSExtTriggerStatus,
                              UInt_t nUserSebErrCode,
                              UInt_t nUserTrbs,
                              UInt_t nUserTdcs,
                              UInt_t nUserSubEvtDecError){
	nSubEvtSize		= UserSubEvtHeader.nSize;
	nSubEvtDecoding = UserSubEvtHeader.nDecoding;
	nSubEvtId		= UserSubEvtHeader.nEventId;
	nSubEvtTrigger	= UserSubEvtHeader.nTrigger;
	nCTSExtTrigger = nUserCTSExtTrigger;  
	nCTSExtTriggerStatus = nUserCTSExtTriggerStatus; 
	nSebErrCode		= nUserSebErrCode;
	nTrbs			= nUserTrbs;
	nTdcs			= nUserTdcs;
	nSubEvtDecError	= nUserSubEvtDecError;
}

void TTrbEventData::Fill(THldEvent& UserHldEvent){
	// fill event & subevent header and trailer information
	AddEvtHeader(UserHldEvent.EventHeader);
	if(UserHldEvent.bHasSubEvent)
		AddSubEvt(UserHldEvent);
	else
		nSebErrCode = 0;
}

void TTrbEventData::Init(){
	Hits = NULL;
	// initialise HLD event header data
	nEvtSize		= 0;
	nEvtDecoding	= 0;
	nEvtId			= 0;
	nEvtSeqNr		= 0; // sequential trigger number, use to identfy event
	nEvtDate		= 0;
	nEvtTime		= 0;
	nEvtRun			= 0;
	nEvtPad			= 0;

	nSubEvtSize		= 0;
	nSubEvtDecoding = 0;
	nSubEvtId		= 0;
	nSubEvtTrigger	= 0;
	nSebErrCode		= 0;

	nCTSExtTrigger = 0;
	nCTSExtTriggerStatus = 0;
	
	nTrbs = 0;
	nTdcs = 0;
	nSubEvtDecError = 0;
}
