
#ifndef TKRVTXCUTVALS_H
#define TKRVTXCUTVALS_H

#include "Event/Recon/TkrRecon/TkrVertex.h"
#include "Event/Recon/TkrRecon/TkrFitTrackBase.h"
#include <vector>

/** 
* @class TkrVtxCutVals
*
* @brief A utility class for calculating PDR era "cut" variables from the output
*        of TkrRecon. Divides the (presumed) two tracks in the gamma into a "best" and a
*        "pair" track, following the analogy of the PDR era analysis. Then calculates/stores
*        the primary quantities used in further analysis (e.g. by merit). 
*        Currently, this includes:
*        Quality  - Reconstructed track quality 
*        T Angle  - angle between the "best" gamma track and the reconstructed gamma direction
*        Fit Kink - The largest of the the difference in slopes between the first two segments
*                   of a reconstructed track in either the X or Y projection
*
* @author The Tracking Software Group
*
* $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease/merit/src/meritAlg/TkrVtxCutVals.h,v 1.1 2002/08/29 00:53:00 burnett Exp $
*/

class TkrVtxCutVals
{
public:
    // Constructor needs fit track collection and a reconstructed vertex
    TkrVtxCutVals(const Event::TkrVertex* vtx);
   ~TkrVtxCutVals();

    // Return the gamma direction
   Vector                          getVertexDir()   {return vertex->getDirection();}

    // Access methods for "best" and "pair" tracks
    const Event::TkrFitTrackBase*  getBestTrack()   {return best;}
    const Event::TkrFitTrackBase*  getPairTrack()   {return pair;}
    double                         getBestQuality() {return bestTrkQual;}
    double                         getPairQuality() {return pairTrkQual;}

    // Global event cut variables
    double                         getTangle()      {return tangle;}
    double                         getFitKink()     {return fitKink;}


private:
    // Functions
    void                           calcTangle();
    void                           calcFitKink();

    // Data members
    const Event::TkrVertex*        vertex;

    const Event::TkrFitTrackBase*  best;
    double                         bestTrkQual;
    const Event::TkrFitTrackBase*  pair;
    double                         pairTrkQual;

    int                            numTracks;
    double                         tangle;
    double                         fitKink;
};

#endif