void Test(string cAnyString){
	cout << cAnyString << endl;
	gROOT->ProcessLine(".L FlashAnalysisExample.cpp");
	FlashAnalysisOverview(cAnyString,"TDC_FLASH.txt");
}