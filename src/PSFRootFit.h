// PSFRootFit.h


#ifndef PSFRootFit_H
#define PSFRootFit_H

#include "PSFanalysis.h"

#include <iostream>
#include <iomanip>

//=============================================================================
class PSFRootFit {
// RootFit of the Point Spread Function
public:
    PSFRootFit(const PSFanalysis& psfanal);

    virtual void    report(std::ostream& out);
    /// formatted report

    /// do the fit
    double          rootFit(std::ostream& returnstr);

private:

    //analysis calling this class
    const PSFanalysis& m_psfanal;
};

#endif
