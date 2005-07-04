// $Header: /nfs/slac/g/glast/ground/cvs/merit/src/PSFtailCuts.cxx,v 1.12 2004/09/08 20:29:25 burnett Exp $
// PSFtailCuts.cxx: implementation of the PSFtailCuts class.
//
//////////////////////////////////////////////////////////////////////

#include "PSFtailCuts.h"
#include "Cut.h"
#include <sstream>
#include <cmath>
//=============================================================================
class AbsValueCut : public Analyze {
    friend class PSFtailCuts;
    AbsValueCut(const Tuple& t, const std::string& name, double value)
        : Analyze(t,name)
        , m_value(value)
    {
        std::stringstream label;
        label << "abs(" << name << ")<" << value;
        set_name(label.str());
    }

    virtual bool apply() {
        return fabs(item()) < m_value;
    }
    double m_value;
};


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PSFtailCuts::PSFtailCuts(const Tuple& t): AnalysisList("CT PSF tail cuts")
{
    push_back( new Cut(t, "CTgoodCal>0.25") );
    push_back( new Cut(t, "Tkr1Zdir<-0.25") );
}
