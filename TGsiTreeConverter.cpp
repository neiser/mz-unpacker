#include "TGsiTreeConverter.h"

ClassImp(TGsiTreeConverter);

TGsiTreeConverter::TGsiTreeConverter(string cUserDataFilename, string cUserTdcAddressFile, UInt_t nUserTdcOffset, UInt_t nUserTdcWidth) : TDircAnalysisBase(cUserDataFilename, cUserTdcAddressFile,nUserTdcOffset,nUserTdcWidth) {
	cout << "Initialising TGsiTreeConverter..." << endl;
	Init();
}

TGsiTreeConverter::TGsiTreeConverter(string cUserDataFilename, string cUserTdcAddressFile) : TDircAnalysisBase(cUserDataFilename, cUserTdcAddressFile) {
	cout << "Initialising TGsiTreeConverter..." << endl;
	Init();
}

TGsiTreeConverter::~TGsiTreeConverter(){
	if(OnlineTree!=NULL){
		delete OnlineTree;
		OnlineTree = NULL;
	}
	CleanUp();
}

void TGsiTreeConverter::AddBranches(){
	if(OnlineTree==NULL) // TTree does not exist
		return; // do nothing and return
	// declare tree leafs
	stringstream cLeafList; // string to store leaf type declaration
	// leading edge data
	cLeafList << "fLeadingEdge[" << GetSizeOfMapTable() << "]/D";
	OnlineTree->Branch("fLeadingEdge",EventData.fLeadingEdge,cLeafList.str().c_str());
	// trailing edge data
	cLeafList << "fTrailingEdge[" << GetSizeOfMapTable() << "]/D";
	OnlineTree->Branch("fTrailingEdge",EventData.fTrailingEdge,cLeafList.str().c_str());
	// time-over-threshold data
	cLeafList.str(""); // clear leaf descriptor
	cLeafList << "fTot[" << GetSizeOfMapTable() << "]/D";
	OnlineTree->Branch("fTot",EventData.fTot,cLeafList.str().c_str());
	// reference time data
	cLeafList.str(""); // clear leaf descriptor
	cLeafList << "fSync[" << GetSizeOfMapTable() << "]/D";
	OnlineTree->Branch("fSync",EventData.fSync,cLeafList.str().c_str());
	// TDC ID data
	cLeafList.str(""); // clear leaf descriptor
	cLeafList << "nTdcId[" << GetSizeOfMapTable() << "]/i";
	OnlineTree->Branch("nTdcId",EventData.nTdcId,cLeafList.str().c_str());
	// TDC channel data
	cLeafList.str(""); // clear leaf descriptor
	cLeafList << "nTdcChan[" << GetSizeOfMapTable() << "]/i";
	OnlineTree->Branch("nTdcChan",EventData.nTdcChan,cLeafList.str().c_str());
	// channel hit multipicity
	cLeafList.str(""); // clear leaf descriptor
	cLeafList << "nMult[" << GetSizeOfMapTable() << "]/i";
	OnlineTree->Branch("nMult",EventData.nMult,cLeafList.str().c_str());
	// MCP ID data
	cLeafList.str(""); // clear leaf descriptor
	cLeafList << "nSeqId[" << GetSizeOfMapTable() << "]/i";
	OnlineTree->Branch("nSeqId",EventData.nSeqId,cLeafList.str().c_str());
	// data valid flag
	cLeafList.str(""); // clear leaf descriptor
	cLeafList << "bValid[" << GetSizeOfMapTable() << "]/O";
	OnlineTree->Branch("bValid",EventData.bIsValid,cLeafList.str().c_str());
	// Evt no
	cLeafList.str(""); // clear leaf descriptor
	cLeafList << "nEvtId/i";
	OnlineTree->Branch("nEvtId",&EventData.nEvtId,cLeafList.str().c_str());
	// Number of hits in event
	cLeafList.str(""); // clear leaf descriptor
	cLeafList << "nHits/i";
	OnlineTree->Branch("nHits",&EventData.nHits,cLeafList.str().c_str());

}

void TGsiTreeConverter::Analyse(string cUserAnalysisFilename){
	TFile *AnalysisOut = new TFile(cUserAnalysisFilename.c_str(),"RECREATE"); // open RooT file for analysis results
	// define histograms
	TH1D hEvtStats("hEvtStats","hEvtStats; ; frequency",15,-0.5,14.5);
	hEvtStats.GetXaxis()->SetBinLabel(NO_CUTS+1,"no cuts");
	hEvtStats.GetXaxis()->SetBinLabel(DECODE_ERR+1,"decoding error");
	hEvtStats.GetXaxis()->SetBinLabel(RNDM_BIT_ERR+1,"random bit error");
	hEvtStats.GetXaxis()->SetBinLabel(SYNC_ERR+1,"TDC sync error");
	hEvtStats.GetXaxis()->SetBinLabel(NO_HITS_ERR+1,"no TDC hits error");
	hEvtStats.GetXaxis()->SetBinLabel(NO_MATCH_ERR+1,"no matched hits error");
	hEvtStats.GetXaxis()->SetBinLabel(NO_TRIG_ERR+1,"no event trigger error");

	TH1D hEvtMultiplicity("hEvtMultiplicity","hEvtMultiplicity;",50,-0.5,49.5); // number of hit pixels per event
	TH1D hPixelHits("hPixelHits","hPixelHits; pixel ID; frequency",(Int_t)GetSizeOfMapTable()+2,-0.5,GetSizeOfMapTable()+1.5); // distribution of pixel hits
	TH1D hMultiHitPixels("hMultiHitPixels","hMultiHitPixels; no of channels with mult hits per event; frequency",20,-0.5,19.5);
	TH1D hLETiming("hLETiming","Synchronised Leading Edge Timing; sync LE time (ns); frequency",5000,-1000.0,0.0); // synchronised leading edge time of all channels
	TH2D hHitToT("hHitToT","hHitToT;pixel ID; Time-over-Threshold (ns); frequency",GetSizeOfMapTable()+2,-0.5,GetSizeOfMapTable()+1.5,1000,-1.0,50.0); // hit Time-over-Threshold for all channels
	TH1D hTriggerTime("hTriggerTime","hTriggerTime;trigger LE (ns); frequency",1000,-500.0,0.0);
	// main analysis loop
	Int_t nEvents = GetNEvents();
	Int_t nFraction = nEvents * 0.1;
	for(Int_t i=0; i<nEvents; i++){ // begin loop over all events
		// first, get event data
		if (GetEntry(i)<1){ // this entry is not valid
			continue; // skip rest of loop
		}
		// compute progress
		if(i%nFraction==0){
			cout << (i/nFraction)*(1.0/0.1) << "% of events analysed...\r" ;
		}
		hEvtStats.Fill((Double_t)NO_CUTS);
		// now check data quality
		if(!CheckDecodingStatus()) // check if there were any problems during conversion
			continue; // skip rest of loop
		hEvtStats.Fill((Double_t)DECODE_ERR);
		if(!CheckRandomBits()){ // random bits in event do not match
			continue; // skip rest of loop
		}
		hEvtStats.Fill((Double_t)RNDM_BIT_ERR);
		UInt_t nSyncTimestampsFound = GetNSyncTimestamps();
		if(nSyncTimestampsFound!=GetNTdcs()){ // check if all TDCs have a sync timestamp
			continue; // skip rest of loop
		}
		hEvtStats.Fill((Double_t)SYNC_ERR);
		if(EvtTdcHits.empty()){ // check if there are no TDC hits in this event
			continue; // skip rest of loop
		}
		hEvtStats.Fill((Double_t)NO_HITS_ERR);
		if(EvtReconHits.empty()){
			continue; // skip rest of loop
		}
		hEvtStats.Fill((Double_t)NO_MATCH_ERR);
		Double_t fTrigTime;
		if(!GetTriggerTime(fTrigTime)){ // no event trigger found
			//continue;
		}
		hEvtStats.Fill((Double_t)NO_TRIG_ERR);
		//PrintMatchedHits();
		hEvtMultiplicity.Fill((Double_t)GetNMatchedHits());
		hMultiHitPixels.Fill((Double_t)GetNMultiHits());
		hTriggerTime.Fill(fTrigTime);
		std::map< UInt_t,std::list<PixelHitModel> >::const_iterator FirstChannel	= EvtReconHits.begin();
		std::map< UInt_t,std::list<PixelHitModel> >::const_iterator LastChannel	= EvtReconHits.end();
		std::map< UInt_t,std::list<PixelHitModel> >::const_iterator CurChannel;
		for(CurChannel=FirstChannel; CurChannel!=LastChannel; ++CurChannel){ // begin of loop over all matched hits
			std::list<PixelHitModel>::const_iterator FirstHit	= CurChannel->second.begin();
			std::list<PixelHitModel>::const_iterator LastHit	= CurChannel->second.end();
			std::list<PixelHitModel>::const_iterator CurHit;
			for(CurHit=FirstHit; CurHit!=LastHit; ++CurHit){ // begin of loop over all hits in this channel
				hPixelHits.Fill((Double_t)CurHit->nChannelA);
				hPixelHits.Fill((Double_t)CurHit->nChannelB);
				hLETiming.Fill(CurHit->fSyncLETime);
				hHitToT.Fill((Double_t)CurHit->nChannelA,CurHit->fTimeOverThreshold);
			}
		}
	} // end of loop over all events
	cout << endl;
	AnalysisOut->Write(); // write all histograms in memeory to this file
	delete AnalysisOut; // close RooT file and delete pointer
}

void TGsiTreeConverter::CleanUp(){ // delete arrays holding data for tree conversion
	if(EventData.fLeadingEdge!=NULL){
		delete[] EventData.fLeadingEdge;
		EventData.fLeadingEdge = NULL;
	}
	if(EventData.fTrailingEdge!=NULL){
		delete[] EventData.fTrailingEdge;
		EventData.fTrailingEdge = NULL;
	}
	if(EventData.fTot!=NULL){
		delete[] EventData.fTot;
		EventData.fTot = NULL;
	}
	if(EventData.fSync!=NULL){
		delete[] EventData.fSync;
		EventData.fSync = NULL;
	}
	if(EventData.nTdcId!=NULL){
		delete[] EventData.nTdcId;
		EventData.nTdcId = NULL;
	}
	if(EventData.nTdcChan!=NULL){
		delete[] EventData.nTdcChan;
		EventData.nTdcChan = NULL;
	}
	if(EventData.nSeqId!=NULL){
		delete[] EventData.nSeqId;
		EventData.nSeqId = NULL;
	}
	if(EventData.bIsValid!=NULL){
		delete[] EventData.bIsValid;
		EventData.bIsValid = NULL;
	}
}

void TGsiTreeConverter::ConvertTree(string cUserAnalysisFilename, UInt_t nStartIndex, UInt_t nStopIndex){
	if(nStartIndex > nStopIndex) // check if start and stop index are in the right order
		std::swap(nStartIndex,nStopIndex);
	if(nStopIndex>(UInt_t)nEvtsInTree) // check if stop index is within range of event numbers
		nStopIndex = (UInt_t)nEvtsInTree;
	// declare array size based on mapping table size
	EventData.fLeadingEdge	= new Double_t[nArraySize];
	EventData.fTrailingEdge	= new Double_t[nArraySize];
	EventData.fTot			= new Double_t[nArraySize];
	EventData.fSync			= new Double_t[nArraySize];
	EventData.nTdcId		= new UInt_t[nArraySize];
	EventData.nTdcChan		= new UInt_t[nArraySize];
	EventData.nMult			= new UInt_t[nArraySize];
	EventData.nSeqId		= new UInt_t[nArraySize];
	EventData.bIsValid		= new Bool_t[nArraySize];
	// open ROOT output file
	OutputFile = new TFile(cUserAnalysisFilename.c_str(),"RECREATE"); // open RooT file for analysis results
	// create ROOT tree
	OnlineTree = new TTree("A","Compact Tree for Online Analysis");
	//OnlineTree->SetDirectory(0); // remove this tree from current directory
	AddBranches(); // add branches for storing data
	// loop over data and fill into trees
	Int_t nFraction = (nStopIndex-nStartIndex) * 0.1;
	for(Int_t i=nStartIndex; i<nStopIndex; i++){ // begin loop over all events
		InitArrays();
		// first, get event data
		if (GetEntry(i)<1){ // this entry is not valid
			continue; // skip rest of loop
		}
		// need to reset all data arrays first!!!
		if(EvtReconHits.empty()){
			continue;
		}
		std::map< UInt_t,std::list<PixelHitModel> >::const_iterator FirstChannel = EvtReconHits.begin();
		std::map< UInt_t,std::list<PixelHitModel> >::const_iterator LastChannel	 = EvtReconHits.end();
		std::map< UInt_t,std::list<PixelHitModel> >::const_iterator CurChannel;
		for(CurChannel=FirstChannel; CurChannel!=LastChannel; ++CurChannel){ // begin of loop over all matched hits
			EventData.fLeadingEdge[CurChannel->first] = CurChannel->second.back().GetLeadEdgeTime(); // get leading edge time of last hit for this channel
			EventData.fTrailingEdge[CurChannel->first] = GetTime(CurChannel->second.back().nChannelBIndex) - GetTdcSyncTimestamp(GetTdcAddress(CurChannel->second.back().nChannelBIndex));  // get trailing edge time of last hit for this channel
			EventData.fTot[CurChannel->first] = CurChannel->second.back().GetToT();
			EventData.fSync[CurChannel->first] = GetTdcSyncTimestamp(GetTdcAddress(CurChannel->second.back().nChannelBIndex));
			Int_t nTempTdcId, nTempTdcChan;
			GetChannelAddress(CurChannel->first,nTempTdcId,nTempTdcChan);
			EventData.nTdcId[CurChannel->first] = (UInt_t)nTempTdcId;
			EventData.nTdcChan[CurChannel->first] = (UInt_t)nTempTdcChan;
			EventData.nMult[CurChannel->first] = (UInt_t)CurChannel->second.size();
			EventData.nSeqId[CurChannel->first] = CurChannel->second.back().GetLeadEdgeChan();
			EventData.bIsValid[CurChannel->first] = kTRUE;

		} // end of loop over all matched hits in this event
		EventData.nEvtId = TrbData->nEvtSeqNr;
		EventData.nHits = TrbData->Hits_;
		OnlineTree->Fill(); // write data to tree memory
	} // end of loop over all entries
	OutputFile->Write("",TObject::kOverwrite);
	delete OutputFile; // delete file object, this triggers writing of the tree to disc and properly closing the file
	OnlineTree = NULL;
	OutputFile = NULL;
	CleanUp();
}


void TGsiTreeConverter::Init(){
	OnlineTree = NULL;
	OutputFile = NULL;
	EventData.fLeadingEdge	= NULL;
	EventData.fTrailingEdge	= NULL;
	EventData.fTot			= NULL;
	EventData.fSync			= NULL;
	EventData.nTdcId		= NULL;
	EventData.nTdcChan		= NULL;
	EventData.nMult			= NULL;
	EventData.nSeqId		= NULL;
	EventData.bIsValid		= NULL;

	cout << "Get stats..." << endl;
	nEvtsInTree = GetNEvents();
	cout << nEvtsInTree << endl;
	nArraySize	= GetSizeOfMapTable();
}

void TGsiTreeConverter::InitArrays(){
	for(Int_t i=0; i<nArraySize; ++i){
		EventData.fLeadingEdge[i]	= -9999.0;
		EventData.fTrailingEdge[i]	= -9999.0;
		EventData.fTot[i]			= -9999.0;
		EventData.fSync[i]			= -9999.0;
		EventData.nTdcId[i]			= -1;
		EventData.nTdcChan[i]		= -1;
		EventData.nMult[i]			= 0;
		EventData.nSeqId[i]			= -1;
		EventData.bIsValid[i]		= kFALSE;
	}
}