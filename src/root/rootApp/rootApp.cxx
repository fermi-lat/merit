// $Header: /nfs/slac/g/glast/ground/cvs/merit/src/app/meritapp.cxx,v 1.7 2001/10/22 18:34:32 burnett Exp $

// Main for merit

#include <iostream>
#include <fstream>
#include <string>
#include <strstream>

#include <assert.h>

#include "TROOT.h"
#include "TApplication.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TPaveLabel.h"

#include "TFolder.h"
#include "TFrame.h"
#include "TCollection.h"
//#include "TIter.h"
#include "TPad.h"

#include "MeritPlots.h"

void FATAL(const char* s){std::cerr << "\nERROR: "<< s;}
void WARNING(const char* s){std::cerr << "\nWARNING: " << s;}

const char* _MERIT_VERSION = "$Revision: 1.7 $";
static std::string  cutstr("nA");
static std::string  file_name("");

static void drawPlots(TApplication* pTheApp);
static void plotPSFvsE();

static const char* helptext=
"\n----------------------------------------------"
"\nCalling syntax: merit  -CUTS [p] [file1 | - ] [file2] ..."
"\n\nAnalysis cuts: allow re-ordering of gamma rejection cuts and trigger"
"\ncounting at runtime. Order of cuts is same as in this string, specific"
"\n'cuts' will increment the counters merit uses for analysis of the tuple."
"\nIf 'p' is specified  then percentages will be displayed as well."
"\n Reads from stdin next parameter is \"-\", or from $(MERIT_INPUT_FILE) otherwise."
"\n\n\tCut keys are as follows:"
"\n\t\tGnnnn, : Set number generated to nnnnn, overriding gen(nnn) in tuple title"
"\n\t\t1 : level 1 trigger: Track or LoCal or HiCal "
"\n\t\tV : level 1 VETO throttle "
"\n\t\t2 : level 2 trigger: track, no doca veto except if cal "
"\n\t\t3 : level 3 trigger"
"\n\t\tF : TKR FRONT Section only"
"\n\t\tB : TKR BACK Section only"
"\n\t\tn : number of tracks (N_tracks > 0)"
"\n\t\tc : cosmic rejection cuts (old set)"
"\n\t\tj : Jose style tail suppresion cuts"
"\n\t    \"(...)\" : cut expression, like Chisq<50 ( chars ()<>  must be enclosed in quotes)"
"\n\t\tX : Xname, -- statistics on name"
"\n\t\tA : Analysis of PSF, etc."                                                    \
"\n\t\tW : write the (ascii) event to standard output (useful for filtering tuples!)"
"\n\t\tL : elapsed time: last time found in variable 'Triage_Time'"
"\n\t\tR : Rate: number of events/elapsed time"
"\n\t\ts : size: report on event size (assuming various bits/hit)"
"\n\t\tMx: Multi-PSF for bin x, x=0,1,2,3,4: do PSF analysis for 6 dE/E bins from "
"\n\t\t    31 MeV to 3.1 GeV, cos theta #n, bins are 0.2 wide"
"\n\t\t    x=a: all bins from 1 to 0.2, 0.2 bins"
"\n\t\t      b: all bins from 1 to 0.2, 0.1 bins"
"\n\t\t      n: one bin  from 1 to 0, appropriate for normal incidence, or average"
"\n\t\t      i: all costh bins but one energy bin, appropriate for single energy"
 ;

void	help()
{
    std::cerr << "\nmerit Version " << _MERIT_VERSION << " -- Help"
	   << helptext << "\n\n\tDefault: " << cutstr << std::endl;
}

int main(int argc, char* argv[])
{
    int		n=0;

    if( argc>++n ) file_name = argv[n];		// required
    if ( std::string(file_name).find("?") != std::string::npos
        || file_name=="--help") {	// print help information
	help();
	return 0;
    }
    // first arg is option string if preceded by dash
    if( file_name[0]=='-' && file_name.size()>1) {
        cutstr = file_name.substr(1);
        file_name = "";
    }
    if( argc>++n ) {			// optional
        if ( argv[n][0] == 'p' )  {
//            Analyze::showperc(true);
            n++;
        }

        if ( argc>n ) {
            if ( argv[n][0] == '-' )  {	// this is the cut sequence
                cutstr = &argv[n][1];
            } else file_name = argv[n];	
        }
    }

    //Create a root application
    argc = 1;
    TApplication* pTheApp = new TApplication("App", &argc, argv);
    gROOT->Reset("a");


    //Create the merit analysis object
    MeritPlots plots(file_name.data(), cutstr.data());

    //Execute the analysis object
    plots.run();

    //Make some plots... 
    plots.plotPSFvsE();
    pTheApp->Run(kTRUE);

    plots.plotPSFvals();
    pTheApp->Run(kTRUE);

    plots.plotPSFrat();
    pTheApp->Run(kTRUE);

    plots.report();

    delete pTheApp;

    return 0;
}

/*
void drawPlots(TApplication* pTheApp)
{
    //Open a canvas for the output
    TCanvas *pCan = new TCanvas("Graphs", "Merit Output", 400, 600);

    ((TFrame*)pCan->GetFrame())->SetFillColor(46);
    pCan->SetFillColor(19);
    pCan->SetBorderMode(28);

    //Divide into pads...
    int    numPadsX = 2;
    int    numPadsY = 3;
    int    nPads    = numPadsX * numPadsY;

    float  xDiv     = 0.95/numPadsX;
    float  yDiv     = 0.94/numPadsY;

    int    idx      = 0;
    float  yHigh    = (float)0.95;
    float  yLow     = (float)0.95 - yDiv;

    TPad** pads = new TPad*[nPads];

    for(int idxy=0; idxy<numPadsY; idxy++)
    {
        float xLow  = (float)0.025;
        float xHigh = xDiv;

        for(int idxx=0; idxx<numPadsX; idxx++)
        {
            char padTitle[6];
            sprintf(padTitle,"pad_%0i",idx);
            pads[idx++] = new TPad(padTitle,"Drawing Pad",xLow,yLow,xHigh,yHigh,19);
            xLow   = xHigh;
            xHigh += xDiv;
        }
        yHigh  = yLow;
        yLow  -= yDiv;
    }

    for(idx=0; idx<nPads; idx++) pads[idx]->Draw();
    pads[0]->cd();
    pCan->Update();

    // Enter event loop, one can now interact with the objects in
    // the canvas. Select "Exit ROOT" from Canvas "File" menu to exit
    // the event loop and execute the next statements.
    //pTheApp.Run(kTRUE);

    //Find and draw the PSF plots
    TFolder* mFolder = (TFolder*)gROOT->GetRootFolder()->FindObject("MeritFolder");

    TFolder* psfvals = (TFolder*)mFolder->FindObject("PSFValues");
    if (psfvals)
    {
        char histName[] = "PSFhist_cn_en";
        int  cosBin     = 0;
        int  eneBin     = 6;

        while(eneBin--)
        {
            sprintf(histName,"PSFhist_c%i_e%i",cosBin,eneBin);

            PSFanalysis* pAnal = (PSFanalysis*)psfvals->FindObject(histName);

            TString objName(pAnal->GetName());

            printf(" ** Found object %s **\n",objName.Data());

            TPad* pPad = pads[eneBin];
            pPad->cd();
            pPad->Clear();
            pAnal->Draw();
            pPad->Update();
        }
    }

    //Go into the event loop so we can look at the plots
    pTheApp->Run(kTRUE);

    //Clean up nicely...
    idx = nPads;
    while(idx--) delete pads[idx];
    delete pads;
    delete pCan;

    return;
}

void plotPSFvsE()
{
  //Do we need a new Canvas?
  //if (numPadsX != 2 || numPadsY != 2) newCanvas(2,2);

  //Now get the folder containing the histograms
    TFolder* merit = (TFolder*)gROOT->GetRootFolder()->FindObject("MeritFolder");

    TFolder* psfvals = (TFolder*)merit->FindObject("PSFValues");
    if (psfvals)
    {
        char histName[] = "PSFhist_cn_en";
        int  cosBin     = 4;

        while(cosBin--)
        {
            int   eneBin = 6;
            float eneMin[6];
            float eneMax[6];
            float eMean[6];
            float psf68[6];
            float psf95[6];

            while(eneBin--)
            {
                sprintf(histName,"PSFhist_c%i_e%i",cosBin,eneBin);

                PSFanalysis* pAnal = (PSFanalysis*)psfvals->FindObject(histName);

                eneMin[eneBin] = pAnal->minE();
                eneMax[eneBin] = pAnal->maxE();
                eMean[eneBin]  = pAnal->meanE();
                psf68[eneBin]  = pAnal->percentile(68.) * 180./3.14159;
                psf95[eneBin]  = pAnal->percentile(95.) * 180./3.14159;
            }

            printf("**********************************************\n");
            eneBin = 6;
            while(eneBin--)
            {
                printf("** E min=%8.4f, E mean=%8.4f, E max=%8.4f **\n",eneMin[5-eneBin],eMean[5-eneBin],eneMax[5-eneBin]);
                printf("** PSF 68%% = %8.4f degrees, 95%% = %8.4f **\n",psf68[5-eneBin],psf95[5-eneBin]);
            }
        }
    }

    return;
}
*/