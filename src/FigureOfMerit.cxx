//  $Header: /cvs/glastsim/merit/FigureOfMerit.cxx,v 1.45 1999/08/27 20:14:24 burnett Exp $

#ifdef __GNUG__
#pragma implementation
#endif

#include "FigureOfMerit.h"
#include "Cut.h"
#include "MultiPSF.h"
#include "Level1.h"
#include "BackgroundCuts.h"
#include "PSFtailCuts.h"

#include <cmath>
#include <iomanip>
#include <algorithm>


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
    void FOMelapsed::report(ostream& out)
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
    void FOMtrigrate::report(ostream& out)
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
    void FOMdeadtime::report(ostream& out)
    {
	out << endl << make_label(name());
	out << setw(6) << total();
    }
};
//=============================================================================
class FOMROtime : public Summation {
public:
    FOMROtime(const Tuple&t ): Summation(t, "Max_RO_Time", "Readout Time (max):"){};
    void FOMROtime::report(ostream& out)
    {
	out << endl << make_label(name());
	out << setw(6) << average();
    }
};

//=============================================================================
class FOMevtsize : public Summation {
public:
    FOMevtsize(const Tuple&t ): Summation(t, "Total_Evt_Size", "Event Size (total):"){};
    void FOMevtsize::report(ostream& out)
    {
	out << endl << make_label(name());
	out << setw(6) << average() << " bits";
    }
};
  //=============================================================================
class EventSize : public Analyze{
public:
    EventSize(const Tuple&t ) : Analyze("Average Event size"),
        m_acd(t, "No_Vetos_Hit", "ACD Hits (19 b)"),
        m_ssd(t, "Cnv_Lyr_Hits", "SSD Hits (20 b)"),
        m_cal(t, "CsI_No_Xtals", "CAL Hits (40 b)")
    {}
    void EventSize::report(ostream& out)
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
    FOMnsistrips(const Tuple&t ): Summation(t, "N_Tracker_Hits", "Tracker hits (avg):"){};
    void FOMnsistrips::report(ostream& out)
    {
	out << endl << make_label(name());
	out << setw(6) << average() << " hits";
    }

};

//=============================================================================
class FOMncsixtals : public Summation {
public:
    FOMncsixtals(const Tuple&t ): Summation(t, "CsI_No_Xtals", "CsI logs hit (avg):"){};
    void FOMncsixtals::report(ostream& out)
    {
	out << endl << make_label(name());
	out << setw(6) << average() << " logs";
    }
};

//=============================================================================
class FOMnacdtiles : public Summation {
public:
    FOMnacdtiles(const Tuple&t ): Summation(t, "No_Vetos_Hit", "ACD tiles hit (avg):"){};
    void FOMnacdtiles::report(ostream& out)
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
    Level2(const Tuple& t): Analyze(t, "Trig_Level2_Bits", " Level2")
        , m_trig_cal(t,32)
    { clear(); };
    void report(ostream& out)
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
// Analyze the level 3 cuts
class Level3 : public AnalysisList {	
public: 

    Level3(const Tuple& t): AnalysisList(" Level 3")
    {
        push_back( new Cut(t, "CsI_No_Xtals>0") );
        push_back( new Cut(t, "No_Tracks>0") );
        push_back( new Cut(t, "Veto_DOCA>25") );
        //push_back( new Cut(t, "CsI_Fit_errNrm>10"));
    };
    void report(ostream& out)
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
        push_back( new Cut(t, "Surplus_Hit_Ratio", Cut::GT, 2.05) );
        push_back( new Cut(t, "Veto_DOCA",      Cut::GT, 30) );
        push_back( new Cut(t, "CsI_Fit_errNrm", Cut::LT, 5) );
        push_back( new Cut(t, "CsI_Xtal_Ratio",  Cut::GT, 0.25) );
    }
};
//=============================================================================
class ResolutionCuts : public AnalysisList {	
public:
    ResolutionCuts(const Tuple& t, bool noline=false)
        : AnalysisList("  ---Resolution cuts---",noline)
    {
        push_back( new Cut(t, "No_Tracks",      Cut::GT,  0,  "track found") );
        push_back( new Cut(t, "Chisq",          Cut::LT, 50) );
        push_back( new Cut(t, "First_Fit_Gaps", Cut::LT, 0.5) );
        push_back( new Cut(t, "Diff_1st_XY_Lyr",Cut::EQ, 0) );
        push_back( new Cut(t, "Active_Dist",    Cut::GT, 0 ) );
    };
};
//=============================================================================
class Atwood : public AnalysisList {	
public:
    Atwood(const Tuple& t): AnalysisList(" Atwood cuts")
    {
        push_back( new ResolutionCuts(t,true));
        push_back( new CosmicCuts(t, true) );
    };
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


unsigned FigureOfMerit::s_generated = 10000;
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
	case 'a':	    // a = Atwood cuts
            m_cuts->push_back( new Atwood(*s_tuple) );
	    break;

	case '1':	    // 1(one) = level I trigger
	    m_cuts->push_back( new Level1(*s_tuple) );
	    break;
	case 'I':	    // I = level I trigger with ACD
	    m_cuts->push_back( new Level1(*s_tuple,true) );
	    break;
	case '2':	    // 2 = level II trigger - no veto
	    m_cuts->push_back( new Level2(*s_tuple) );
	    break;
	case '3':	    // 3 = level 3 trigger 
       	    m_cuts->push_back( new Level3(*s_tuple) );
	    break;

	case 'd':		// d = level II trigger - with veto
		m_cuts->push_back( new FOMtrigII(*s_tuple) );
		break;
	case 'n':	    // n = ntracks
	    m_cuts->push_back( new Cut(*s_tuple, "No_Tracks>0" ) );
	    break;
	case 'r':	    // r = resolution cuts
	    m_cuts->push_back( new ResolutionCuts(*s_tuple) );
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

        case 'S': ; 
        case 'b': //b for background (or S for Steve Ritz) 
            m_cuts->push_back( new BackgroundCuts(*s_tuple) );
            break;
        case 'j': //j for Jose's cuts
            m_cuts->push_back( new PSFtailCuts(*s_tuple) );
            break;

	case 'P': /* P = PSF analysis   */  m_cuts->push_back( new PSFanalysis(*s_tuple) );   break;
	case 'E':	    break;

	case 'A': /* A = accepted */
            m_layers.push_back(LayerGroup(*s_tuple,0,11));
            m_layers.push_back(LayerGroup(*s_tuple,12,15));

            m_cuts->push_back( new FOMaccepted(this) );	    break;

        case 'W': /* W = Write */        m_cuts->push_back( new WriteTuple(*s_tuple)); break;
	case 'D': /* D = dead time */	 m_cuts->push_back( new FOMdeadtime(*s_tuple) ); break;
	case 'L': /* L = elapsed time */ m_cuts->push_back( new FOMelapsed(*s_tuple) ); break;
	case 'R': /* R = trigger rate */ m_cuts->push_back( new FOMtrigrate(*s_tuple) ); break;
        case '(': // (cut) -- pass in the iterator
            m_cuts->push_back(new Cut(*s_tuple, it, istr.end() )); break; 
        case 'X': // Xname, statistic on name
            m_cuts->push_back(new Statistic(*s_tuple, it, istr.end() )); break;
        default:  /* counter */		    m_cuts->push_back( new Analyze() );	    break;
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
void FigureOfMerit::execute()
{
    (*m_cuts)();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void FigureOfMerit::report(ostream & out)
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



