void BuildDircAnalysisPostdoc(){
	gROOT->ProcessLine(".L TrbStructs.h+");
	cout << "Building myUtilities ..." << endl;
	gROOT->ProcessLine(".L myUtilities.cpp+");
	//if(!TClass::GetClass("TTrbHit")->IsLoaded()){
		cout << "Building TRB Hit class..." << endl;
		gROOT->ProcessLine(".L TTrbHit.cpp+");
	//}
	//if(!TClass::GetClass("THldSubEvent")->IsLoaded()){
		cout << "Building Subevent class ..." << endl;
		gROOT->ProcessLine(".L THldSubevent.cpp+");
	//}
	//if(!TClass::GetClass("THldEvent")->IsLoaded()){
		cout << "Building HLD Event class ..." << endl;
		gROOT->ProcessLine(".L THldEvent.cpp+");
	//}
	//if(!TClass::GetClass("TTrbEventData")->IsLoaded()){
		cout << "Building TRB Event Data class..." << endl;
		gROOT->ProcessLine(".L TTrbEventData.cpp+");
	//}
	//if(!TClass::GetClass("TTrbUnpacker")->IsLoaded()) {
		cout << "Building TRB Unpacker ..." << endl;
		gROOT->ProcessLine(".L TTrbUnpacker.cpp+");
	//}
	//if(!TClass::GetClass("TTrbDataTree")->IsLoaded()){
		cout << "Building TRB Data Tree ..." << endl;
		gROOT->ProcessLine(".L TTrbDataTree.C+");
	//}
	//if(!TClass::GetClass("TTrbAnalysisBase")->IsLoaded()){
		cout << "Building TRB AnalysisBase ..." << endl;
		gROOT->ProcessLine(".L TTrbAnalysisBase.cpp+");
	//}
	//if(!TClass::GetClass("TDircAnalysisBase")->IsLoaded()){
		cout << "Building DIRC Base Analysis ..." << endl;
		gROOT->ProcessLine(".L TDircAnalysisBase.cpp+");
	//}
	//if(!TClass::GetClass("TDircAnalysisPostdoc")->IsLoaded()){
		cout << "Building DIRC Analysis Postdoc..." << endl;
		gROOT->ProcessLine(".L TDircAnalysisPostdoc.cpp+");
	//}
}