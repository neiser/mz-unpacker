How to use the GSI Tree Converter Package
M. Hoek, 22/05/2014

The GSI Tree Converter package is intended to produce a non-sparsified tree structure withh limited data to enable fast online analyses.

The package is called TGsiTreeConverter and is derived from the class TDircAnalysisBase which, in turn, is derived from TTrbAnalysisBase. These classes provide basic functionality like synchronising the recorded time stamps or matching leading and trailing edges.

The TGsiTreeConverter package consists of the following files:
TGsiTreeConverter.h -> header file defining variables and methods
TGsiTreeConverter.cpp -> implementation of methods
BuildGsiTreeConverter.cpp -> compile script for ROOT console
RunTreeConverter.cpp -> simple example how to run converter package

Use example macro:

root[0] .L RunTreeConverter.cpp
root[1] RunTreeConverter([Data_File],[TDC_Address_File],[Output_File])

You need to supply the correct names for the first two arguments and choose an appropriate value for the last one. I have created an example for the TDC address file called "Analysis_Test_TDC_List_GSI.txt" which should work for the first test experiment at GSI in May 2014.
The TDC address file contains 3 columns corresponding to TDC address (no hex prefix!), number of channels on TDC (standard is 64 but you can put also smaller numbers there), and channel offset (i.e. how many channels are taken up by the reference signal. This has no impact on the number of TDC channels!).
 
