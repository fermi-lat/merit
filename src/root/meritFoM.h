// 
// This serves as the interface between root and FigureOfMerit
//

#ifndef MERITFOM_H
#define MERITFOM_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FigureOfMerit.h"
#include "app/RootTuple.h"
#include "FigureOfMerit.h"
#include "TObject.h"
#include "TCanvas.h"
#include "TPad.h"
#include <iostream>
#include <fstream>

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

    int           numTupleEvents() {return m_tuple->numEvents();}
    // return number of events in the ntuple

    bool          cutEvent(int idx);
    // Does event idx pass the merit selection cuts?

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

    ClassDef(meritFoM, 1)
};
#endif
