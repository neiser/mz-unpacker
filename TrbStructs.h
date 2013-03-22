#ifndef _TRB_STRUCTS_H
#define _TRB_STRUCTS_H

struct TRB_SETUP{
	UInt_t nSubEventId; // subevent ID (should be 0x8c00)
	UInt_t nTdcRefChannel; // TRBv3 TDC reference channel
	UInt_t nCtsAddress; // TRB address of Central Trigger System (TCS), should be 0x0002
	std::vector<UInt_t> nHubAddress; // vector of TRBv3 addresses of HUBs (can be empty)
	std::vector<UInt_t> nTdcAddress; // vector of TRBv3 addresses of TDC endpoints
};

struct HLD_HEADER { // HLD header description
	UInt_t nSize; // size of event in 4-byte words
	UInt_t nDecoding;
	UInt_t nId;
	UInt_t nSeqNr; // sequential trigger number, use to identfy event
	UInt_t nDate;
	UInt_t nTime;
	UInt_t nRun;
	UInt_t nPad;
};

struct SUB_HEADER{
	UInt_t nSize; // subevent size in bytes
	UInt_t nDecoding; // subevent decoding settings
	UInt_t nEventId; // subevent ID (should be 0x8c00 for TRBv3)
	UInt_t nTrigger; // subevent trigger number
};

struct SUB_TRAILER{
	UInt_t nSebHeader;
	UInt_t nSebError; // this is the subevent builder error code (must be 0x00000001 otherwise event is corrupted)
};

struct TDC_HEADER{ // TRBv3 TDC header information
	UInt_t nRandomBits; // random code, generated individually for each event
	UInt_t nErrorBits; // TDC errors are indicated here (0 in case of no errors)
};

#endif
