// $Header: /nfs/slac/g/glast/ground/cvs/merit/src/BackgroundCuts.cxx,v 1.1.1.1 1999/12/20 22:29:12 burnett Exp $
// Initial author Steve Ritz

#include "BackgroundCuts.h"
#include "Cut.h"
//=============================================================================
//  Helper classes applying complicated cuts
//=============================================================================
class Cal2Dfrac : public Analyze {
friend class BackgroundCuts;

    Cal2Dfrac (const Tuple& t) 
        : Analyze (t, "Cal_eLayer8", "Cal2Dfrac")
        , m_csi1(t.tupleItem ("Cal_eLayer1"))
        , m_csi8(t.tupleItem ("Cal_eLayer8"))
	  , m_csiESum(t.tupleItem ("Cal_Energy_Sum"))
    {    }

    virtual bool    apply () {
        float lyr8 = item();
	  float lyr1 = *m_csi1;
        float sum = *m_csiESum;

        return (sum != 0.) ? (lyr8/sum<0.08 || lyr1/sum>0.25 || sum>0.35) : false;
    }
    
    const TupleItem*	m_csi1;
    const TupleItem*	m_csi8;
    const TupleItem*	m_csiESum;
};
    
//=============================================================================
class SurplusHit : public Analyze {
    friend class BackgroundCuts;

    SurplusHit (const Tuple& t) 
        : Analyze (t, "REC_Surplus_Hit_Ratio", "Surplus_Hit_Ratio")
        , m_fstX(t.tupleItem ("TKR_Fst_Cnv_Lyr"))
        , m_SHR(t.tupleItem ("REC_Surplus_Hit_Ratio"))
	  , m_csiESum(t.tupleItem ("Cal_Energy_Sum"))
    {    }

    virtual bool    apply () {
        float SHR = item();
	  float fstX = *m_fstX;
        float sum = *m_csiESum;
// we'll adjust this further for higher energy.  Below 10 GeV this works fine, thanks to MS!
        return (SHR>2.25 || (sum>1.&&fstX>13) || sum>5.);
    }
    
    const TupleItem*	m_fstX;
    const TupleItem*	m_SHR;
    const TupleItem*	m_csiESum;
};
    

//=============================================================================
class CalFitNrm : public Analyze {
    friend class BackgroundCuts;
    CalFitNrm (const Tuple& t) 
        : Analyze (t, "Cal_Fit_errNrm", "CalFitNrm")
	  , m_csiESum(t.tupleItem ("Cal_Energy_Sum"))
    {    }

    virtual bool    apply () {
        float fiterr = item();
        float sum = *m_csiESum;
//    we do this because the normalization is bad at low energy
        return (sum != 0.) ? (sum<1. && fiterr<10. || fiterr<4.) : false;
    }
    
    const TupleItem*	m_csiESum;
};
//=============================================================================
class NumVetos : public Analyze {
/*  Real gammas will seldom light up more than one ACD tile, whereas cosmics that 
    enter and exit will.  We eventually want a more sophisticated analysis that
    uses tracker hit cluster information in addition to the spatial information
    defined by the tiles.  However, a first look at such events indicates that 
    such an analysis will take time to do properly, since our tracker is not 
    designed to recognize tracks approximately parallel to the detector planes.  
    We should worry about environmental effects on-orbit causing ACD tiles to 
    light up randomly at a high enough rate to cause an inefficiency.  A first 
    estimate by Jonathan Ormes indicates that soft particle fluxes are not a 
    problem here, even during a GRB.  Note that the energy-dependent cuts are 
    very far away from any gamma event in the simulation. 
*/
    friend class BackgroundCuts;
    
    NumVetos (const Tuple& t) 
        : Analyze (t, "No_Vetos_Hit", "NumVetos")
	  , m_csiESum(t.tupleItem ("Cal_Energy_Sum"))
    {    }

    virtual bool    apply () {
        float num = item();
        float sum = *m_csiESum;
// worry about real-world effects, but these are fairly loose
        return (num<1.5 || (sum>1. && num<7.) || sum>50.);
    }
    
    const TupleItem*	m_csiESum;
};
//=============================================================================
class CutWithEthresh : public Cut {
    // only apply a cut if E< thresh
    friend class BackgroundCuts;
    CutWithEthresh(const Tuple& t, const std::string& expression, double thresh)
        : Cut(t,expression)
        , m_csiESum(t.tupleItem ("Cal_Energy_Sum"))
        , m_thresh(thresh) {}

    virtual bool apply() {
        float Esum = *m_csiESum;
        return Esum > m_thresh || Cut::apply();
    }
    const TupleItem* m_csiESum;
    double m_thresh;
};   

//=============================================================================


BackgroundCuts::BackgroundCuts(const Tuple& t) : AnalysisList(" Ritz cuts")
{
    
    push_back( new Cut(t, "TKR_No_Tracks",      Cut::GT,  0,  "track found") );
    push_back( new Cut(t, "Cal_Xtal_Ratio>0.25" ) );
    
    push_back( new SurplusHit (t) );
    push_back( new CalFitNrm (t) );
    push_back( new NumVetos (t) );
    push_back( new Cal2Dfrac (t) );
    
    push_back( new CutWithEthresh(t, "Cal_moment1<15.", 0.35 ) );
    push_back( new Cut(t, "Cal_Z>-30.") ); // will loosen slightly above 75 GeV, but still quite efficient even at 300 GeV
    push_back( new Cut(t, "Quality_Parm>10.") );
    push_back( new CutWithEthresh(t, "Cal_No_Xtals_Trunc<20", 75.) );

}
