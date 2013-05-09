+++ README for Mainz Unpacker ROOT Class +++
+ V0.1, 12/11/2012
+ M. Hoek, hoek@uni-mainz.de
+

+ This unpacker is intended for TRBv3 TDC HLD file decoding

++++++++++++++++++++++
+ TRBv3 HLD Unpacker +
++++++++++++++++++++++
+ Building libraries
root[0] .x BuildTrbUnpacker.cpp

+ Run Unpacker
root[1] TTrbUnpacker a(HLD_filename, 0x8c00, 0x0002, "HUB_Addresses_test.txt", "TDC_Addresses_test.txt", 1, kTRUE)
# first hex number: subevent id
# second hex number: TRB address of trigger control system
# also provide a more or less complete list of endpoint addresses of TDCs in TDC_Addresses_test.txt
# and specify the HUB TRB addresses if your readout topology has some (provide empty string "" if you have no hubs)
# note: the last kTRUE in the function call switches the verbose mode on

root[2] a.Decode(N)
# this should print the decoded data on the screen;
# at the moment the Unpacker will only look at the first N events;
# Provide N=0 to unpack all (remaining) events in the file
# An offset (=skip specified number of events) can be provided as a second argument
# Unpacker also creates a very basic RooT Tree now;
# please switch off verbose mode when decoding many events (otherwise it takes ages)



+++++++++++++++++++++++++
+ TRBv3 TDC Calibration +
+++++++++++++++++++++++++
+ Building libraries
root[0] .x BuildTrbCalibration.cpp

+ Run Calibration
root[1] TTrbCalibration a(RooT_filename,Calibration_type=0,Min_Statistics=10000,VerboseMode=kFALSE)
# so far, two calibration methods have been implemented (more to follow):
# 0 -> simple calibration using only the total width of the fine time distribution
# 1 -> sophisticated calibration computing an individual bin width based on fine time bin entries
root[2] a.DoTdcCalibration()
This produces two RooT files: one with calibration graphs and another with the calibrated TRB data;

+++++++++++++++++++++++
+ TRBv3 Data Analysis +
+++++++++++++++++++++++
+ Building libraries
root[0] .x BuildTrbAnalysis.cpp
root[1] TTrbAnalysis a(RooT_filename,"TDC_Addresses_test.txt",VerboseMode=kFALSE)
root[2] a.Analyse(Output_filename)
