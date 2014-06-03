void RunTreeConverter(string cUserDataFile, string cUserTdcAddresses, string cUserOutFile){
	gROOT->ProcessLine(".x BuildGsiTreeConverter.cpp");
	TGsiTreeConverter a(cUserDataFile,cUserTdcAddresses);
	a.KeepMultiHits();
	a.ConvertTree(cUserOutFile,0,1000);
}