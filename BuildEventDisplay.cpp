void BuildEventDisplay(){
	gROOT->ProcessLine(".x BuildMapmtLibrary.cpp");
	gROOT->ProcessLine(".x BuildTrbUnpacker.cpp");
	gROOT->ProcessLine(".L TTrbDataTree.C+");
	gROOT->ProcessLine(".L TTrbAnalysis.cpp+");
	cout << "Building TRB AnalysisBase ..." << endl;
	gROOT->ProcessLine(".L TTrbAnalysisBase.cpp+");
	cout << "Building TRB Event Display ..." << endl;
	gROOT->ProcessLine(".L TTrbEventDisplay.cpp+");
}