// 
// This serves as the interface between root and FigureOfMerit
//

#ifndef MERITFOM_H
#define MERITFOM_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FigureOfMerit.h"
#include "TObject.h"
#include "TCanvas.h"
#include "TPad.h"
#include <iostream>
#include <fstream>
//#include <sys/types.h>
//#include <sys/timeb.h>

class FigureOfMerit;
class RootTuple;

//====================================================================================
class meritFoM : public TObject
{
public:
    meritFoM(const char* fileName, const char* cutstring);
   ~meritFoM();

    void execute();
    // analyze the current tuple row

    void report();
    // write results, return acceptacnce

    int           getAnalysisListSize()      {return m_FoM->getAnalysisList()->size();}
    Analyze*      getAnalysisListItem(int i);
    AnalysisList* getAnalysisList()          {return m_FoM->getAnalysisList();}
private:
    //Some private functions
    void drawMerit();
    void drawRunning();
    void drawFinished();

    //Data members
    FigureOfMerit* m_FoM;
    RootTuple*     m_tuple;
    TCanvas*       m_canvas;
    TPad*          m_pad;

    std::ostream*  outstream;
//    timeb          t_init;
//    timeb          t_final;
    ClassDef(meritFoM, 1)
};
#endif
