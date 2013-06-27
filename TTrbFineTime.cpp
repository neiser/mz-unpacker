#include "TTrbFineTime.h"

ClassImp(TTrbFineTime);

TTrbFineTime::TTrbFineTime() :
	TObject(),
		bVerboseMode(kFALSE),
		bCalibrationIsValid(kFALSE),
		bTableIsComputed(kFALSE),
		nCalibrationType(0),
		nMinEntries(MIN_STATS),
		nTdcAddress(0x0),
		nTdcChannel(-1){ // standard constructor
	SetChannelName(kTRUE);
	Init();
}

TTrbFineTime::TTrbFineTime(const TTrbFineTime &a) : TObject(a) { // copy constructor
	Init();
	// this copy constructor is need for some STL containers and algorithms
	bVerboseMode		= a.bVerboseMode;
	bCalibrationIsValid = a.bCalibrationIsValid;
	bTableIsComputed	= a.bTableIsComputed;
	fClockCycle			= a.fClockCycle;
	nCalibrationType	= a.nCalibrationType;
	nMinEntries			= a.nMinEntries;
	nTdcAddress			= a.nTdcAddress;
	nTdcChannel			= a.nTdcChannel;
	cChannelName.str(a.cChannelName.str());
	hFineTimeDistribution	= a.hFineTimeDistribution;
	hBinWidth				= a.hBinWidth;
	grCalibrationTable		= a.grCalibrationTable;
	grFineTimeBinWidth		= a.grFineTimeBinWidth;
	CalibrationTable		= a.CalibrationTable;
	BinWidthTable			= a.BinWidthTable;
}

TTrbFineTime::~TTrbFineTime(){ // standard destructor

}

void TTrbFineTime::AnalyseHistogram(){
	nLowerEdgeBin = hFineTimeDistribution.FindFirstBinAbove(fBinThreshold);
	nUpperEdgeBin = hFineTimeDistribution.FindLastBinAbove(fBinThreshold);
}

void TTrbFineTime::CalibrationMode0(){ // compute calibration table using simple method
	if(!CalibrationTable.empty()) // check if calibration table map is empty
		CalibrationTable.clear(); // clear map contents
	Double_t fWidth = hFineTimeDistribution.GetBinCenter(nUpperEdgeBin) - hFineTimeDistribution.GetBinCenter(nLowerEdgeBin); // compute width of fine time distribution
	if(bVerboseMode)
		cout << "Checking Finetime Histogram Width: " << fWidth << " <= " << fMinWidth << "? ";
	if(fWidth<=fMinWidth){ // check if width is too narrow
		if(bVerboseMode)
			cout << "YES! Skipping." << endl;
		bCalibrationIsValid = kFALSE;
		bTableIsComputed	= kFALSE;
		return;
	}
	if(bVerboseMode)
		cout << "No, that's ok." << endl;
	Double_t fSlope = fClockCycle/fWidth; // compute slope based on clock cycle length and width of fine time distribution
	//cout << "TDC address " << hex << nTdcAddress << dec << " " << nTdcChannel << endl;
	//cout << fSlope << endl;
	for(Int_t nCurrentTdcBin=nLowerEdgeBin; nCurrentTdcBin<(nUpperEdgeBin+1); ++nCurrentTdcBin){ // begin of loop over all TDC fine time bins
		UInt_t nBinIndex = (UInt_t)hFineTimeDistribution.GetBinCenter(nCurrentTdcBin); // get central value of bin (should be bin index minus one, but you never know)
		BinWidthTable.insert(make_pair(nBinIndex,fSlope));
		hBinWidth.Fill(fSlope);
		Double_t fBinValue = (nCurrentTdcBin-nLowerEdgeBin) * fSlope; // compute timing value based on slope
		CalibrationTable.insert(make_pair(nBinIndex,fBinValue));
	} // end of loop over all TDC fine time bins
	bCalibrationIsValid = kTRUE;
	bTableIsComputed	= kTRUE;
	if(bVerboseMode)
		cout << cChannelName.str() << " - Calibration successful! " << CalibrationTable.size() << endl;
}

void TTrbFineTime::CalibrationMode1(){ // compute calibration table using advanced method
	if(!CalibrationTable.empty()) // check if calibration table map is empty
		CalibrationTable.clear(); // clear map contents
	// compute bin widths
	Double_t fTotalEntries	= hFineTimeDistribution.GetEntries();
	Double_t fBinValue		= 0.0;
	Double_t fBinWidthOld	= 0.0;
	//cout << "TDC address " << hex << nTdcAddress << dec << " " << nTdcChannel << endl;
	for(Int_t nCurrentTdcBin=nLowerEdgeBin; nCurrentTdcBin<(nUpperEdgeBin+1); ++nCurrentTdcBin){ // begin of loop over all TDC fine time bins
		Double_t fBinEntries = hFineTimeDistribution.GetBinContent(nCurrentTdcBin);
		if((Int_t)fBinEntries == 0) // skip empty bins
			continue;
		UInt_t nBinIndex = (UInt_t)hFineTimeDistribution.GetBinCenter(nCurrentTdcBin); // get central value of bin (should be bin index minus one, but you never know)
		Double_t fBinWidth = fBinEntries/fTotalEntries * fClockCycle;
		BinWidthTable.insert(make_pair(nBinIndex,fBinWidth));
		//cout << fBinWidth << endl;
		hBinWidth.Fill(fBinWidth);
		fBinValue += 0.5*fBinWidthOld + 0.5*fBinWidth;
		//cout << nBinIndex << " , " << fBinValue << endl;
		CalibrationTable.insert(make_pair(nBinIndex,fBinValue));
		fBinWidthOld = fBinWidth;
	} // end of loop over all TDC fine time bins
	bCalibrationIsValid = kTRUE;
	bTableIsComputed	= kTRUE;
	if(bVerboseMode)
		cout << cChannelName.str() << " - Calibration successful! " << CalibrationTable.size() << endl;
}

void TTrbFineTime::CalibrationMode2(){ // compute calibration table using static limits
	if(!CalibrationTable.empty()) // check if calibration table map is empty
		CalibrationTable.clear(); // clear map contents
	Double_t fWidth = (Double_t)STATIC_UPPER_LIMIT-STATIC_LOWER_LIMIT;
	Double_t fSlope = fClockCycle/fWidth; // compute slope based on clock cycle length and width of fine time distribution
	for(Int_t nCurrentTdcBin=STATIC_LOWER_LIMIT; nCurrentTdcBin<(STATIC_UPPER_LIMIT+1); ++nCurrentTdcBin){ // begin of loop over all TDC fine time bins
		BinWidthTable.insert(make_pair(nCurrentTdcBin,fSlope));
		hBinWidth.Fill(fSlope);
		Double_t fBinValue = (nCurrentTdcBin-STATIC_LOWER_LIMIT) * fSlope; // compute timing value based on slope
		CalibrationTable.insert(make_pair(nCurrentTdcBin,fBinValue));
	} // end of loop over all TDC fine time bins
	bCalibrationIsValid = kTRUE;
	bTableIsComputed	= kTRUE;
	if(bVerboseMode)
		cout << cChannelName.str() << " - Calibration successful! " << CalibrationTable.size() << endl;
}

void TTrbFineTime::ComputeCalibrationTable(){ // compute calibration constants
	AnalyseHistogram(); // extract basic quantities of fine time distribution from histogram
	if((Int_t)hFineTimeDistribution.GetEntries()<nMinEntries){ // check if enough entries in fine time distribution to attempt calibration
		if(bVerboseMode)
			cout << "Skipping, not enough entries in FineTime histogram: "
			     << hFineTimeDistribution.GetEntries() << " < " << nMinEntries  << endl;
		bCalibrationIsValid = kFALSE;
		bTableIsComputed	= kTRUE;
		return;
	}

	switch(nCalibrationType){
		case 0: // simple calibration based only on total width of fine time distribution
			CalibrationMode0();
			break;
		case 1: // advanced calibration taking fine time bin width variations into account
			CalibrationMode1();
			break;
		case 2: // static fine time limits (need to use this in case of degenrate fine time distributions)
			CalibrationMode2();
			break;
		default: // if get here a wrong calibration type has been specified
			bCalibrationIsValid = kFALSE;
			bTableIsComputed	= kFALSE;
	}
	FillCalibrationGraph();
	FillBinWidthGraph();
}

void TTrbFineTime::FillBinWidthGraph(){
	if(!bCalibrationIsValid) // if calibration is not valid, return w/o filling graph
		return;
	grFineTimeBinWidth.Set((Int_t)BinWidthTable.size());
	std::map< UInt_t,Double_t >::const_iterator BinWidthTableIndex;
	std::map< UInt_t,Double_t >::const_iterator BinWidthTableEnd = BinWidthTable.end();
	Int_t nIndex = 0;
	for(BinWidthTableIndex=BinWidthTable.begin(); BinWidthTableIndex!=BinWidthTableEnd; ++BinWidthTableIndex){
		grFineTimeBinWidth.SetPoint(nIndex,BinWidthTableIndex->first,BinWidthTableIndex->second);
		++nIndex;
	}
}

void TTrbFineTime::FillCalibrationGraph(){ // fill calibration table graph
	if(!bCalibrationIsValid) // if calibration is not valid, return w/o filling graph
		return;
	grCalibrationTable.Set((Int_t)CalibrationTable.size());
	std::map< UInt_t,Double_t >::const_iterator CalTableIndex;
	std::map< UInt_t,Double_t >::const_iterator CalTableEnd = CalibrationTable.end();
	Int_t nIndex = 0;
	for(CalTableIndex=CalibrationTable.begin(); CalTableIndex!=CalTableEnd; ++CalTableIndex){
		grCalibrationTable.SetPoint(nIndex,CalTableIndex->first,CalTableIndex->second);
		++nIndex;
	}
}

Double_t TTrbFineTime::GetCalibratedTime(UInt_t nUserFineTime) const{
	std::map< UInt_t,Double_t >::const_iterator Index = CalibrationTable.find(nUserFineTime);
	if(Index==CalibrationTable.end())
		return (-1.0);
	else
		return Index->second;
}

void TTrbFineTime::Init(){ // initialise object
	CalibrationTable.clear(); // clear calibration table;
	BinWidthTable.clear();
	InitHistogram(); // initialise fine time histogram
	fMinWidth = MIN_HISTOGRAM_WIDTH; // minimum fine time distribution width
	fClockCycle = CLOCK_CYCLE_LENGTH;
}

void TTrbFineTime::InitHistogram(){
	SetHistogramNames(); // set names of histograms (need to be unique to avoid warnings in ROOT)
	//hFineTimeDistribution.SetName(cChannelName.str().c_str());
	hFineTimeDistribution.SetBins(FINE_TIME_BINS,-0.5,FINE_TIME_BINS-0.5);
	hBinWidth.SetBins(300,-0.1,0.5); // |MH> for now set the range from 0 to 1ns
}

void TTrbFineTime::PrintSettings() const{
	cout << "+++ TRB Fine Time Calibration Settings +++" << endl;
	cout << "Calibration type:\t" << nCalibrationType << endl;
	cout << "Min entries:\t" << nMinEntries << endl;
	cout << "TDC Address:\t" << hex << nTdcAddress << dec << endl;
	cout << "TDC Channel:\t" << nTdcChannel << endl;
	cout << "Histogram name:\t" << cChannelName.str() << endl;
}

void TTrbFineTime::PrintStatus() const{
	cout << "+++ TRB Fine Time Calibration Status +++" << endl;
	cout << hex << nTdcAddress << dec << " - " << nTdcChannel << endl;
	cout << "Length of calibration table:\t" << CalibrationTable.size() << endl;
}

void TTrbFineTime::SetChannelAddress(std::pair< UInt_t, UInt_t > UserAddress){ // set TDC channel address
	nTdcAddress = (Int_t)UserAddress.first;
	nTdcChannel = (Int_t)UserAddress.second;
	SetChannelName(kFALSE);
	SetHistogramNames();
}

void TTrbFineTime::SetChannelAddress( UInt_t nUserTdcAddress, UInt_t nUserTdcChannel){ // set TDC channel address
	SetChannelAddress(make_pair(nUserTdcAddress,nUserTdcChannel));
}

void TTrbFineTime::SetChannelName(Bool_t bSetRandom){ // set channel name based on TDC address and TDC channel ID
	cChannelName.clear(); // clear any error flags
	cChannelName.str(""); // clear string content
	if(bSetRandom){ // if random channel is required
		cChannelName.str(RandomString(8));
	}
	else{ // if TDC address and channel ID are known
		cChannelName << hex << nTdcAddress << dec << "_Chan_" << nTdcChannel;
	}
	if(bVerboseMode)
		cout << cChannelName.str() << endl;
}

void TTrbFineTime::SetHistogramNames(){ // set histogram names
	stringstream cHistogramName; // holds name for the histograms
	// start with fine time histogram
	cHistogramName << "hTdc_"  << cChannelName.str();
	hFineTimeDistribution.SetName(cHistogramName.str().c_str());
	hFineTimeDistribution.SetTitle(";fine time bin;frequency");
	// now bin width distribution
	cHistogramName.clear();
	cHistogramName.str("");
	cHistogramName << "hBinWidth_"  << cChannelName.str();
	hBinWidth.SetName(cHistogramName.str().c_str());
	hBinWidth.SetTitle(";fine time bin width (ns);frequency");
	// now calibration table graph
	cHistogramName.clear();
	cHistogramName.str("");
	cHistogramName << "grCalibrationTable_" << cChannelName.str();
	grCalibrationTable.SetName(cHistogramName.str().c_str());
	grCalibrationTable.SetTitle(";raw fine time (bins);cal. fine time (ns)");
	// now fine time bin width graph
	cHistogramName.clear();
	cHistogramName.str("");
	cHistogramName << "grFineTimeBinWidth_" << cChannelName.str();
	grFineTimeBinWidth.SetName(cHistogramName.str().c_str());
	grFineTimeBinWidth.SetTitle(";fine time bin;bin width (ns)");
}

void TTrbFineTime::WriteHistograms() const{ // write histograms to current directory
	hFineTimeDistribution.Write();
	hBinWidth.Write();
	if(bCalibrationIsValid){
		grCalibrationTable.Write();
		grFineTimeBinWidth.Write();
	}
}
