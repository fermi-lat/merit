// This intended to be a root macro for very basic MC - Tracking comparisons
// Tracy Usher June 14, 2002

#include "TkrVtxCutVals.h"
#include "geometry/Vector.h"

#include <cmath>

TkrVtxCutVals::TkrVtxCutVals(const Event::TkrVertex* vtx) : vertex(vtx)
{
    numTracks   = const_cast<Event::TkrVertex*>(vertex)->getNumTracks();

    best        = 0;
    pair        = 0;
    bestTrkQual = 0.;
    pairTrkQual = 0;

    fitKink     = -9999.;
    tangle      = -9999.;

    SmartRefVector<Event::TkrFitTrack>::const_iterator trackIter = vertex->getTrackIterBegin();

    for(int idx = 0; idx < numTracks; idx++)
    {
        const Event::TkrFitTrack* track = *trackIter++;

        if (track->getQuality() > bestTrkQual)
        {
            best = track;
            bestTrkQual = track->getQuality();
        }
    }


    if (best)
    {
        if (numTracks > 1)
        {
            pairTrkQual = -1.;
            trackIter   = vertex->getTrackIterBegin();

            for (idx = 0; idx < numTracks; idx++)
            {
                const Event::TkrFitTrack* track = *trackIter++;

                if (track->getQuality() > pairTrkQual && track->getQuality() < bestTrkQual)
                {
                    pair = track;
                    pairTrkQual = track->getQuality();
                }
            }
        }

        calcTangle();
        calcFitKink();
    }

    return;
}

TkrVtxCutVals::~TkrVtxCutVals()
{
}

// Calculate the infamous t angle
void TkrVtxCutVals::calcTangle()
{
    Vector           vtxDir    = vertex->getDirection();
    Event::TkrFitPar trkPar    = best->getTrackPar(Event::TkrRecInfo::Start);
    Vector           trkDir    = Vector(-trkPar.getXSlope(),-trkPar.getYSlope(),-1.).unit();
    double           cosVtxTrk = trkDir * vtxDir;

    if (cosVtxTrk > 1.) cosVtxTrk = 1.;

    tangle = sqrt(1. - cosVtxTrk*cosVtxTrk);

    return;
}

// Calculate the Fit Kink
void TkrVtxCutVals::calcFitKink()
{
    Event::TkrFitPlaneConPtr hitIter = best->getHitIterBegin();

    Event::TkrFitHit firstHit  = hitIter[0].getHit(Event::TkrFitHit::SMOOTH);
    Event::TkrFitHit secondHit = hitIter[1].getHit(Event::TkrFitHit::SMOOTH);

    double           xKink     = secondHit.getPar().getXSlope() - firstHit.getPar().getXSlope();
    double           yKink     = secondHit.getPar().getYSlope() - firstHit.getPar().getYSlope();

    fitKink = (fabs(yKink) > fabs(xKink)? yKink : xKink);
    
    return;
}
