// PSFanalysis.h


#ifndef PSFANALYSIS_H
#define PSFANALYSIS_H

#include "Analyze.h"

#include "analysis/RebinHist.h"

#include "analysis/Tuple.h"
#include "analysis/smplstat.h"

#include "math.h"
#include <iostream>

//=============================================================================
class PSFanalysis : public Analyze , public RebinHist{
// analysis of the Point Spread Function
public:
    PSFanalysis(const Tuple& t, double emin=0, double emax=0);
    // create new analysis object connected to the tuple, that will accept events in the 
    // energy range and layer range. (Default values: all events accepted)
    PSFanalysis();

    virtual void    report(std::ostream& out);
    // formatted report

    void            clear();

    unsigned	    count () const; 
    // return number used (override Analyze) 

    double          percentile(double percent);
    // return angle for cumulative distribution
    
    double          sigma();
    // return the effective sigma

    double          minE()  {return m_emin;}
    double          maxE()  {return m_emax;}
    double          meanE() {return exp(m_loge.mean());}

    void            row_report(std::ostream& out);
    // special row for making tables

private:
    virtual bool    apply ();
    // analyze an event from the tuple

    double          m_sigma;
    Analyze         m_energy;      // access to energy
    Analyze         m_first_layer; // access to (fit) conversion layer
    SampleStatistic m_loge;        // histogra
    double          m_emin;
    double          m_emax;        // optional energy cuts to apply
};

#endif


/*

    PSFanalysis(const Tuple& t, double emin=0, double emax=0);
    // create new analysis object connected to the tuple, that will accept events in the 
    // energy range and layer range. (Default values: all events accepted)
    PSFanalysis();

    virtual void report(std::ostream& out);
    // formatted report

    void clear();

    unsigned	    count () const; 
    // return number used (override Analyze) 

    double sigma();
    // return the effective sigma

    double percentile(double percent);
    // return angle for cumulative distribution

    void row_report(std::ostream& out);
    // special row for making tables

private:
    virtual bool  apply ();
    // analyze an event from the tuple

    double m_sigma;
    Analyze m_energy; // access to energy
    Analyze m_first_layer; // access to (fit) conversion layer
    SampleStatistic m_loge; // histogra
    double m_emin, m_emax; // optional energy cuts to apply
*/