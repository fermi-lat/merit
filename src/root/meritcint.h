/********************************************************************
* MeritCint.h
********************************************************************/
#ifdef __CINT__
#error MeritCint.h/C is only for compilation. Abort cint.
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#define G__ANSIHEADER
#define G__DICTIONARY
#include "G__ci.h"
extern "C" {
extern void G__cpp_setup_tagtableMeritCint();
extern void G__cpp_setup_inheritanceMeritCint();
extern void G__cpp_setup_typetableMeritCint();
extern void G__cpp_setup_memvarMeritCint();
extern void G__cpp_setup_globalMeritCint();
extern void G__cpp_setup_memfuncMeritCint();
extern void G__cpp_setup_funcMeritCint();
extern void G__set_cpp_environmentMeritCint();
}


#include "TROOT.h"
#include "TMemberInspector.h"
#include "analysis/Tuple.h"
#include "analysis/RebinHist.h"
#include "MultiPSF.h"
#include "PSFtailCuts.h"
#include "meritFoM.h"
#include "MeritPlots.h"

#ifndef G__MEMFUNCBODY
#endif

extern G__linked_taginfo G__MeritCintLN_bool;
extern G__linked_taginfo G__MeritCintLN_TClass;
extern G__linked_taginfo G__MeritCintLN_TObject;
extern G__linked_taginfo G__MeritCintLN_ostream;
extern G__linked_taginfo G__MeritCintLN_string;
extern G__linked_taginfo G__MeritCintLN_TupleItem;
extern G__linked_taginfo G__MeritCintLN_Tuple;
extern G__linked_taginfo G__MeritCintLN_vectorlETupleItemmUcOallocatorlETupleItemmUgRsPgR;
extern G__linked_taginfo G__MeritCintLN_vectorlETupleItemmUcOallocatorlETupleItemmUgRsPgRcLcLreverse_iterator;
extern G__linked_taginfo G__MeritCintLN__RanitlETupleItemmUcOlonggR;
extern G__linked_taginfo G__MeritCintLN_SampleStatistic;
extern G__linked_taginfo G__MeritCintLN_Histogram;
extern G__linked_taginfo G__MeritCintLN_vectorlEfloatcOallocatorlEfloatgRsPgR;
extern G__linked_taginfo G__MeritCintLN_vectorlEfloatcOallocatorlEfloatgRsPgRcLcLreverse_iterator;
extern G__linked_taginfo G__MeritCintLN__RanitlEfloatcOlonggR;
extern G__linked_taginfo G__MeritCintLN_RebinHist;
extern G__linked_taginfo G__MeritCintLN_Analyze;
extern G__linked_taginfo G__MeritCintLN_PSFanalysis;
extern G__linked_taginfo G__MeritCintLN_MultiPSF;
extern G__linked_taginfo G__MeritCintLN_vectorlEPSFanalysismUcOallocatorlEPSFanalysismUgRsPgR;
extern G__linked_taginfo G__MeritCintLN_vectorlEPSFanalysismUcOallocatorlEPSFanalysismUgRsPgRcLcLreverse_iterator;
extern G__linked_taginfo G__MeritCintLN__RanitlEPSFanalysismUcOlonggR;
extern G__linked_taginfo G__MeritCintLN_AnalysisList;
extern G__linked_taginfo G__MeritCintLN_vectorlEAnalyzemUcOallocatorlEAnalyzemUgRsPgR;
extern G__linked_taginfo G__MeritCintLN_vectorlEAnalyzemUcOallocatorlEAnalyzemUgRsPgRcLcLreverse_iterator;
extern G__linked_taginfo G__MeritCintLN__RanitlEAnalyzemUcOlonggR;
extern G__linked_taginfo G__MeritCintLN_PSFtailCuts;
extern G__linked_taginfo G__MeritCintLN_LayerGroup;
extern G__linked_taginfo G__MeritCintLN_FigureOfMerit;
extern G__linked_taginfo G__MeritCintLN_vectorlELayerGroupcOallocatorlELayerGroupgRsPgR;
extern G__linked_taginfo G__MeritCintLN_vectorlELayerGroupcOallocatorlELayerGroupgRsPgRcLcLreverse_iterator;
extern G__linked_taginfo G__MeritCintLN__RanitlELayerGroupcOlonggR;
extern G__linked_taginfo G__MeritCintLN_TCanvas;
extern G__linked_taginfo G__MeritCintLN_TPad;
extern G__linked_taginfo G__MeritCintLN_RootTuple;
extern G__linked_taginfo G__MeritCintLN_meritFoM;
extern G__linked_taginfo G__MeritCintLN_MeritPlots;

/* STUB derived class for protected member access */
