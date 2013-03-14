#ifndef _MAPMT_PIXEL_H_
#define _MAPMT_PIXEL_H_

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

using namespace std;

struct MAPMT_PIXEL{
	Int_t nPixelId; // unique pixel ID
	Int_t nQdcChannelId; // unique QDC channel number
	Double_t fAmplitude; // signal amplitude 
	Double_t fGain; // gain factor
	Double_t fThreshold; // signal threshold
	Int_t nPixelRow; // pixel's row index
	Int_t nPixelCol; // pixel's column index
	Double_t fX; // pixel's absolute horizontal position
	Double_t fY; // pixel's absolute vertical position
	MAPMT_PIXEL(Int_t a=-1, Int_t b=-1, Double_t c=-1.0, Double_t d=1.0, Double_t e=0.0, Int_t f=-1, Int_t g=-1, Double_t h=0.0, Double_t i=0.0) : nPixelId(a), nQdcChannelId(b), fAmplitude(c), fGain(d), fThreshold(e), nPixelRow(f), nPixelCol(g), fX(h), fY(i) {}
	bool operator < (const MAPMT_PIXEL &b) const { // find smaller of two MAPMT_PIXEL structs based on the pixel ID
		return nPixelId < b.nPixelId;
	}
	bool operator == (const MAPMT_PIXEL &b) const { // compare two MAPMT_PIXEL structs based on the pixel ID
		return nPixelId == b.nPixelId;
	}
	friend ostream& operator << (ostream &s, const MAPMT_PIXEL &a) { // overloaded output stream operator for printing MAPMT_PIXEL struct to screen
		s << a.nPixelId << "\t" << a.nQdcChannelId << "\t" << a.fGain << "\t" << a.fAmplitude << "\t" << a.fThreshold << "\t" << a.nPixelRow << "\t" << a.nPixelCol << "\t" << a.fX << "\t" << a.fY;
		return s;
	}
	bool operator ()(const MAPMT_PIXEL &b) const { 
		return ( (nPixelId==b.nPixelId) || (nQdcChannelId==b.nQdcChannelId) );
	}
};

bool ComparePixelsByEnergy(const MAPMT_PIXEL &a, const MAPMT_PIXEL &b){ return (a.fAmplitude < b.fAmplitude); }
bool ComparePixelsByGain(const MAPMT_PIXEL &a, const MAPMT_PIXEL &b){ return (a.fGain < b.fGain); }
bool PixelIsAboveThreshold(const MAPMT_PIXEL &a, Double_t fUserThreshold){ return (a.fAmplitude > fUserThreshold);}
Double_t DistanceToPixel(const MAPMT_PIXEL &a, const MAPMT_PIXEL &b) { 
	Double_t fDistance = sqrt(pow(a.fX-b.fX,2)+pow(a.fY-b.fY,2));
	return (fDistance);
}
Double_t DistanceToPixel(const MAPMT_PIXEL &UserPixel, const Double_t fUserX, const Double_t fUserY) {
	Double_t fDistance = sqrt(pow(UserPixel.fX-fUserX,2) + pow(UserPixel.fY-fUserY,2));
	return (fDistance);
}
void ResetPixelAmplitude(MAPMT_PIXEL &a) {a.fAmplitude = 0.0; }
void ResetPixelGain(MAPMT_PIXEL &a) { a.fGain = 1.0; }
void ResetPixelThreshold(MAPMT_PIXEL &a) { a.fThreshold = 0.0; }

#endif