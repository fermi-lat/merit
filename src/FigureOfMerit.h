//  $Header: /nfs/slac/g/glast/ground/cvs/merit/src/FigureOfMerit.h,v 1.2 2001/11/01 17:28:00 burnett Exp $
//  Project: glast analysis
//   Author: Toby Burnett
//
//  Analyzes the glast tuple to determine a figure of merit that is relevant
// for the point source discovery potential of the instrument. For a gaussian
// point spread function it is
//    sqrt(Aeff)/sigma


#ifndef FIGUREOFMERIT_H
#define FIGUREOFMERIT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LayerGroup.h"
#include "PSFanalysis.h"
#include "EnergyAnalysis.h"
#include "AnalysisList.h"
#include <vector>


//====================================================================================
class FigureOfMerit
{
public:
    FigureOfMerit(const Tuple& t, std::string cutstring="");

    void            setCuts(std::string);
    // associate a sequence of cuts, identifed by characters in the string

    void            execute();
    // analyze the current tuple row

    void            report(std::ostream&);
    // write results, return acceptacnce

    void            accept();

    // process an accepted event
    unsigned        accepted() const;   // { return m_accepted; }

    static float    area();             //{return s_area;}
    static unsigned generated();        //  { return s_generated; }
    // data access

    AnalysisList*   getAnalysisList() {return m_cuts;}

private:
    AnalysisList*   m_cuts;	// list of cuts to employ in analysis

    std::vector<LayerGroup> m_layers;	// PSF analysis objects, for range of layers

    unsigned	    m_accepted;	// number passing analysis cuts

    static const Tuple* s_tuple;
    // reference to the Glast tuple


    static unsigned s_generated;    // number generated, from tuple title
    static double   s_area;	    //  cross-sectional area, from title
};
#endif
