// This macro for executing merit in the root command window
// Assumes the merit library has been loaded!!
// 
// Tracy Usher (Dec 7, 2001) -- Pearl Harbor Day 60th 

#include <iostream>
#include <vector>
#include "math.h"

#include "meritFoM.h"

#include "TROOT.h"
#include "TProfile.h"
#include "TCanvas.h"
#include "TPad.h"

class MeritPlots
{
public:
    MeritPlots(const char* tupleFile, const char* cutString);
   ~MeritPlots();
    void run();
    void report();
    void plotPSFvals(int cosBin = 0);
    void plotPSFvsE(TPad* pPad=0);
    void surfPSFvsE();
    void plotPSFrat(TPad* pPad=0);
    meritFoM* getMeritFoM() {return merit;}

    AnalysisList* getList()         {return merit->getAnalysisList();}
    int           getListSize()     {return merit->getAnalysisListSize();}
    Analyze*      getListObj(int i) {return merit->getAnalysisListItem(i);}

private:
    void newCanvas(int nx, int ny);
    void drawPSFplot(PSFanalysis* pPsf, TPad* pPad);

    meritFoM* merit;
    TCanvas*  pCan;
    TPad**    pads;

    int nPads;
    int numPadsX;
    int numPadsY;
};
