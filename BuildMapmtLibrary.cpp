void BuildMapmtLibrary(){
	gROOT->ProcessLine(".L MapmtPixel.cpp+");
	gROOT->ProcessLine(".L TPixelCluster.cpp+");
	gROOT->ProcessLine(".L TMapmt.cpp+");
}