// $Header: /nfs/slac/g/glast/ground/cvs/merit/src/app/meritapp.cxx,v 1.4 2001/05/30 22:09:28 burnett Exp $

// Main for merit

// either tuple
#include "AsciiTuple.h"
#include "RootTuple.h"

#include "../Analyze.h"
#include "../FigureOfMerit.h"

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/timeb.h>
#include <string>
#include <strstream>

#include <assert.h>

const char* _MERIT_VERSION = "$Revision: 1.4 $";
static std::string  cutstr("nA");
static std::string  file_name("");



void FATAL(const char* s){std::cerr << "\nERROR: "<< s;}
void WARNING(const char* s){std::cerr << "\nWARNING: " << s;}

static timeb t_init, t_final;
static const char* helptext=
"\n----------------------------------------------"
"\nCalling syntax: merit  -CUTS [p] [file1 | - ] [file2] ..."
"\n\nAnalysis cuts: allow re-ordering of gamma rejection cuts and trigger"
"\ncounting at runtime. Order of cuts is same as in this string, specific"
"\n'cuts' will increment the counters merit uses for analysis of the tuple."
"\nIf 'p' is specified  then percentages will be displayed as well."
"\n Reads from stdin next parameter is \"-\", or from $(MERIT_INPUT_FILE) otherwise."
"\n\n\tCut keys are as follows:"
"\n\t\t1 : level 1 trigger: Track or LoCal "
"\n\t\tI : level 1 trigger: (Track or Local)* !Veto + HiCal "
"\n\t\t2 : level 2 trigger: track, no veto except if cal"
"\n\t\t3 : level 3 trigger: track, veto, calorimeter error"
"\n\t\tn : number of tracks (N_tracks > 0)"
"\n\t\tc : cosmic rejection cuts (old set)"
"\n\t\tb : background rejection cuts (Ritz version)"
"\n\t\tr : resolution enhancement cuts "
"\n\t\tj : Tail suppression cuts (Jose Hernando version) "
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
            Analyze::showperc(true);
            n++;
        }

        if ( argc>n ) {
            if ( argv[n][0] == '-' )  {	// this is the cut sequence
                cutstr = &argv[n][1];
            } else file_name = argv[n];	
        }
    }

    if( file_name=="" ) {
        const char * env = ::getenv("MERIT_INPUT_FILE");
        if( env ) file_name=env;
        else {
            std::cerr << "No input file specified, see help" << std::endl;
            exit(1);
        }
    }

	// setup output stream
	std::ostream* outstream = &std::cerr;
	const char * env = ::getenv("MERIT_OUTPUT_FILE");
	if( env ){
		outstream = new std::ofstream(env);
	}
    std::cerr << "Merit reading from file: \"" << file_name << "\"" << std::endl;
    // charge a head with ROOT here, figure out later how to do it consistently

//    RootTuple* tuple = new RootTuple("unknown", file_name, "AORECON/t1");
    RootTuple* tuple = new RootTuple("unknown", file_name, "PDR/t1");
    // now assign input stream, either the file name or std input
        // Determine the # of events stored in ntuple

    std::strstream title; 
    title << "gen(" << tuple->numEvents() << ")" << '\0';
    tuple->setTitle(title.str());

#if 0
    std::istream* infile;
    if( file_name== "-") {
        infile = &std::cin;
    } else {
        std::ifstream* realfile = new std::ifstream(file_name.c_str());
        if( !realfile->is_open()) {
            std::cerr << "Cannot open file \"" << file_name << "\"";
            exit(1);
        }

	infile = realfile;
    }
    Tuple* tuple;

    if( file_name.find_first_of(".root") >0 ) {
        // opened a root file
        tuple = new RootTuple("unknown title", file_name, "t1");
    }
    else {
        // assume it is ascii. (need to fix, I think).
        tuple = new AsciiTuple(*infile);
    }


#endif
    (*outstream) << "Tuple title: \""<< tuple->title() << "\"\n" ;

    FigureOfMerit fm(*tuple, cutstr);

    ::ftime(&t_init);

    while ( tuple->nextEvent() ) fm.execute();
    ::ftime(&t_final);
    fm.report(*outstream);
	
    std::cerr << "\nElapsed time: "<< t_final.time-t_init.time << " sec" << std::endl;
    return 0;
}

#if 0 // ascii multi-file here for future reference
    int line=0, m=1;
    for(;;) {

        // read and process the current file
        for(; ; ++line) {
	    (*infile) >> tup;
	    if( !infile->good() ) break;
	    fm.execute();	
        }
        if( argc<=++n) break;
        // setup next file
        delete infile;
        file_name = argv[n];
        std::ifstream* realfile = new std::ifstream(file_name.c_str());
        if( !realfile->is_open()) {
            std::cerr << "Cannot open file \"" << file_name << "\"";
            exit(1);
        }
        tup.nextStream(*(infile=realfile));
        std::cerr << "Tuple# "<< ++m << " \""<< tup.title() << "\"\n" ;

    }
#endif