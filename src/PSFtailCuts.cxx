// $Header: /nfs/slac/g/glast/ground/cvs/merit/src/PSFtailCuts.cxx,v 1.2 2001/06/14 19:51:59 usher Exp $
// PSFtailCuts.cxx: implementation of the PSFtailCuts class.
//
//////////////////////////////////////////////////////////////////////

#include "PSFtailCuts.h"
#include "Cut.h"
#include <strstream>
#include <cmath>
//=============================================================================
class AbsValueCut : public Analyze {
    friend class PSFtailCuts;
    AbsValueCut(const Tuple& t, const std::string& name, double value)
        : Analyze(t,name)
        , m_value(value)
    {
        std::strstream label;
        label << "abs(" << name << ")<" << value << '\0';
        set_name(std::string(label.str()));
    }

    virtual bool apply() {
        return fabs(item()) < m_value;
    }
    double m_value;
};


//This function calculates the energy/angle dependent cut for FitKink and Tangle
static float kalError(float kalEne, int kplane, float cosz) 
{
	if (kalEne <0) return 1e6;

    //Convert from MeV to GeV for calculation
    kalEne *= 0.001;

    //Calculate the cut value depending on top or bottom
    float kalFit;

    if (kplane  < 12) kalFit = (2.8 /kalEne) + (2.1 /sqrt(kalEne));
    else              kalFit = kalFit = (6.35/kalEne) + (3.75/sqrt(kalEne));

    kalFit = - (0.001*kalFit) / cosz;

    return 3.5 * kalFit;
}


class FitKinkCut : public Analyze
{
    friend class PSFtailCuts;

    FitKinkCut(const Tuple& t) : Analyze(t, "TKR_Fit_Kink", "Tracker Fit Kink"),
                                 m_CalEne  (t.tupleItem("Cal_Energy_Deposit")),
                                 m_1stPlane(t.tupleItem("TKR_First_XHit")),
                                 m_cosTheta(t.tupleItem("TKR_Gamma_zdir"))
    { }

    virtual bool apply()
    {
        float fitKink = item();
        float calEne  = *m_CalEne;
        int   frstLyr = *m_1stPlane;
        float cosZ    = *m_cosTheta;

        return fabs(fitKink) <= kalError(calEne, frstLyr, cosZ);
    }

    const TupleItem* m_CalEne;
    const TupleItem* m_1stPlane;
    const TupleItem* m_cosTheta;
};

class TAngleCut : public Analyze
{
    friend class PSFtailCuts;

    TAngleCut(const Tuple& t) : Analyze(t, "TKR_t_angle", "Tracker t0-t1 angle"),
                                m_CalEne  (t.tupleItem("Cal_Energy_Deposit")),
                                m_1stPlane(t.tupleItem("TKR_First_XHit")),
                                m_cosTheta(t.tupleItem("TKR_Gamma_zdir"))
    { }

    virtual bool apply()
    {
        float tangle  = item();
        float calEne  = *m_CalEne;
        int   frstLyr = *m_1stPlane;
        float cosZ    = *m_cosTheta;

        return fabs(tangle) <= kalError(calEne, frstLyr, cosZ);
    }

    const TupleItem* m_CalEne;
    const TupleItem* m_1stPlane;
    const TupleItem* m_cosTheta;
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PSFtailCuts::PSFtailCuts(const Tuple& t): AnalysisList(" PSF tail cuts")
{

    push_back( new Cut(t, "TKR_Fit_Type>0") );
    //push_back( new AbsValueCut(t, "TKR_Fit_Kink" , 3*0.0041 ) );
   
    // allow either name for the opening angle difference
    //std::string tname="t0-t1_Angle";
    //if (t.find(tname)==t.end()) tname = "TKR_t_angle"; 
    //push_back( new Cut(t, tname, Cut::LT, 4*0.0041 ) );

    push_back( new FitKinkCut(t) );
    push_back( new TAngleCut(t)  );
}
