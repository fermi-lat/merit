#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

//start by defining things in analysis package we need

#pragma link C++ class SampleStatistic;
#pragma link C++ class Histogram;
#pragma link C++ class RebinHist;
#pragma link C++ class TupleItem;
#pragma link C++ class Tuple;

//Now the things in the merit package we want

#pragma link C++ class Analyze;
#pragma link C++ class AnalysisList;
#pragma link C++ class PSFtailCuts;
#pragma link C++ class PSFanalysis;
#pragma link C++ class MultiPSF;
#pragma link C++ class FigureOfMerit;
#pragma link C++ class meritFoM;
#pragma link C++ class MeritPlots;

#endif