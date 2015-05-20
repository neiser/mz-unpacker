void RunTreeConverter(string cUserDataFile, string cUserTdcAddresses, string cUserOutFile, Bool_t bIsDoubleEdge=kFALSE){
	gROOT->ProcessLine(".x BuildGsiTreeConverter.cpp");
	TGsiTreeConverter a(cUserDataFile,cUserTdcAddresses);
	a.KeepMultiHits();
	if(bIsDoubleEdge)
		a.SetIsDoubleEdge();
	//a.SetTimingWindow(-300.0,-240.0);
	a.ConvertTree(cUserOutFile,1,a.GetNEvents()-1);
}

void UnpackAndCalibrate(string cUserDataFile, string cUserSubIdList, string cUserHubList, string cUserTdcList){
	UInt_t nCTSAddress	= 0x7999;
	UInt_t nRefChanId	= 0;
	gROOT->ProcessLine(".x BuildTrbUnpacker.cpp");
	gROOT->ProcessLine(".x BuildTrbCalibration.cpp");
	// Unpack HLD file
	TTrbUnpacker RawData(cUserDataFile,cUserSubIdList,nCTSAddress,cUserHubList,cUserTdcList,nRefChanId);
	RawData.Decode(RawData.GetHldEntries()-1,1); // skip first entry (always empty)
	// Calibrate TDCs
	UInt_t nCalibType = 1; // calibration types: 0->semi-static; 1->dynamics (best); 2->static (for low statistics)
	string cRawDataFile = cUserDataFile + ".root";
	TTrbCalibration TdcCalib(cRawDataFile,nCalibType);
	TdcCalib.DoTdcCalibration();
}