#ifndef _T_HLD_SUBEVENT_H
#define _T_HLD_SUBEVENT_H

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
//#include "TTrbEventData.h"
#include "TTrbHit.h"
#include "myUtilities.h"

#define NO_ERR_BITS 8
#define SEB_ERROR_CODE 0x00000001
#define TRB_HEADER_MARKER 0xC0
#define TDC_HEADER_MARKER 0b001
#define TDC_EPOCH_MARKER  0b011
#define TDC_DEBUG_MARKER  0b010

/* +++ ERROR BITS DEFINITION +++
	BIT 0 -> No global TRB header found
	BIT 1 -> No TRB board header found
	BIT 2 -> No TDC header found
	BIT 3 -> No TDC data available
	BIT 4 -> Wrong TDC data word (header word duplicated)
*/

/* +++ WORK TO DO +++
- implement error codes

*/

// +++ class definition +++
class THldSubEvent : public TObject{
	friend class TTrbEventData;
	friend class TTrbUnpacker;
	friend void ClearTdcHeader(TDC_HEADER& TdcHeader);
private:
	ifstream* HldFile;
	Bool_t CheckTrbAddress(UInt_t nUserTrbAddress); // check decoded TRB address against user provided list
	void DecodeBaseEventSize();
	//Bool_t DecodeTdcHeader(std::vector<UInt_t>::const_iterator DataWord, UInt_t& nTdcRandomBits, UInt_t& nTdcErrorCode);
	Bool_t DecodeTdcHeader(std::vector<UInt_t>::const_iterator DataWord, TDC_HEADER& TdcHeader);
	Bool_t DecodeTdcWord(std::vector<UInt_t>::const_iterator DataWord, UInt_t nUserTrbAddress, TDC_HEADER& TdcHeader);
	Bool_t DecodeTrbHeader(std::vector<UInt_t>::const_iterator DataWord, UInt_t& nTrbAddress, size_t& nTrbWords);
	void Init(); // initialise variables etc.
	Bool_t ReadHeader(); // read subevent header words
	Bool_t ReadTrailer(); // read subevent trailer words
	Bool_t ReadTrbData(); // read TRB TDC data words
	void SwapHeaderWords(); // swap bytes of header words from big Endian to little Endian
	void SwapTrailerWords(); // swap bytes of trailer words from big Endian to little Endian
protected:
	const TRB_SETUP* TrbSettings; // pointer to structure containing setup information of the TRB system
	SUB_HEADER SubEventHeader; // structure for storing HLD subevent header information
	SUB_TRAILER SubEventTrailer; // structure for storing HLD subevent trailer information
	std::bitset<NO_ERR_BITS> ErrorCode;
	Bool_t bIsValid; // flag indicating validity of subevent, based on subevent trailer information
	Bool_t bVerboseMode; // control level of printing messages
	size_t nBaseEventSize; // base data word size (should be 4)
	size_t nDataBytes; // number of data bytes in subevent
	size_t nDataWords; // number of data words in subevent
	size_t nSubEventSize; // number of bytes read from HLD file 
	UInt_t nNumberOfTrbsFound; // number of TRB boards found in subevent
	UInt_t nNumberOfTdcsFound; // number of TDCs found in subevent (there should be 4 TDCs per TRBv3 board)
	size_t nTrbWordsRead; // not used at the moment
	std::vector<UInt_t> nTrbData; // vector containing TRBv3 data words, excluding header and trailer information
	TClonesArray* Hits;
	UInt_t nTdcHits; // number of TDC hits in subevent
public:
	THldSubEvent(ifstream* UserHldFile, const TRB_SETUP* UserTrbSettings, TClonesArray* UserArray, Bool_t bUserVerboseMode=kFALSE); // constructor
	~THldSubEvent(); // destructor
	Bool_t Decode(); // read & decode TRB subevent data
	std::bitset<NO_ERR_BITS> GetErrorStatus() const { return (ErrorCode); };
	size_t GetNBytes() { return (nSubEventSize); }; // get number of bytes read from HLD file
	UInt_t GetNTdcHits() const { return (nTdcHits); };
	void PrintHeader(); 
	void PrintTrailer();
	void SetVerboseMode(Bool_t bUserVerboseMode) { bVerboseMode = bUserVerboseMode; };
	/* some magic ROOT stuff... */
	ClassDef(THldSubEvent,1);
};

#endif
