#include "TTrbAnalysis.h"

ClassImp(TTrbAnalysis);

TTrbAnalysis::TTrbAnalysis(string cUserDataFilename, string cUserTdcAddressesFile, Bool_t bUserVerboseMode){
	bVerboseMode = bUserVerboseMode; // set verbose mode flag according to user input
	Init(); // initialise variables
	nTrbEndpoints	= SetTrbAddresses(cUserTdcAddressesFile); // decode TRB board addresses provided by user
	if(nTrbEndpoints<1) // no TRB boards defined
		exit (-1);
	if(!OpenTrbTree(cUserDataFilename)) // check if we can open TRB data tree
		exit (-1);
}

TTrbAnalysis::~TTrbAnalysis(){
	delete TrbData;
}

void TTrbAnalysis::Analyse(string cUserAnalysisFilename){
	/* ======================================= *\
	+ place for user analysis code				+
	+ only edit this function to adapt analysis +
	+ DO NOT CHANGE ANY OTHER FUNCTION			+
	+ M. Hoek, 31/01/2013						+
	\* ======================================= */
	Int_t nEvtCntDecErr		= 0; // event counter: passing event decoding error cut
	Int_t nEvtCntRndmBits	= 0; // event counter: passing TDC random bits cut
	Int_t nEvtCntEmptyTdc	= 0; // event counter: passing empty TDC cut
	Int_t nEvtCntRefMiss	= 0; // event counter: passing missing reference signal cut
	// define histograms
	TFile *AnalysisOut = new TFile(cUserAnalysisFilename.c_str(),"RECREATE");
	TH1D hEvtStats("hEvtStats","hEvtStats",15,-0.5,14.5);
	TH1D hRefClocks("hRefClocks","hRefClocks; no of reference channels per event; frequency",18,-0.5,17.5);
	TH1D hTdcHits("hTdcHits","hTdcHits;no of leading edge hits per event; frequency",51,-1.5,49.5);
	TH1D hPixelHits("hPixelHits","hPixelHits;no of complete hits per event; frequency",51,-1.5,49.5);
	TH1D hTdcEff("hTdcEff","hTdcEff",12,-1.5,10.5);
	TH2D hTdcHitMatching("hTdcHitMatching","hTdcHitMatching; no of leading edge hits per event; no of complete hits per event; frequency",100,-0.5,99.5,100,-0.5,99.5);
	TH1D hTdcHitChannels("hTdcHitChannels","hTdcHitChannels; unique TDC channel ID; frequency",nMaxTdcChannel,-0.5,nMaxTdcChannel-0.5);
	TH1D hTdcHitTiming("hTdcHitTiming","hTdcHitTiming; leading edge time stamp (corrected for reference clock) (ns); frequency",6000,HIT_TIME_MIN,HIT_TIME_MAX);
	TH1D hTdcHitTimingPeak("hTdcHitTimingPeak","hTdcHitTimingPeak",100,-400.0,-360.0);
	TH1D hTdcEventTiming("hTdcEventTiming","hTdcEventTiming; #DeltaT (ns); frequency",500,-1.0,20.0);
	TH2D hTdcEvtTimingChanDist("hTdcEvtTimingChanDist","hTdcEvtTimingChanDist; #DeltaT (ns); unique TDC channel ID; frequency",500,-1.0,20.0,nMaxTdcChannel,-0.5,nMaxTdcChannel-0.5);
	TH1D hHitWidth("hHitWidth","hHitWidth; hit width (ns); frequency",120,-10.0,50.0);
	TH2D hHitWidthVsChannel("hHitWidthVsChannel","hHitWidthVsChannel; unique TDC channel ID; hit width (ns)",nMaxTdcChannel,-0.5,nMaxTdcChannel-0.5,400,-10.0,50.0);
	TH2D hHitWidthVsTiming("hHitWidthVsTiming","hHitWidthVsTiming; leading edge time stamp (ns); hit width (ns); frequency",2000,-HIT_TIME_MIN,HIT_TIME_MAX,400,-10.0,50.0);
	TH2D hHitTimeVsChannel("hHitTimeVsChannel","hHitTimeVsChannel; unique TDC channel ID; leading edge time stamp (ns); frequency",nMaxTdcChannel,-0.5,nMaxTdcChannel-0.5,6000,HIT_TIME_MIN,HIT_TIME_MAX);
	TH1D hMultiHits("hMultiHits","hMultiHits; no of channels with multiple hits per event; frequency",25,-1.5,23.5);
	// begin with analysis
	for(Int_t i=0; i<nEventsMax; i++){ // begin loop over all events
	//for(Int_t i=4; i<5; i++){ // begin loop over all events
		GetEntry(i);

		if(TrbData->nSubEvtDecError!=0) { // check if there were any problems during conversion
			if(bVerboseMode)
				cout << "Skipped: " << TrbData->nSubEvtDecError << endl;
			continue; // skip rest of loop
		}
		if(bVerboseMode) {
			cout << "Analyzing event " << i << endl;
			PrintTdcHits();
			PrintRefTimestamps();
			PrintTdcLeadingEdges();
		}
		++nEvtCntDecErr; // increment counter
		if(!CheckRandomBits())
			continue;
		++nEvtCntRndmBits; // increment counter
		hRefClocks.Fill((Double_t)TdcRefTimes.size());
		hTdcHits.Fill((Double_t)TdcLeadingEdges.size());
		if(TdcLeadingEdges.empty()) // check if there are any hits in the TDC channels (excluding reference signals)
			continue; // skip rest of loop
		++nEvtCntEmptyTdc;
		hMultiHits.Fill((Double_t)nEvtMultHits);
		for(std::map< Int_t,Double_t >::const_iterator Hit=TdcLeadingEdges.begin(); Hit!=TdcLeadingEdges.end(); Hit++){
			hTdcHitChannels.Fill((Double_t)Hit->first);
			hTdcHitTiming.Fill(Hit->second);
			hTdcHitTimingPeak.Fill(Hit->second);
		}
		std::vector< std::pair< Double_t,Int_t > > fEvtTiming = ComputeEventTiming();
		if(fEvtTiming.size()>0){
			for(std::vector< std::pair< Double_t,Int_t > >::const_iterator it=fEvtTiming.begin(); it!=fEvtTiming.end(); ++it){
				hTdcEventTiming.Fill(it->first);
				hTdcEvtTimingChanDist.Fill(it->first,(Double_t)it->second);
			}
		}
		hPixelHits.Fill((Double_t)PixelHits.size());
		if(PixelHits.empty()) // check that we have matched hits and computed their width
			continue; // skip rest of loop
		if(bAllRefChanValid){ // check if all reference clock signals are available
			++nEvtCntRefMiss;
			hTdcHitMatching.Fill((Double_t)TdcLeadingEdges.size(),(Double_t)PixelHits.size());
			hTdcEff.Fill((Double_t)(TdcLeadingEdges.size()-PixelHits.size()));
		}
		for(std::map< Int_t,Double_t >::const_iterator Hit=TimeOverThreshold.begin(); Hit!=TimeOverThreshold.end(); Hit++){
			hHitWidth.Fill(Hit->second);
			hHitWidthVsChannel.Fill((Double_t)Hit->first,Hit->second);
			std::map< Int_t,Double_t >::const_iterator LeadingEdge = TdcLeadingEdges.find(Hit->first);
			if(LeadingEdge!=TdcLeadingEdges.end()){
				hHitWidthVsTiming.Fill(LeadingEdge->second,Hit->second);
				hHitTimeVsChannel.Fill((Double_t)LeadingEdge->first,LeadingEdge->second);
			}
		}
	} // end of loop over all events
	// fill general histograms
	hEvtStats.Fill(0.0,(Double_t)nEventsMax);
	hEvtStats.GetXaxis()->SetBinLabel(1,"no cuts");
	hEvtStats.Fill(1.0,(Double_t)nEvtCntDecErr);
	hEvtStats.GetXaxis()->SetBinLabel(2,"decoding error cut");
	hEvtStats.Fill(2.0,(Double_t)nEvtCntRndmBits);
	hEvtStats.GetXaxis()->SetBinLabel(3,"Random bits cut");
	hEvtStats.Fill(3.0,(Double_t)nEvtCntEmptyTdc);
	hEvtStats.GetXaxis()->SetBinLabel(4,"empty TDC cut");
	hEvtStats.Fill(4.0,(Double_t)nEvtCntRefMiss);
	hEvtStats.GetXaxis()->SetBinLabel(5,"missing reference signal cut");
	// write histograms to file
	hEvtStats.Write();
	hRefClocks.Write();
	hTdcHits.Write();
	hPixelHits.Write();
	hTdcEff.Write();
	hTdcHitMatching.Write();
	hTdcHitChannels.Write();
	hTdcHitTiming.Write();
	hTdcHitTimingPeak.Write();
	hTdcEventTiming.Write();
	hTdcEvtTimingChanDist.Write();
	hHitWidth.Write();
	hHitWidthVsChannel.Write();
	hHitWidthVsTiming.Write();
	hHitTimeVsChannel.Write();
	hMultiHits.Write();
	delete AnalysisOut; // close RooT file and delete pointer
}

void TTrbAnalysis::Analyse(string cUserAnalysisFilename, UInt_t nUserTrbAddress, UInt_t nUserTdcChannel){
	SetRefChannel(nUserTrbAddress,nUserTdcChannel);
	Analyse(cUserAnalysisFilename);
}

Bool_t TTrbAnalysis::CheckRandomBits(){
	// check that all TDC hits in an event have the same random bits sequence
	switch (TrbData->Hits_){
		case 0: // no TDC hits, shouldn't happen
			return (kFALSE);
			break;
		case 1: // only one TDC hit, is always true
			return (kTRUE);
			break;
		default: // more than 1 TDC hit
			UInt_t nRefRndmBits = TrbData->Hits_nSubEvtId[0]; // set reference sequence
			for(Int_t i=1; i<TrbData->Hits_; ++i){ // loop over all hits in entry
				if(TrbData->Hits_nSubEvtId[1]!=nRefRndmBits){ // check if current sequence doesn't equal reference sequence
					return (kFALSE);
				}
			} // end of loop over all hits in entry
			return (kTRUE); // all sequences are the same
	}
	return (kFALSE);
}

void TTrbAnalysis::ClearEventMaps(){
	PixelHits.clear();
	TdcRefTimes.clear();
	TdcHits.clear();
	TdcLeadingEdges.clear();
	TimeOverThreshold.clear();
	// reset event level variables
	bAllRefChanValid = kFALSE;
	nEvtMultHits = -1;
}

Int_t TTrbAnalysis::ComputeTdcChanId(UInt_t nTrbAddress, UInt_t nTdcChannel){
	// compute uinque channel index based on FPGA address and TDC channel number
	std::map<UInt_t,UInt_t>::const_iterator TrbIndex;
	TrbIndex = TrbAddresses.find(nTrbAddress);
	if(TrbIndex==TrbAddresses.end()) // check if TRB address is present in user list
		return (-1);
	//UInt_t nTdcIndex = nTrbAddress&0xF;
	//cout << nTrbAddress << " , " << TrbIndex->second << " , " << nTdcIndex << endl;
	Int_t nUniqueId = (nTdcChannel-TDC_CHAN_OFFSET) + TrbIndex->second * N_TDC_CHAN;
	return (nUniqueId);
}

std::vector< std::pair< Double_t,Int_t > > TTrbAnalysis::ComputeEventTiming(){
	// compute the timing difference between hits in the same event
	std::vector< std::pair< Double_t,Int_t > > fTimingDifference;
	if(TdcLeadingEdges.size()<2)
		return (fTimingDifference);
	std::map< Int_t,Double_t >::const_iterator itA;
	std::map< Int_t,Double_t >::const_iterator itB;
	for(itA=TdcLeadingEdges.begin(); itA!=TdcLeadingEdges.end(); ++itA){ // begin of loop A over all leading edge timestamps
		// only need to look at upper half of the symmetric matrix and drop the sign of the difference
		for(itB=itA; itB!=TdcLeadingEdges.end(); ++itB){ // begin of loop B over all leading edge timestamps
			if(itA==itB) // ignore same entries
				continue;
			Int_t nChannelDiff = abs(itA->first - itB->first);
			Double_t fTimeDiff = itA->second - itB->second; // compute timing difference
			fTimingDifference.push_back(make_pair(fTimeDiff,nChannelDiff)); // insert timing difference into vector
		} // end of loop B over all leading edge timestamps
	} // end of loop A over all leading edge timestamps
	return (fTimingDifference);
}

Bool_t TTrbAnalysis::ExcludeChannel(UInt_t nUserTrbAddress, UInt_t nUserTdcChannel){
	std::pair< UInt_t,UInt_t > TempPair (nUserTrbAddress, nUserTdcChannel); // create pair consisting of FPGA address and TDC channel
	std::vector< pair< UInt_t,UInt_t > >::iterator CheckPair; // iterator on vector containing excluded channel addresses
	CheckPair = find(ExcludedChannels.begin(),ExcludedChannels.end(),TempPair); // check if channel is already excluded
	if(CheckPair!=ExcludedChannels.end())
		return (kFALSE);
	ExcludedChannels.push_back(TempPair); // enter this channel into vector
	return(kTRUE);
}

UInt_t TTrbAnalysis::ExcludeChannels(string UserFilename){
	UInt_t nExcludedChannels = 0;
	ifstream UserInputFile(UserFilename.c_str(),ifstream::in);
	Int_t nLineIndex = 0; // input file line index
	while(UserInputFile.good()){ // start loop over input file
		string cCurrentLine;
		getline(UserInputFile,cCurrentLine); // get line from input file
		if(cCurrentLine.empty()) // skip empty lines
			continue;
		nLineIndex++; // increment line index
		string cBuffer;
		vector<string> tokens;
		stringstream cParsingLine(cCurrentLine);
		while(!cParsingLine.eof()){ // parse string containing line from input file
			cParsingLine >> cBuffer;
			if(!cBuffer.empty()) // if token is not empty add to list of tokens
				tokens.push_back(cBuffer);
			cBuffer.clear();
		}
		UInt_t nTempAddress = (UInt_t)strtol(tokens.at(0).c_str(),NULL,0); // decode FPGA address
		UInt_t nTempChannel = (UInt_t)strtol(tokens.at(1).c_str(),NULL,10); // decode TDC channel
		switch (tokens.size()) {
			case 2: // FPGA address first (hex) then TDC channel
				if(ExcludeChannel(nTempAddress,nTempChannel)) // try to enter channel into map containing excluded channels
					++nExcludedChannels; // increment counter of excluded channels
				break;
			default:
				continue; // do nothing
		}
	} // end loop over input file
	UserInputFile.close();
	return(nExcludedChannels);
}

void TTrbAnalysis::FillTdcHits(){
	TdcHits.clear(); // clear any previous TDC hits
	for(Int_t i=0; i<TrbData->Hits_; ++i){ // loop over all hits in entry
		if(TrbData->Hits_nTdcChannel[i]<TDC_CHAN_OFFSET) // check if reference channel
			continue; // skip rest of loop
		if(!TrbData->Hits_bIsCalibrated[i]) // channel fine time is not calibrated
			continue; // skip rest of loop
		std::pair< UInt_t,UInt_t > ChanAddress (TrbData->Hits_nTrbAddress[i],TrbData->Hits_nTdcChannel[i]); // create address pair consisting of FPGA address and TDC channel ID
		if(find(ExcludedChannels.begin(),ExcludedChannels.end(),ChanAddress)!=ExcludedChannels.end())
			continue;
		//cout << i << endl;
		Int_t nUniqueId = ComputeTdcChanId(TrbData->Hits_nTrbAddress[i],TrbData->Hits_nTdcChannel[i]);
		if(nUniqueId<0)
			continue;
		TdcHits.insert(make_pair((UInt_t)nUniqueId,i));
	}
}

void TTrbAnalysis::FillTdcLeadingEdge(){
	// leading edges are in the even-numbered TDC channels encoded
	// need to check for multiple hits in each channel and remove them
	// correct timestamps for reference signal offset
	TdcLeadingEdges.clear(); // clear leading edges map
	if(TdcHits.empty()) // no TDC hits available
		return;
	for(std::multimap< UInt_t,Int_t >::const_iterator CurrentTdcHit=TdcHits.begin(); CurrentTdcHit!=TdcHits.end(); CurrentTdcHit++){ // begin of loop over all TDC hits (needs to be a while loop!)
		Int_t nMultiplicity = (Int_t) TdcHits.count(CurrentTdcHit->first);
		if(nMultiplicity>1){ // check if multiple hits occur
			CurrentTdcHit = TdcHits.upper_bound(CurrentTdcHit->first); // increment iterator to skip multiple hits
			CurrentTdcHit--;
			continue; // skip rest of loop
		}
		if(((CurrentTdcHit->first-TDC_CHAN_OFFSET+TDC_SWAP_RISING_FALLING) % 2)!=0){ // channel number not even, skip this entry
			continue; // skip rest of loop
		}
		// adjust for reference time
		UInt_t nTdcAddress = TrbData->Hits_nTrbAddress[CurrentTdcHit->second];
		std::map< UInt_t,Int_t >::const_iterator Offset = TdcRefTimes.find(nTdcAddress);
		if(Offset==TdcRefTimes.end()) // couldn't find reference signal timestamp
			continue; // skip rest of loop
		if(TrbData->Hits_bIsCalibrated[CurrentTdcHit->second]){
			Double_t fLeadingEdge = TrbData->Hits_fTime[CurrentTdcHit->second] - TrbData->Hits_fTime[Offset->second];
			TdcLeadingEdges.insert(make_pair((Int_t)CurrentTdcHit->first,fLeadingEdge)); // fill entry into leading edge map
		}

	} // end of loop over all TDC hits
	// now correct leading edge time stamps for user reference channel
	if(bRefChanIsSet&&!TdcLeadingEdges.empty()){ // find time stamp of user reference channel
		std::map< Int_t,Double_t >::const_iterator RefTime = TdcLeadingEdges.find(nTdcRefChannel);
		if(RefTime==TdcLeadingEdges.end()){ // couldn't find user reference time
			TdcLeadingEdges.clear(); // clear leading edges map
			return;
		}
		for(std::map< Int_t, Double_t >::iterator ThisLeadingEdge=TdcLeadingEdges.begin(); ThisLeadingEdge!=TdcLeadingEdges.end(); ThisLeadingEdge++){ // begin loop over all leading edge timestamps
			ThisLeadingEdge->second -= RefTime->second;
		}
	}
}

void TTrbAnalysis::FillTimeOverThreshold(){
	TimeOverThreshold.clear();
	if(PixelHits.empty()) // matched pixel hits map is empty
		return;
	for(std::map< Int_t,Int_t >::const_iterator CurrentHit=PixelHits.begin(); CurrentHit!=PixelHits.end(); ++CurrentHit){ // begin of loop over matched hits
		Double_t fTempHitWidth = TrbData->Hits_fTime[CurrentHit->second] - TrbData->Hits_fTime[CurrentHit->first];
		// Undo the swapping if desired
		if(TDC_SWAP_RISING_FALLING==1)
			fTempHitWidth = -fTempHitWidth;
		if(bVerboseMode)
			cout << "Time from leading to trailing edge: " << fTempHitWidth << endl;
		Int_t nTempChanId = ComputeTdcChanId(TrbData->Hits_nTrbAddress[CurrentHit->first],TrbData->Hits_nTdcChannel[CurrentHit->first]);
		if(nTempChanId<0)
			continue;
		TimeOverThreshold.insert(make_pair(nTempChanId,fTempHitWidth));
	} // end of loop over matched hits
}

Int_t TTrbAnalysis::GetEntry(Long64_t nEntryIndex){
	// Get event entry from TTree object and perform basic analysis tasks
	ClearEventMaps(); // reset all event-level variables and maps
	Int_t nEntrySize = TrbData->GetEntry(nEntryIndex); // retrieve data from tree
	if(nEntrySize<1) // if entry is invalid return now
		return (nEntrySize);
	// if(bVerboseMode)
	// 	cout << "Getting entry " << nEntryIndex << "..."<< nEntrySize << endl;

	bAllRefChanValid = SetRefTimestamps(); // extract reference timestamps
	FillTdcHits(); // fill all TDC hits into multimap, reference channels are excluded
	if(!TdcHits.empty()){ // if there are any TDC hits, do basic analysis tasks
		FillTdcLeadingEdge(); // correct leading edge timestamps for reference time and fill into map
		nEvtMultHits = HitMatching(); // try matching leading and trailing edges and fill array indices into map, skipping multi-hit channels
		FillTimeOverThreshold(); // compute pulse lengths based on matched hits and fill into map
	}
	return (nEntrySize);
}

Int_t TTrbAnalysis::HitMatching(Bool_t bSkipMultiHits){
	// match leading and trailing edge timestamps
	// use TdcHits multimap as starting point
	Int_t nMultipleHits = 0;
	PixelHits.clear();
	if(TdcHits.empty()) // no TDC hits available
		return (nMultipleHits);
	std::multimap< UInt_t,Int_t >::const_iterator CurrentTdcHit=TdcHits.begin();
	while(CurrentTdcHit!=TdcHits.end()){ // begin of loop over all TDC hits (excluding reference channels & user exclude list)
		// TdcHits->first is the uniqueId of the channel
		// TdcHits->second index in the TrbData->Hits_ array
		Int_t nMultiplicity = (Int_t) TdcHits.count(CurrentTdcHit->first);
		if(bSkipMultiHits && nMultiplicity>1){ // check if multiple hits occur
			CurrentTdcHit = TdcHits.upper_bound(CurrentTdcHit->first); // increment iterator to skip multiple hits
			++nMultipleHits;
			continue; // skip rest of loop
		}
		if(((CurrentTdcHit->first) % 2)!=0){ // channel number not even, skip this entry (hit must start with an even-numbered channel
			++CurrentTdcHit;
			continue; // skip rest of loop
		}
		std::multimap< UInt_t,Int_t >::const_iterator TempTdcHit = CurrentTdcHit; // store pointer to leading-edge entry
		++CurrentTdcHit; // increment iterator to point to the next element
		if(CurrentTdcHit==TdcHits.end()) // check if we reached end of hit map
			break; // if end of hit map is reached, exit this loop
		if((CurrentTdcHit->first-TempTdcHit->first)==1){ // found hit sequence
			PixelHits.insert(make_pair(TempTdcHit->second,CurrentTdcHit->second)); // enter this combination into pixel hit map
			// fill Time-over-Threshold map here

		}
		++CurrentTdcHit; // increment iterator
	}
	if(bVerboseMode)
		cout << "Found " << PixelHits.size() << " hit matches." << endl;
	return (nMultipleHits);
}

void TTrbAnalysis::Init(){

	TrbData = NULL;
	// intialise setup specific variables
	nEventsMax		= -1; // number of events in data set
	nMaxTdcChannel	= 0; // unique index of last channel
	nTrbEndpoints	= 0; // number of TRB boards in setup
	bRefChanIsSet	= kFALSE; // no user reference channel is set
	// initialise event level variables
	bAllRefChanValid = kFALSE;
	nEvtMultHits	= 0; // number of channels with multiple hits
	// clear all containers
	PixelHits.clear();
	TrbAddresses.clear();
	TdcRefTimes.clear();
	TimeOverThreshold.clear();
	ExcludedChannels.clear();
}

Bool_t TTrbAnalysis::OpenTrbTree(string cUserDataFilename){
	if(cUserDataFilename.empty()){
		cerr << "TRB data filename is empty!" << endl;
		return (kFALSE);
	}
	TFile *TrbTreeFile = new TFile(cUserDataFilename.c_str());
	if(TrbTreeFile->IsZombie()){
		cerr << "Error opening TRB data file " << cUserDataFilename << endl;
		return (kFALSE);
	}
	TTree *TrbTree = (TTree*)TrbTreeFile->Get("T");
	TrbData = new TTrbDataTree(TrbTree);
	nEventsMax = (Int_t)TrbData->fChain->GetEntriesFast();
	if(bVerboseMode)
		cout << "TrbTree opened with " << nEventsMax << " entries" << endl;
	return (kTRUE);
}

void TTrbAnalysis::PrintExcludedChannels() const {
	cout << "++++++++++++++++++++++++++++++++++++++" << endl;
	cout << "+ TRBv3 Analysis - Excluded Channels +" << endl;
	cout << "++++++++++++++++++++++++++++++++++++++" << endl;
	for(std::vector< pair< UInt_t,UInt_t > >::const_iterator ChannelIndex=ExcludedChannels.begin(); ChannelIndex!=ExcludedChannels.end(); ChannelIndex++){
		cout << hex << ChannelIndex->first << dec << " " << ChannelIndex->second << endl;
	}
	cout << "++++++++++++++++++++++++++++++++++++++" << endl;
}

void TTrbAnalysis::PrintRefTimestamps() const {
	cout << "+++ TDC REFERENCE TIMESTAMPS +++" << endl;
	cout << "+++++++++++++++++++++++++++" << endl;
	cout << TdcRefTimes.size() << " REFERENCE TIMESTAMPS FOUND" << endl;
	cout << "+++++++++++++++++++++++++++" << endl;
	for(std::map< UInt_t,Int_t >::const_iterator CurIndex=TdcRefTimes.begin(); CurIndex!=TdcRefTimes.end(); CurIndex++){
		cout << hex << CurIndex->first << dec << " , " << TrbData->Hits_nCoarseTime[CurIndex->second] << endl;
	}
	cout << "+++++++++++++++++++++++++++" << endl;
}

void TTrbAnalysis::PrintTdcHits() const {
	cout << "++++++++++++++++++++" << endl;
	cout << "+++ TDC HIT LIST +++" << endl;
	cout << "++++++++++++++++++++" << endl;
	cout << TdcHits.size() << " TDC HITS FOUND" << endl;
	cout << "++++++++++++++++++++" << endl;
	for(std::multimap< UInt_t,Int_t >::const_iterator CurIndex=TdcHits.begin(); CurIndex!=TdcHits.end(); CurIndex++){
		if(TrbData->Hits_bIsCalibrated[CurIndex->second])
			cout << CurIndex->first << " , " << std::setprecision(10) << TrbData->Hits_fTime[CurIndex->second] << " ns" << endl;
		else
			cout << CurIndex->first << " , " << TrbData->Hits_nCoarseTime[CurIndex->second] << " , " << TrbData->Hits_nFineTime[CurIndex->second] << endl;
	}
	cout << "++++++++++++++++++++" << endl;
}

void TTrbAnalysis::PrintTdcLeadingEdges() const {
	cout << "++++++++++++++++++++" << endl;
	cout << "+++ TDC LEADING EDGE LIST +++" << endl;
	cout << "+++++++++++++++++++++++++++++" << endl;
	cout << TdcLeadingEdges.size() << " TDC LEADING EDGES FOUND" << endl;
	cout << "+++++++++++++++++++++++++++++" << endl;
	for(std::map< Int_t,Double_t >::const_iterator CurIndex=TdcLeadingEdges.begin(); CurIndex!=TdcLeadingEdges.end(); CurIndex++){
		cout << CurIndex->first << " , " << CurIndex->second << endl;
	}
	cout << "+++++++++++++++++++++++++++++" << endl;
}

void TTrbAnalysis::PrintTrbAddresses() const {
	cout << "+++ TRB BOARD ADDRESSES +++" << endl;
	for(std::map< UInt_t,UInt_t >::const_iterator CurIndex=TrbAddresses.begin(); CurIndex!=TrbAddresses.end(); CurIndex++){
		cout << hex << CurIndex->first << dec << " , " << CurIndex->second << endl;
	}
	cout << "+++++++++++++++++++++++++++" << endl;
}

void TTrbAnalysis::SetRefChannel(UInt_t nTrbAddress, UInt_t nTdcChannel){
	Int_t nTempRefChan = ComputeTdcChanId(nTrbAddress,nTdcChannel);
	if(nTempRefChan>-1){
		bRefChanIsSet = kTRUE;
		nTdcRefChannel = nTempRefChan;
	}
}

Bool_t TTrbAnalysis::SetRefTimestamps(){
	TdcRefTimes.clear();
	// loop over Hits_nTdcChannel array and extract time whenever we find the reference channel
	for(Int_t i=0; i<TrbData->Hits_; i++){ // loop over all hits in entry
		if(!TrbData->Hits_bIsRefChannel[i]) // check for TDC reference channels
			continue; // skip rest of loop
		TdcRefTimes.insert(make_pair(TrbData->Hits_nTrbAddress[i],i)); // fill TDC address and array index into map
	}
	if(TdcRefTimes.size()!=TrbAddresses.size())
		return (kFALSE);
	return (kTRUE);
}


Int_t TTrbAnalysis::SetTrbAddresses(string cUserTdcAddressesFile){
	// set TRB addresses, address delimeter is '|'
	if(cUserTdcAddressesFile.empty()){ // check if TRB address string is empty
		if(bVerboseMode)
			cout << "TRB address string is empty!" << endl;
		return 0;
	}
	ifstream UserInputFile(cUserTdcAddressesFile.c_str(),ifstream::in); // open text file containing TRB endpoint addresses
	if(!UserInputFile.is_open()){ // check if opening text file was successful
		cerr << "Could not open TRB addresses file " << cUserTdcAddressesFile << endl;
		return (0);
	}
	while(UserInputFile.good()){ // start loop over input file
		string cCurrentLine;
		getline(UserInputFile,cCurrentLine); // get line from input file
		if(cCurrentLine.empty()) // skip empty lines
			continue;
		vector<string> tokens = LineParser(cCurrentLine,' ',bVerboseMode);
		UInt_t nBoardIndex;
		switch (tokens.size()) {
			case 1: // we only expect one address per line
				//cTdcAddresses.push_back(tokens.at(0));
				nBoardIndex = (Int_t)distance(TrbAddresses.begin(),TrbAddresses.end());
				TrbAddresses.insert(make_pair(HexStringToInt(tokens.at(0)),nBoardIndex));
				break;
			default: // anything with more than one token per line will be ignored!
				continue; // do nothing
		}
	} // end loop over input file
	UserInputFile.close(); // close text file
	if(bVerboseMode){
		cout << TrbAddresses.size() << " TDC endpoint addresses decoded." << endl;
		PrintTrbAddresses();
	}
	nMaxTdcChannel = (Int_t)TrbAddresses.size() * N_TDC_CHAN;
	return (TrbAddresses.size());
}

void TTrbAnalysis::WriteTdcMapping(string cUserMappingFile) {
	if(cUserMappingFile.empty())
		return;
	ofstream MappingFileOut(cUserMappingFile.c_str()); // open text file for writing mapping scheme
	for(std::map<UInt_t,UInt_t>::const_iterator CurTrbBoard=TrbAddresses.begin(); CurTrbBoard!=TrbAddresses.end(); ++CurTrbBoard){ // begin loop over TRB endpoint addresses
		//UInt_t nTrbAddress = CurTrbBoard->first + (UInt_t)nTdcIndex;
		for(Int_t i=TDC_CHAN_OFFSET; i<(N_TDC_CHAN+TDC_CHAN_OFFSET); ++i){ // begin loop over TDC channels
			Int_t nTdcChanId = ComputeTdcChanId(CurTrbBoard->first,(UInt_t)i);
			MappingFileOut << hex << CurTrbBoard->first << dec << "\t" << i << "\t" << nTdcChanId << endl;
		} // end of loop over TDC channels
	} // end of loop over TRB board addresses
	MappingFileOut.close();
}
