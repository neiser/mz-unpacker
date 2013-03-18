#include "TTrbHit.h"

ClassImp(TTrbHit);

//TTrbHit::TTrbHit(UInt_t nUserTrbAddress, UInt_t nUserTdcChannel, UInt_t nUserSubEvtId, UInt_t nUserEdge, UInt_t nUserCoarseTime, UInt_t nUserFineTime, Bool_t bUserIsRefChannel, Bool_t bUserVerboseMode){ // constructor
//	bVerboseMode = bUserVerboseMode;
//	Init();
//	SetTrbAddress(nUserTrbAddress);
//	SetTdcChannel(nUserTdcChannel);
//	SetSubEvtId(nUserSubEvtId);
//	SetSignalEdge(nUserEdge);
//	SetCoarseTime(nUserCoarseTime);
//	SetFineTime(nUserFineTime);
//	SetIsRefChannel(bUserIsRefChannel);
//	ComputeTime();
//	if(bVerboseMode)
//		Print();
//}

TTrbHit::~TTrbHit(){ // destructor

}

void TTrbHit::Clear(Option_t *option) {
	(void)option;
	Init();
}

void TTrbHit::Init(){ // initialise variables etc
	nTrbAddress = 0;
	nTdcChannel = 0;
	nSubEvtId	= 0;
	nTdcErrCode	= 0;
	nSignalEdge = 0;
	nEpochCounter = 0;
	nCoarseTime = 0;
	nFineTime	= 0;
	fTime		= -9999.0;
	bIsCalibrated	= kFALSE;
	bIsRefChannel	= kFALSE;
	bVerboseMode	= kFALSE;
}

void TTrbHit::Print(Option_t* option) const { // print hit information to terminal
	(void)option; // suppress warning
	cout << "+++ TRB HIT Data +++" << endl;
	cout << "TRB Address:\t" << hex << nTrbAddress << dec << endl;
	cout << "TDC Channel:\t" << nTdcChannel << endl;
	cout << "SubEvt ID:\t" << hex << nSubEvtId << dec << endl;
	cout << "TDC Error Code:\t" << hex << nTdcErrCode << dec << endl;
	cout << "Signal Edge:\t" << nSignalEdge << endl;
	cout << "Epoch Counter:\t" << nEpochCounter << endl;
	cout << "Coarse Time:\t" << nCoarseTime << endl;
	cout << "Fine Time:\t" << nFineTime << endl;
	cout << "Time:\t" << fTime << endl;
	if(bIsRefChannel)
		cout << "This is the reference channel!" << endl;
	cout << "++++++++++++++++++++" << endl;
}

void TTrbHit::Set(UInt_t nUserTrbAddress, UInt_t nUserTdcChannel, UInt_t nUserSubEvtId, UInt_t nUserTdcErrCode, UInt_t nUserEdge,
                  UInt_t nUserEpochCounter, UInt_t nUserCoarseTime, UInt_t nUserFineTime, Bool_t bUserIsRefChannel){
	nTrbAddress = nUserTrbAddress;
	nTdcChannel = nUserTdcChannel;
	nSubEvtId	= nUserSubEvtId;
	nTdcErrCode	= nUserTdcErrCode;
	nSignalEdge = nUserEdge;
	nEpochCounter = nUserEpochCounter;
	nCoarseTime = nUserCoarseTime;
	nFineTime	= nUserFineTime;
	bIsRefChannel = bUserIsRefChannel;

	if(bVerboseMode)
		Print();
}
