/* standard C++ header files */
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <cfloat>
#include <climits>
#include <limits>
#include <stdexcept>
#include <string>
#include <ctime>
#include <algorithm>
#include <cmath>
// #include <exception>
/* RooT header files */
#include "TH2D.h"
#include "TH2I.h"

#include "MapmtPixel.cpp"

using namespace std;

//struct MAPMT_PIXEL;

class TPixelCluster : public TObject{
private:
	Double_t fCentroidX, fCentroidY; // centroid coordinates
	Double_t fOffsetX, fOffsetY; // centroid offset from leading pixel
	Double_t fSignalSum; // total signal in cluster
	Double_t fSignalRatio; // ratio of signal in leading pixel and total cluster
	Int_t nSearchPattern;
	vector<MAPMT_PIXEL> ClusterMembers; // vector containing the pixels belonging to this cluster
	void ComputeCentroid();
	void ComputeClusterSum();
	Bool_t IsNeighbour(const MAPMT_PIXEL& TestPixel) const;
	void Update();
public:
	TPixelCluster(Int_t nUserSearchPattern=0); // constructor, using only next neighbour search pattern
	~TPixelCluster(); // destructor
	Bool_t AddPixel(MAPMT_PIXEL UserPixel);
	Double_t DistanceToCluster(Double_t fUserX, Double_t fUserY) const;
	MAPMT_PIXEL GetLeadingPixel() const { return (ClusterMembers.back()); } // return pixel with largest signal in cluster
	Int_t GetN() const { return (ClusterMembers.size()); }
	vector<MAPMT_PIXEL> GetPixels() { return(ClusterMembers); }
	Double_t GetSignalSum() const { return (fSignalSum); }
	Double_t GetSignalRatio() const { return (fSignalRatio); }
	Bool_t IsMember(const MAPMT_PIXEL &UserPixel);
	bool operator < (const TPixelCluster& b) const { return (fSignalSum < b.fSignalSum); }
	friend ostream& operator << (ostream &s, const TPixelCluster &a) { // overloaded output stream operator for printing TPixelCluster to screen
		s << a.fCentroidX << "\t" << a.fCentroidY << "\t" << a.fSignalSum << "\t" << a.ClusterMembers.size();
		return s;
	}
	void Print() const;
	Double_t X() const { return (fCentroidX); }
	Double_t XOff() const { return (fOffsetX); }
	Double_t Y() const { return (fCentroidY); }
	Double_t YOff() const { return (fOffsetY); }
	/* some magic ROOT stuff... */
	ClassDef(TPixelCluster,1);
};

ClassImp(TPixelCluster);

TPixelCluster::TPixelCluster(Int_t nUserSearchPattern) : TObject(){
	fCentroidX	= -1.0; 
	fCentroidY	= -1.0;
	fOffsetX	= 0.0;
	fOffsetY	= 0.0;
	fSignalSum	= 0.0;
	fSignalRatio = -1.0;
	nSearchPattern = nUserSearchPattern;
}

TPixelCluster::~TPixelCluster(){

}

Bool_t TPixelCluster::AddPixel(MAPMT_PIXEL UserPixel){
	if(UserPixel.nPixelId<1)
		return (kFALSE);
	if(find(ClusterMembers.begin(),ClusterMembers.end(),UserPixel)!=ClusterMembers.end())
		return (kFALSE);
	if(!IsNeighbour(UserPixel))
		return (kFALSE);
	ClusterMembers.push_back(UserPixel);
	Update();
	return (kTRUE);
}

void TPixelCluster::ComputeCentroid(){ 
	Double_t fSumX = 0;
	Double_t fSumY = 0;
	for(vector<MAPMT_PIXEL>::const_iterator CurrentPixel = ClusterMembers.begin(); CurrentPixel != ClusterMembers.end(); CurrentPixel++){ // loop over all member of cluster
	// first the x-axis
		fSumX += CurrentPixel->fX * CurrentPixel->fAmplitude;
	// now the y-axis
		fSumY += CurrentPixel->fY * CurrentPixel->fAmplitude;
	} // end o f loop over all cluster members
	fCentroidX = fSumX/fSignalSum;
	fCentroidY = fSumY/fSignalSum;
	fOffsetX = fCentroidX - ClusterMembers.back().fX;
	fOffsetY = fCentroidY - ClusterMembers.back().fY;
}

void TPixelCluster::ComputeClusterSum(){
	fSignalSum = 0.0;
	for(vector<MAPMT_PIXEL>::const_iterator CurrentPixel = ClusterMembers.begin(); CurrentPixel != ClusterMembers.end(); CurrentPixel++){ // loop over all member of cluster
		fSignalSum += CurrentPixel->fAmplitude;
	}
	fSignalRatio = ClusterMembers.back().fAmplitude / fSignalSum;
}


Double_t TPixelCluster::DistanceToCluster(Double_t fUserX, Double_t fUserY) const {
	Double_t fDistance = pow(fCentroidX-fUserX,2) + pow(fCentroidY-fUserY,2);
	return (sqrt(fDistance));
}

Bool_t TPixelCluster::IsMember(const MAPMT_PIXEL &UserPixel){
	if(find(ClusterMembers.begin(),ClusterMembers.end(),UserPixel)==ClusterMembers.end())
		return (kFALSE);
	return (kTRUE);
}

Bool_t TPixelCluster::IsNeighbour(const MAPMT_PIXEL& TestPixel) const {
	if(ClusterMembers.size()<1) // cluster is empty, this is the seed pixel
		return (kTRUE);
	for(vector<MAPMT_PIXEL>::const_iterator CurrentPixel = ClusterMembers.begin(); CurrentPixel != ClusterMembers.end(); CurrentPixel++){
		Int_t nDeltaCol = abs(CurrentPixel->nPixelCol - TestPixel.nPixelCol);
		Int_t nDeltaRow = abs(CurrentPixel->nPixelRow - TestPixel.nPixelRow);
		switch (nSearchPattern) {
			case 0:
				if( (nDeltaRow==1&&nDeltaCol==0) || (nDeltaRow==0&&nDeltaCol==1))
					return (kTRUE);
				break;
			case 1:
				if(nDeltaCol<2 && nDeltaRow<2)
					return (kTRUE);
				break;
		}
	}
	return (kFALSE);
}

void TPixelCluster::Print() const {
	for(vector<MAPMT_PIXEL>::const_iterator CurrentPixel = ClusterMembers.begin(); CurrentPixel != ClusterMembers.end(); CurrentPixel++){
		cout << *CurrentPixel << endl;
	}
}

void TPixelCluster::Update(){ // compute updates for cluster properties
	sort(ClusterMembers.begin(),ClusterMembers.end(),ComparePixelsByEnergy);
	ComputeClusterSum();
	ComputeCentroid();
}