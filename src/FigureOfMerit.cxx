//  $Header: /nfs/slac/g/glast/ground/cvs/merit/src/FigureOfMerit.cxx,v 1.9 2001/12/18 23:28:29 usher Exp $


#include "FigureOfMerit.h"
#include "Cut.h"
#include "MultiPSF.h"
#include "Level1.h"
#include "PSFtailCuts.h"

#include <cmath>
#include <iomanip>
#include <algorithm>

// put these here because gcc could not find the inline versions???
unsigned        FigureOfMerit::accepted() const { return m_accepted; }
float    FigureOfMerit::area(){return s_area;}
unsigned FigureOfMerit::generated()  { return s_generated; }

using namespace std;

static inline double sqr(double x){return x*x;}
//=============================================================================
// Figure of Merit analysis cut functions: allow dynamic allocation/ordering of a list
//	of analysis cuts which directly interface with Figure of Merit.

//=============================================================================
// utility that sums the variable
class Summation : public Analyze {
public:
    Summation(const Tuple&t , const char* item, const char* label )
        : Analyze(t, item, label), m_total(0) {};
    float average()const{return count() ? m_total/count():0;}
    float total()const{return m_total;}
private:
    float  m_total; 
    virtual bool apply () { m_total += item();  return true;   }
};
//=============================================================================
class FOMelapsed : public Analyze {
public:
    FOMelapsed(const Tuple&t ): Analyze(t, "Triage_Time", "Elapsed time (sec):"), m_total(0),m_last(0) {};
    void FOMelapsed::report(std::ostream& out)
    {
	out << endl << make_label(name());
	out << setw(6) << m_total;
    }
    double time()const{return m_total;}

private:
    float  m_total, m_last;

    virtual bool apply (){ 
        float val = item();
        if( val < m_last ) m_last=0;  // in case concatenated input files
        m_total += val - m_last;
        m_last = val;
        return    true;
    };
};

//=============================================================================
class FOMtrigrate : public FOMelapsed {
public:
    FOMtrigrate(const Tuple&t ): FOMelapsed(t) {};
    void FOMtrigrate::report(std::ostream& out)
    {
	out << endl << make_label("Rate:");
	out << setw(6) << setprecision(3);
        float rate = (time() ? count()/time() : 0);
        if( rate<1000. )  out  << rate << " Hz";
        else out << rate/1000. << " kHz";
    }

private:
};

//=============================================================================
class FOMdeadtime : public Summation {
public:
    FOMdeadtime(const Tuple&t ): Summation(t, "Dead_Time", "Dead Time:"){};
    void FOMdeadtime::report(std::ostream& out)
    {
	out << endl << make_label(name());
	out << setw(6) << total();
    }
};
//=============================================================================
class FOMROtime : public Summation {
public:
    FOMROtime(const Tuple&t ): Summation(t, "Max_RO_Time", "Readout Time (max):"){};
    void FOMROtime::report(std::ostream& out)
    {
	out << endl << make_label(name());
	out << setw(6) << average();
    }
};

//=============================================================================
class FOMevtsize : public Summation {
public:
    FOMevtsize(const Tuple&t ): Summation(t, "Total_Evt_Size", "Event Size (total):"){};
    void FOMevtsize::report(std::ostream& out)
    {
	out << endl << make_label(name());
	out << setw(6) << average() << " bits";
    }
};
  //=============================================================================
class EventSize : public Analyze{
public:
    EventSize(const Tuple&t ) : Analyze("Average Event size"),
        m_acd(t, "ACD_TileCount", "ACD Hits (19 b)"),
        m_ssd(t, "TKR_Cnv_Lyr_Hits", "SSD Hits (20 b)"),
        m_cal(t, "Cal_No_Xtals", "CAL Hits (40 b)")
    {}
    void EventSize::report(std::ostream& out)
    {   
        float size 
            = 19* m_acd.stat().mean()
            + 20* m_ssd.stat().mean()
            + 40* m_cal.stat().mean();
        separator(out);
        out << endl << make_label(name()) 
            << setw(6)<< setprecision(3) << size/1e3 <<" kbits";

        m_acd.report(out);
        m_ssd.report(out);
        m_cal.report(out);
    }
    bool apply(){
        m_acd(); m_ssd(); m_cal();
    return true;}

private:
    Statistic m_acd;
    Statistic m_ssd;
    Statistic m_cal; 
};
//=============================================================================
class FOMnsistrips : public Summation {
public:
    FOMnsistrips(const Tuple&t ): Summation(t, "TKR_Cnv_Lyr_Hits", "Tracker hits (avg):"){};
    void FOMnsistrips::report(std::ostream& out)
    {
	out << endl << make_label(name());
	out << setw(6) << average() << " hits";
    }

};

//=============================================================================
class FOMncsixtals : public Summation {
public:
    FOMncsixtals(const Tuple&t ): Summation(t, "Cal_No_Xtals", "CsI logs hit (avg):"){};
    void FOMncsixtals::report(std::ostream& out)
    {
	out << endl << make_label(name());
	out << setw(6) << average() << " logs";
    }
};

//=============================================================================
class FOMnacdtiles : public Summation {
public:
    FOMnacdtiles(const Tuple&t ): Summation(t, "ACD_TileCount", "ACD tiles hit (avg):"){};
    void FOMnacdtiles::report(std::ostream& out)
    {
	out << endl << make_label(name());
	out << setw(6) << average() << " tiles";
    }
};

//=============================================================================
class FOMtrig1Cal : public Analyze {
public:
    FOMtrig1Cal(const Tuple&t, int mask=64 )
        : Analyze(t, "Trig_Bits", "Level1")
        ,m_mask(mask){};
private:
    virtual bool apply () {
        return ( (static_cast<int>(item()) & m_mask )==m_mask); } //  cal
    int m_mask;
};

//=============================================================================
// Level  2 analysis
class Level2 : public Analyze {	
public:
    Level2(const Tuple& t): Analyze(t, "Trig_Level2_Bits", "Level2")
        , m_trig_cal(t,32)
    { clear(); };
    void report(std::ostream& out)
    {
        out << endl << name() ;
        out << endl << "          track:" << setw(8) << m_track;
        out << endl << "           veto:" << setw(8) << m_veto;
        out << endl << "  veto & !hical:" << setw(8) << m_cal;
        Analyze::report(out);
        separator(out);
    }
    void clear(){m_track=m_veto=m_cal=0; Analyze::clear();}
private:
    virtual bool  apply ()
    { 
        int trig2 = static_cast<unsigned>(item()) & 15;
        bool cal = m_trig_cal() ;
        if( trig2 & 1){
            m_track++; 
            if( trig2 >1 ){
                m_veto++;
                if( !cal) m_cal++ ;
            }
        }
        return trig2 == 1 ||  cal;
    }
    FOMtrig1Cal m_trig_cal;
    int m_track; // number with track bit set
    int m_veto;  // number with also veto
    int m_cal;   // calorimeter trigger overrides veto

};
//=============================================================================
class FOMtrigII : public Analyze {	
public:
    FOMtrigII(const Tuple& t): Analyze(t, "Trig_Level2_Bits", "Level2 3row only"){};
private:
    virtual bool  apply (){ return ((static_cast<unsigned>(item()) & 1) == 1); }
};
//=====================================================================================
class FOML2T : public Analyze {
public:
    FOML2T (const Tuple& t) 
        : Analyze (t, "ACD_DOCA", "L2 Selections")
        , m_sourceid(t.tupleItem ("MC_src_Id"))
	  , m_numtracks(t.tupleItem ("TKR_No_Tracks"))
	  , m_edeposit(t.tupleItem ("Cal_Energy_Deposit"))
	  , m_acd_topdoca(t.tupleItem ("ACD_TopDOCA"))
	  , m_acd_s0doca(t.tupleItem ("ACD_S0DOCA"))
	  , m_acd_s1doca(t.tupleItem ("ACD_S1DOCA"))

    {    }
private:
    virtual bool    apply () {
        float doca = item();
	    float sourceid = *m_sourceid;
        float numtracks = *m_numtracks;
        float edeposit = *m_edeposit;
        float acd_topdoca = *m_acd_topdoca;
        float acd_s0doca = *m_acd_s0doca;
        float acd_s1doca = *m_acd_s1doca;
        bool newdoca = min(acd_topdoca,min(acd_s0doca,acd_s1doca))>25.;
//        return (sourceid!=2&&((numtracks>0.&&doca>25.)||(edeposit>10000.)));
        return ((numtracks>0.&&newdoca)||edeposit>10000.); // we'll want to use this
    }
    
    const TupleItem*	m_sourceid;
    const TupleItem*	m_numtracks;
    const TupleItem*	m_edeposit;
    const TupleItem*	m_acd_topdoca;
    const TupleItem*	m_acd_s0doca;
    const TupleItem*	m_acd_s1doca;
};
//=============================================================================
class FOML3T : public Analyze {
public:
    FOML3T (const Tuple& t) 
        : Analyze (t, "Cal_Energy_Deposit", "L3 Selections")
        , m_surplushit(t.tupleItem ("REC_Surplus_Hit_Ratio"))
	  , m_xtalrat(t.tupleItem ("Cal_Xtal_Ratio"))
	  , m_fiterrnrm(t.tupleItem ("Cal_Fit_errNrm"))
      , m_tilecount(t.tupleItem ("ACD_TileCount"))
	  , m_siderow3(t.tupleItem ("ACD_No_SideRow3"))
	  , m_siderow2(t.tupleItem ("ACD_No_SideRow2"))
	  , m_elayer0(t.tupleItem ("Cal_eLayer0"))
	  , m_elayer1(t.tupleItem ("Cal_eLayer1"))
	  , m_elayer2(t.tupleItem ("Cal_eLayer2"))
	  , m_elayer3(t.tupleItem ("Cal_eLayer3"))
	  , m_elayer4(t.tupleItem ("Cal_eLayer4"))
	  , m_elayer5(t.tupleItem ("Cal_eLayer5"))
	  , m_elayer6(t.tupleItem ("Cal_eLayer6"))
	  , m_elayer7(t.tupleItem ("Cal_eLayer7"))
	  , m_s2doca(t.tupleItem ("ACD_S2DOCA"))
	  , m_tkr_zbottom(t.tupleItem ("TKR_Zbottom"))
	  , m_tkr_skirtx(t.tupleItem ("REC_Tkr_SkirtX"))
	  , m_tkr_skirty(t.tupleItem ("REC_Tkr_SkirtY"))
	  , m_tkr_no_tracks(t.tupleItem ("TKR_No_Tracks"))


    {    }
private:
    virtual bool    apply () {
        float edeposit = item();
    	float surplushit = *m_surplushit;
        float xtalrat = *m_xtalrat;
        float fiterrnrm = *m_fiterrnrm;
    	float tilecount = *m_tilecount;
        float siderow3 = *m_siderow3;
        float siderow2 = *m_siderow2;
        float elayer0 = *m_elayer0;
        float elayer1 = *m_elayer1;
        float elayer2 = *m_elayer2;
        float elayer3 = *m_elayer3;
        float elayer4 = *m_elayer4;
        float elayer5 = *m_elayer5;
        float elayer6 = *m_elayer6;
        float elayer7 = *m_elayer7;
        float s2doca  = *m_s2doca;

        float tkr_zbottom  = *m_tkr_zbottom;
        float tkr_skirty  = *m_tkr_skirty;
        float tkr_skirtx  = *m_tkr_skirtx;
        float tkr_no_tracks  = *m_tkr_no_tracks;
        
        float rat0 = 2.;
        float rat7 = 2.;
        if (edeposit>0.) {
              rat0=elayer0/edeposit;
              rat7=elayer7/edeposit;
        }

        bool ACDinfo = ((tilecount-siderow3-siderow2)<1.||edeposit>350.) && (edeposit>0.||tilecount<1.)&&
            (s2doca<40.||s2doca>199.||edeposit>500.);
        bool CALinfo = ((edeposit<1.&&surplushit>2.)||(edeposit>0.&&(xtalrat>.2||edeposit>1000.)))&&(fiterrnrm<15.||edeposit<300.) &&
            (edeposit==0.||(rat0>0.01&&rat0<.9));
        bool TKRinfo = ((tkr_no_tracks>2.&&edeposit>0.)||
        ((abs(tkr_skirtx)<70.||abs(tkr_skirtx)>85.)&&(abs(tkr_skirty)<70.||abs(tkr_skirty)>85.)))&&
        (surplushit>2.||edeposit>350.)&&(tkr_zbottom<2.||edeposit==0.);



        return (ACDinfo&&CALinfo&&TKRinfo);
    }
    const TupleItem*	m_surplushit;
    const TupleItem*	m_xtalrat;
    const TupleItem*	m_fiterrnrm;
    const TupleItem*	m_tilecount;
    const TupleItem*	m_siderow3;
    const TupleItem*    m_siderow2;
    const TupleItem*    m_elayer0;
    const TupleItem*    m_elayer1;
    const TupleItem*    m_elayer2;
    const TupleItem*    m_elayer3;
    const TupleItem*    m_elayer4;
    const TupleItem*    m_elayer5;
    const TupleItem*    m_elayer6;
    const TupleItem*    m_elayer7;
    const TupleItem*    m_s2doca;
    const TupleItem*    m_tkr_zbottom;
    const TupleItem*    m_tkr_skirty;
    const TupleItem*    m_tkr_skirtx;
    const TupleItem*    m_tkr_no_tracks;

};
//=============================================================================
class FOML1V : public Analyze {
public:
    FOML1V (const Tuple& t) 
        : Analyze (t, "ACD_Throttle_Bits", "L1 Throttle")
        , m_tilecount(t.tupleItem ("ACD_TileCount"))
	  , m_siderow3(t.tupleItem ("ACD_No_SideRow3"))
	  , m_siderow2(t.tupleItem ("ACD_No_SideRow2"))
	  , m_trigbits(t.tupleItem ("Trig_Bits"))

    {    }
private:
    virtual bool    apply () {
        float throttlebits = item();
	  float tilecount = *m_tilecount;
        float siderow3 = *m_siderow3;
        float siderow2 = *m_siderow2;
        int trigbits = *m_trigbits;

        return (((throttlebits==0||throttlebits>127.)&&(tilecount-siderow2-siderow3)<3.)||(trigbits&16));
    }
    
    const TupleItem*	m_throttlebits;
    const TupleItem*	m_tilecount;
    const TupleItem*	m_siderow3;
    const TupleItem*    m_siderow2;
    const TupleItem*    m_trigbits;
};
//=============================================================================
class WriteTuple : public Analyze {
public:
    WriteTuple(const Tuple& t) : Analyze( "Written to output tuple"),m_tuple(t)
    {
        m_tuple.writeHeader(std::cout);
    }
private:
    virtual bool    apply () {std::cout << m_tuple; ; return true; }
    const Tuple& m_tuple;
};
//=============================================================================
// Only the Front
class FOMFront : public AnalysisList {	
public: 

    FOMFront(const Tuple& t): AnalysisList(" TKR Front only")
    {
        push_back( new Cut(t, "TKR_First_XHit<12.") );
    };
    void report(std::ostream& out)
    {
        separator(out);
        AnalysisList::report(out);
    }
};
//=============================================================================
// Only the Back
class FOMBack : public AnalysisList {	
public: 

    FOMBack(const Tuple& t): AnalysisList(" TKR Back only")
    {
        push_back( new Cut(t, "TKR_First_XHit>11.") );
    };
    void report(std::ostream& out)
    {
        separator(out);
        AnalysisList::report(out);
    }
};
//=============================================================================
// Analyze the level 3 cuts
class Level3 : public AnalysisList {	
public: 

    Level3(const Tuple& t): AnalysisList(" Level 3")
    {
        push_back( new Cut(t, "Cal_No_Xtals>0") );
        push_back( new Cut(t, "TKR_No_Tracks>0") );
        push_back( new Cut(t, "ACD_DOCA>25") );
        //push_back( new Cut(t, "CsI_Fit_errNrm>10"));
    };
    void report(std::ostream& out)
    {
        separator(out);
        AnalysisList::report(out);
    }

};
//=============================================================================
// Analyze the Atwood cuts
class CosmicCuts : public AnalysisList {
public:
    CosmicCuts(const Tuple&t, bool noline=false)
        : AnalysisList("  ---Cosmic cuts---", noline)
    {
        push_back( new Cut(t, "REC_Surplus_Hit_Ratio", Cut::GT, 2.05) );
        push_back( new Cut(t, "ACD_DOCA",      Cut::GT, 30) );
        push_back( new Cut(t, "Cal_Fit_errNrm", Cut::LT, 5) );
        push_back( new Cut(t, "Cal_Xtal_Ratio",  Cut::GT, 0.25) );
    }
};


//=============================================================================
class FOMaccepted : public Analyze {
public:
    FOMaccepted(FigureOfMerit* f) : Analyze("Accepted for analysis"),m_f(f) {}
private:
    virtual bool    apply () { m_f->accept(); return true; }
    FigureOfMerit* m_f;
};

//=============================================================================

const Tuple*	 FigureOfMerit::s_tuple=0;


unsigned FigureOfMerit::s_generated = 100000;
double FigureOfMerit::s_area = 60000.;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void	FigureOfMerit::setCuts ( string istr )
{
    delete m_cuts;
    m_cuts = new AnalysisList( string("Analysis cuts: ")+istr );
    string::const_iterator	it = istr.begin();

    m_cuts->push_back( new Analyze("Found in tuple") );  // an event count for starting (# in tuple)
    m_cuts->push_back( new Statistic(*s_tuple, "MC_Energy", "Generated energy"));
    
    while (it != istr.end()) {
        switch (*it++) {
            
        case 'G': // Gnn, -- override number generated
        {
            s_generated= ::atoi(it);
            while( it !=istr.end() && (*it++)!=',');
            cerr << "Overriding generated number to " << s_generated << endl;
            break;
        }
        case '1':	    // 1(one) = level I trigger
        {
            m_cuts->push_back( new Level1(*s_tuple) );
            break;
        }
            // added 18 oct 2001 by S.Ritz
        case 'V':        // V = level 1 VETO using ACD
            m_cuts->push_back( new FOML1V(*s_tuple) );
            break;
            //change 18 oct 2001 by S.Ritz to new L2T and L3T selections
        case '2':	    // 2 = level II trigger - no veto
            m_cuts->push_back( new FOML2T(*s_tuple) );
            break;
        case '3':	    // 3 = level 3 trigger 
       	    m_cuts->push_back( new FOML3T(*s_tuple) );
            break;
            // 18 october 2001 by S.Ritz -- add selections for Front and Back
        case 'F':		// F = Front TKR section only
            m_cuts->push_back( new FOMFront(*s_tuple) );
            break;
        case 'B':		// B = Back TKR section only
            m_cuts->push_back( new FOMBack(*s_tuple) );
            break;
        case 'd':		// d = level II trigger - with veto
            m_cuts->push_back( new FOMtrigII(*s_tuple) );
            break;
        case 'n':	    // n = ntracks
            m_cuts->push_back( new Cut(*s_tuple, "TKR_No_Tracks>0" ) );
            break;
        case 'c':	    // c = Cosmic cuts
            m_cuts->push_back( new CosmicCuts(*s_tuple) );
            break;
        case 's':
            m_cuts->push_back( new EventSize(*s_tuple) );
            break;
        case 't':
            m_cuts->push_back( new FOMROtime(*s_tuple) );
            break;
        case 'M': //M0, M1, ...
            m_cuts->push_back( new MultiPSF(*s_tuple, *(it++)) );
            break;
        case 'j': // Jose style resolution cuts (again)
            m_cuts->push_back( new PSFtailCuts(*s_tuple) );
            break;            
        case 'P': /* P = PSF analysis   */  
            m_cuts->push_back( new PSFanalysis(*s_tuple) );   
            break;
        case 'E':	    
            break;            
        case 'A': /* A = accepted */
            m_layers.push_back(LayerGroup(*s_tuple,0,11));
            m_layers.push_back(LayerGroup(*s_tuple,12,15));            
            m_cuts->push_back( new FOMaccepted(this) );	    
            break;            
        case 'W': /* W = Write */        
            m_cuts->push_back( new WriteTuple(*s_tuple));   
            break;
        case 'D': /* D = dead time */	 
            m_cuts->push_back( new FOMdeadtime(*s_tuple) ); 
            break;
        case 'L': /* L = elapsed time */ 
            m_cuts->push_back( new FOMelapsed(*s_tuple) );  
            break;
        case 'R': /* R = trigger rate */ 
            m_cuts->push_back( new FOMtrigrate(*s_tuple) ); 
            break;
        case '(': // (cut) -- pass in the iterator
            m_cuts->push_back(new Cut(*s_tuple, it, istr.end() )); 
            break; 
        case 'X': // Xname, statistic on name
            m_cuts->push_back(new Statistic(*s_tuple, it, istr.end() )); 
            break;
        default: 
            cerr << "Key '" << *(it-1) << "' ignored" << endl;
            break;
        }   // switch
    }	// while
    
}
//=============================================================================
FigureOfMerit::FigureOfMerit(const Tuple& t, std::string cut_string)
: m_cuts(0)
, m_accepted(0)
{
    if (!s_tuple)	{   // initialize static member functions
	s_tuple = &t;

	// analyze title for parameters
	string title(t.title());
	string::size_type pos = title.find("area =");
	if( pos != string::npos ) {
	    string a(title, pos+6, title.length() );
	    s_area = atof(a.data());
	}
        else {
            cerr << "Area not found in title: assuming "
                << s_area <<" cm^2" << endl;
        }

	// look for number generated in title: either generated:1000 or gen(1000)
	pos = title.find("generated");
	if( pos != string::npos ) {
	    string a(title, pos+10, 10);
	    s_generated = atoi(a.data());
	}
	else if ( (pos = title.find("gen(")) != string::npos) {
            string a(title, pos+4, 10);
	    s_generated = atoi(a.data());
	}
	else	cerr << "generated events not found in title" << endl;
    }	// if (!s_instance)

    if( !cut_string.empty()) setCuts(cut_string);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void FigureOfMerit::accept()
{
    m_accepted++;
    for( std::vector<LayerGroup>::iterator layer = m_layers.begin(); layer!=m_layers.end(); ++layer) (*layer)();

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool FigureOfMerit::execute()
{
    return (*m_cuts)();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void FigureOfMerit::report(std::ostream & out)
{
    out	<< "\n"<< Analyze::make_label("Generated events") << setw(6) << generated() ;

    Analyze::separator(out,'=');
    m_cuts->report(out);	// print analysis cuts

    if( ! accepted() ) return;

    float  area_per_event = s_area/s_generated,  fom_total=0;

    // loop over the ranges of layers
    for( std::vector<LayerGroup>::iterator layers = m_layers.begin(); layers!=m_layers.end(); ++layers){
        layers->report(out);
        float eff_area = area_per_event*layers->count();
        
        out	<< "\n" << Analyze::make_label(" effective area")
            << setw(6)
            << static_cast<int>(eff_area+0.5) << " cm^2";
        float fom = sqrt(eff_area)/layers->sigma();
        out << "\n" << Analyze::make_label("Figure of merit")
            << setw(6)
            << static_cast<int>(fom+0.5) << " cm";
        
        Analyze::separator(out);
        fom_total = sqrt(sqr(fom_total)+sqr(fom));
    }
    out	<< "\n" << Analyze::make_label("total effective area")
	<< setw(6)
	<< static_cast<int>(area_per_event*accepted()+0.5) << " cm^2";

    out << "\n" << Analyze::make_label("Combined FOM")
        << setw(6) << static_cast<int>(fom_total+0.5) << " cm";

    out	<< endl;
}


