//
// Implements the root to FigureOfMerit interface
//

#include "meritFoM.h"

#include "TROOT.h"
#include "TFrame.h"
#include "TPaveLabel.h"
#include "app/RootTuple.h"
#include "FigureOfMerit.h"

ClassImp(meritFoM)

static const char* _MERIT_VERSION = "$Revision: 1.7 $";
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

void	help(const char* cutstr)
{
    std::cerr << "\nmerit Version " << _MERIT_VERSION << " -- Help"
	   << helptext << "\n\n\tDefault: " << cutstr << std::endl;
}

meritFoM::meritFoM(const char* fileName, const char* cutstring)
{
    //Try help file first
    if ( strstr(fileName,"?") || strstr(fileName,"--help"))
    {
	    help(cutstring);
	    return;
    }

    //Make sure some file name was input
    if (!fileName)
    {
        std::cerr << "No input file name given " << std::endl;
        return;
    }

    //Output the filename being used
    std::cerr << "Merit reading from file: \"" << fileName << "\"" << std::endl;

	// setup output stream
	outstream = &std::cerr;
	const char * env = ::getenv("MERIT_OUTPUT_FILE");
	if( env ){
		outstream = new std::ofstream(env);
        std::cerr << "Merit writing output to file: \"" << env << "\"" << std::endl;
	}

    //Define the root tuple to read
    m_tuple = new RootTuple("unknown", fileName, "PDR/t1");

    //Now define the FigureOfMerit object to do the analysis
    m_FoM   = new FigureOfMerit(*m_tuple, cutstring);

    //Open a canvas for telling the world what is going on
    //m_canvas = new TCanvas("MCanvas", "Merit Status", 200, 300);
    m_canvas = 0;

    //((TFrame*)m_canvas->GetFrame())->SetFillColor(46);
    //m_canvas->SetFillColor(19);
    //m_canvas->SetBorderMode(28);
    //m_canvas->cd();

    //m_pad    = new TPad("MStat","Merit Status",0.025,0.025,0.975,0.975,19);
    //m_pad->Draw();
    //drawMerit();
    m_pad = 0;

    return;
}

meritFoM::~meritFoM()
{
    if (m_FoM)    delete m_FoM;
    if (m_tuple)  delete m_tuple;
    if (m_canvas) delete m_canvas;
    if (m_pad)    delete m_pad;
}

void meritFoM::execute()
{
    //drawRunning();

    while ( m_tuple->nextEvent() ) 
    {
        m_FoM->execute();
    }

    //drawFinished();

    return;
}

void meritFoM::report()
{
    (*outstream) << "Tuple title: \""<< m_tuple->title() << "\"\n" ;

    m_FoM->report(*outstream);

    std::cerr << "\nMerit Done Reporting Results" << std::endl;
	
    //std::cerr << "\nElapsed time: "<< t_final.time-t_init.time << " sec" << std::endl;

    return;
}

Analyze* meritFoM::getAnalysisListItem(int  i)
{
    AnalysisList* pAnalList = m_FoM->getAnalysisList();

    int idx = i;
    if (i <  0) idx = 0;
    if (i >= pAnalList->size()) idx = pAnalList->size() - 1;

    //Ok, there must be a better way to do this...
    std::vector<Analyze*>::iterator iter = pAnalList->begin();
    
    return iter[idx];
}

void meritFoM::drawMerit()
{
    if (m_pad)
    {
        TPad* pCurPad = (TPad*)gPad;

        m_pad->cd();
        m_pad->Clear();
        TPaveLabel merit(0.2,0.4,0.8,0.6,"Merit");
        merit.DrawClone();
        m_pad->Update();
        pCurPad->cd();
    }
}

void meritFoM::drawRunning()
{
    if (m_pad)
    {
        TPad* pCurPad = (TPad*)gPad;

        m_pad->cd();
        m_pad->Clear();
        TPaveLabel meritLabel(0.2,0.6,0.8,0.8,"Merit");
        meritLabel.DrawClone();
        TPaveLabel meritRun(0.2,0.3,0.8,0.5,"Executing");
        meritRun.DrawClone();
        m_pad->Update();
        pCurPad->cd();
    }
}

void meritFoM::drawFinished()
{
    if (m_pad)
    {
        TPad* pCurPad = (TPad*)gPad;

        m_pad->cd();
        m_pad->Clear();
        TPaveLabel merit(0.2,0.6,0.8,0.8,"Merit");
        merit.DrawClone();
        TPaveLabel meritDone(0.2,0.3,0.8,0.5,"Finished");
        meritDone.DrawClone();
        m_pad->Update();
        pCurPad->cd();
    }
}