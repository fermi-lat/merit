// $Header: /cvs/glastsim/merit/PSFtailCuts.cxx,v 1.2 1999/09/06 12:10:39 burnett Exp $
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

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PSFtailCuts::PSFtailCuts(const Tuple& t): AnalysisList(" PSF tail cuts")
{

    push_back( new Cut(t, "Fit_Type>0") );
    push_back( new AbsValueCut(t, "Fit_Kink" , 3*0.0041 ) );
   
    // allow either name for the opening angle difference
    std::string tname="t0-t1_Angle";
    if (t.find(tname)==t.end()) tname = "t_Angle"; 
    push_back( new Cut(t, tname, Cut::LT, 4*0.0041 ) );
}
