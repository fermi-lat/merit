// $Header: /nfs/slac/g/glast/ground/cvs/merit/src/PSFtailCuts.cxx,v 1.6 2003/05/08 15:46:39 burnett Exp $
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
    push_back( new Cut(t, "IMgoodCalProb>0.5") );
    push_back( new Cut(t, "IMcoreProb>0.1") );
}
