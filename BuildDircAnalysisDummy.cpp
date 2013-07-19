void BuildDircAnalysisDummy(){
	gROOT->ProcessLine(".L TrbStructs.h+");
	cout << "Building myUtilities ..." << endl;
	gROOT->ProcessLine(".L myUtilities.cpp+");
	cout << "Building TRB Hit class..." << endl;
	gROOT->ProcessLine(".L TTrbHit.cpp+");
	cout << "Building Subevent class ..." << endl;
	gROOT->ProcessLine(".L THldSubevent.cpp+");
	cout << "Building HLD Event class ..." << endl;
	gROOT->ProcessLine(".L THldEvent.cpp+");
	cout << "Building TRB Event Data class..." << endl;
	gROOT->ProcessLine(".L TTrbEventData.cpp+");
	if (!TClass::GetDict("TTrbUnpacker")) {
		cout << "Building TRB Unpacker ..." << endl;
		gROOT->ProcessLine(".L TTrbUnpacker.cpp+");
	}
	cout << "Building TRB Data Tree ..." << endl;
	gROOT->ProcessLine(".L TTrbDataTree.C+");
	cout << "Building TRB AnalysisBase ..." << endl;
	gROOT->ProcessLine(".L TTrbAnalysisBase.cpp+");
	cout << "Building DIRC Base Analysis ..." << endl;
	gROOT->ProcessLine(".L TDircAnalysisBase.cpp+");
	cout << "Building DIRC Analysis Dummy..." << endl;
	gROOT->ProcessLine(".L TDircAnalysisDummy.cpp+");
}