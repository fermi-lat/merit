// $Header: /cvs/glastsim/merit/MultiPSF.h,v 1.4 1999/04/20 17:01:46 pfkeb Exp $
//
#ifndef MULTIPSF_H
#define MULTIPSF_H

#include "Analyze.h"
#include <vector>

class PSFanalysis;
// Analyzed multiple bins in generated energy for PSF analysis

class MultiPSF : public Analyze , std::vector<PSFanalysis*>{
public:
    MultiPSF(const Tuple& t, char code);

    void report(std::ostream& out);
private:
    bool apply();
    double   m_bin_size; 
    std::vector<float>m_costheta_bin;
};

#endif
