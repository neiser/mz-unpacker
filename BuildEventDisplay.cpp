void BuildEventDisplay(){
	gROOT->ProcessLine(".x BuildMapmtLibrary.cpp");
	gROOT->ProcessLine(".x BuildTrbUnpacker.cpp");
	gROOT->ProcessLine(".L TTrbDataTree.C+");
	gROOT->ProcessLine(".L TTrbAnalysis.cpp+");
	gROOT->ProcessLine(".L TTrbEventDisplay.cpp+");
}