/** @file FigureOfMerit.h
    @brief Definition of class FigureOfMerit

   $Header: /nfs/slac/g/glast/ground/cvs/merit/src/FigureOfMerit.h,v 1.5 2002/05/26 03:35:08 burnett Exp $

*/

#ifndef FIGUREOFMERIT_H
#define FIGUREOFMERIT_H


#include "LayerGroup.h"
#include "PSFanalysis.h"
#include "EnergyAnalysis.h"
#include "AnalysisList.h"
#include <vector>


//====================================================================================
/** @class FigureOfMerit


Analyzes the glast tuple to determine a figure of merit that is relevant
 for the point source discovery potential of the instrument. For a gaussian
 point spread function it is
    sqrt(Aeff)/sigma

*/
class FigureOfMerit
{
public:
    FigureOfMerit(const Tuple& t, std::string cutstring="");

    //! associate a sequence of cuts, identifed by characters in the string
    void            setCuts(std::string);

    //! analyze the current tuple row
    bool            execute();

    //! write results, return acceptacnce
    void            report(std::ostream&);

    void            accept();

    //! process an accepted event
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
