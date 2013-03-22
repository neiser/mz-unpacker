#ifndef _T_TRBUNPACKER_H
#define _T_TRBUNPACKER_H
// +++ include header files +++
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <locale>
#include <string>
#include <vector>

#include "TClonesArray.h"
#include "TFile.h"
#include "TObject.h"
#include "TTree.h"

#include "TrbStructs.h"
#include "myUtilities.h"
#include "TTrbEventData.h"
#include "THldEvent.h"
#include "TTrbHit.h"

#define SIZE_OF_DATAWORD 4
#define NO_OF_TDC_ADDRESSES 16
#define TREE_AUTOSAVE 300000000 // set to 300 MBytes


/* +++ WORK TO DO +++
- detailed error handling
- check unpacker status before decoding
- write unpacker logfile (redirect clog for this purpose)
*/


// +++ class definition +++
class TTrbUnpacker : public TObject{
	//friend class TTrbEventData;
private:
	ifstream InputHldFile; // input file stream object to read from raw HLD file
	ofstream LogFile; // output file stream object for log file
	//streambuf *ClogBackup; // rdbuf backup of standard clog, will redirect clog to log file
	TFile* OutputRootFile; // pointer to ROOT file for storing decoded data
	TTree* OutputTree; // pointer to ROOT TTree object for data storage
	Bool_t CloseHldFile(); // close raw HLD file stream
	Bool_t CloseLogFile(); // close logfile
	Bool_t CreateTree(); //  create ROOT Tree
	void IndexEvents(); // create an index of event position within HLD file
	void Init(); // initialise variables
	Bool_t OpenHldFile(); // open raw HLD file and assign it to ifstream object
	Bool_t OpenLogFile(); // open logfile
	Bool_t OpenRootFile(); // open ROOT file for data storage
	void RewindFile() { InputHldFile.seekg(0,ios::beg); }; // set file get pointer to beginning of HLD file
	void SetHldFilename(string cUserFilename) { cHldFilename=cUserFilename; };
	void SetLogFilename();
	void SetRootFilename(); // set file name of output RooT file
	Int_t SetTdcAddresses(string cUserTdcAddressesFile); // set TDC addresses using list in file
	Int_t SetHubAddresses(string cUserHubAddressesFile); // set HUB addresses using list in file
	void CheckHubTdcAddresses();
	//void WriteSettingsToLog();
protected:
	string cHldFilename; // HLD file name as provided by the user
	string cLogFilename; // log file name derived from HLD file name
	string cRootFilename; // ROOT file name derived from HLD file name
	std::vector<string> cHubAddresses; // vector containing HUB addresses as strings
	std::vector<string> cTdcAddresses; // vector containing TDC addresses as strings
	std::vector<Int_t> nEvtIndex; // vector containing raw file positions of events
	TRB_SETUP TrbSettings;
	Bool_t bSkipSubEvents;
	Bool_t bVerboseMode;
	
public:
	TTrbUnpacker(string cUserHldFilename, UInt_t cUserSubEventId, UInt_t cUserCtsAddress, string cUserHubAddressesFile, string cUserTdcAddressesFile,
	             UInt_t nUserTdcRefChannel, Bool_t bUserVerboseMode=kFALSE, Bool_t bUserSkipSubEvents=kFALSE); // constructor
	~TTrbUnpacker(); // destructor
	UInt_t Decode(UInt_t nUserEvents, UInt_t nUserOffset=0); // start decoding of HLD file
	Int_t GetEntryPositon(UInt_t nUserEvtIndex) const { return (nEvtIndex.at(nUserEvtIndex)); };
	UInt_t GetHldEntries() const { return ((UInt_t)nEvtIndex.size()); }; // return size of event index vector
	void PrintSubEventId();
	void PrintCtsAddress();
	void PrintHubAddresses(Bool_t bWriteToLog=kFALSE);
	void PrintTdcAddresses(Bool_t bWriteToLog=kFALSE);
	void PrintTdcRefChannel();
	void PrintUnpackerSettings();
	/* some magic ROOT stuff... */
	ClassDef(TTrbUnpacker,1);


};
#endif
