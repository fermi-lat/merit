/** @file FigureOfMerit.cxx
  @brief Implementation of FigureOfMerit, many Analyze subclasses 


  $Header: /nfs/slac/g/glast/ground/cvs/merit/src/FigureOfMerit.cxx,v 1.22 2003/11/26 21:00:26 burnett Exp $
*/

#include "FigureOfMerit.h"
#include "Cut.h"
#include "MultiPSF.h"
#include "Level1.h"
#include "PSFtailCuts.h"

#include <cmath>
#include <iomanip>
#include <algorithm>
#include <fstream>

// put these here because gcc could not find the inline versions???
unsigned        FigureOfMerit::accepted() const { return m_accepted; }
float    FigureOfMerit::area(){return s_area;}
unsigned FigureOfMerit::generated()  { return s_generated; }


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
    FOMelapsed(const Tuple&t ): Analyze(t, "elapsed_time", "Elapsed time (sec):"), m_total(0),m_last(0) {};
    void FOMelapsed::report(std::ostream& out)
    {
        using namespace std;
        out << std::endl << make_label(name());
        out << setw(6) << m_total;
    }
    double time()const{return m_total;}

private:
    float  m_total, m_last;

    virtual bool apply (){ 
        float val = item();
        if( m_last>0 ){ // use first entry  for setting start time
            if( val < m_last ) m_last=0;  // in case concatenated input files
            m_total += val - m_last;
        }
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
        using namespace std;
        out << endl << make_label("Rate:");
	out << setw(6) << setprecision(3);
        float rate = (time() ? count()/time() : 0);
        if( rate<1000. )  out  << rate << " Hz";
        else out << rate/1000. << " kHz";
    }

private:
};


//=============================================================================
class FOMevtsize : public Summation {
public:
    FOMevtsize(const Tuple&t ): Summation(t, "Total_Evt_Size", "Event Size (total):"){};
    void FOMevtsize::report(std::ostream& out)
    {
        using namespace std;
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
        using namespace std;

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
        using namespace std;

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
        using namespace std;

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
        using namespace std;
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
class WriteTuple : public Analyze {
public:
    WriteTuple(const Tuple& t, std::ostream &stream = std::cout) 
        : Analyze( "Written to output tuple")
        , m_tuple(t)
        , m_stream(stream)
    {
        m_tuple.writeHeader(m_stream);
    }
private:
    virtual bool    apply () {m_stream  << m_tuple ; return true; }
    const Tuple& m_tuple;
    std::ostream& m_stream;
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
void	FigureOfMerit::setCuts ( std::string istr )
{
    using std::string;
    delete m_cuts;
    m_cuts = new AnalysisList( std::string("Analysis cuts: ")+istr );
    string::const_iterator	it = istr.begin();

    m_cuts->push_back( new Analyze("Found in tuple") );  // an event count for starting (# in tuple)
    m_cuts->push_back( new Statistic(*s_tuple, "McEnergy", "Generated energy"));
    
    while (it != istr.end()) {
        switch (*it++) {
            
        case 'G': // Gnn, -- override number generated
        {
            s_generated= ::atoi(&*it);
            while( it !=istr.end() && (*it++)!=',');
            std::cerr << "Overriding generated number to " << s_generated << std::endl;
            break;
        }
        case '1':	    // 1(one) = level I trigger
        {
            m_cuts->push_back( new Level1(*s_tuple) );
            break;
        }

        case 'F':		// F = Front TKR section only
            m_cuts->push_back( new FOMFront(*s_tuple) );
            break;
        case 'B':		// B = Back TKR section only
            m_cuts->push_back( new FOMBack(*s_tuple) );
            break;

        case 'n':	    // n = ntracks
            m_cuts->push_back( new Cut(*s_tuple, "TkrNumTracks>0" ) );
            break;
        case 's':      // event size
            m_cuts->push_back( new EventSize(*s_tuple) );
            break;
        case 'M': //M0, M1, ...
            m_cuts->push_back( new MultiPSF(*s_tuple, *(it++)) );
            break;
        case 't': // tail cuts
            m_cuts->push_back( new PSFtailCuts(*s_tuple) );
            break;            
        case 'P': /* P = PSF analysis   */  
            m_cuts->push_back( new PSFanalysis(*s_tuple) );   
            break;
        case 'E':	    
            break;            
        case 'A': /* A = accepted */
            m_layers.push_back(LayerGroup(*s_tuple,0,11, LayerGroup::VERTEX));
            m_layers.push_back(LayerGroup(*s_tuple,0,11, LayerGroup::ONE_TRACK));
            m_layers.push_back(LayerGroup(*s_tuple,12,15,LayerGroup::VERTEX));            
            m_layers.push_back(LayerGroup(*s_tuple,12,15,LayerGroup::ONE_TRACK));            
            m_cuts->push_back( new FOMaccepted(this) );	    
            break;            
        case 'W': /* W = Write */        
            m_cuts->push_back( new WriteTuple(*s_tuple));   
            break;
        case 'f': // f = file
            m_cuts->push_back( new WriteTuple(*s_tuple, *(new std::ofstream("merit.tup")) ));
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
            std::cerr << "Key '" << *(it-1) << "' ignored" << std::endl;
            break;
        }   // switch
    }	// while
    
}
//=============================================================================
FigureOfMerit::FigureOfMerit(const Tuple& t, std::string cut_string)
: m_cuts(0)
, m_accepted(0)
{
    using std::string;
    if (!s_tuple)	{   // initialize static member functions
        s_tuple = &t;

        // analyze title for parameters
        std::string title(t.title());
        std::string::size_type pos = title.find("area =");
        if( pos != string::npos ) {
            string a(title, pos+6, title.length() );
            s_area = atof(a.data());
        }
        else {
            std::cerr << "Area not found in title: assuming "
                << s_area <<" cm^2" << std::endl;
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
        else	std::cerr << "generated events not found in title" << std::endl;
    }	// if (!s_instance)

    //need this to add these aliases
    Tuple& tt = const_cast<Tuple&>(t);

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
    using namespace std;
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


