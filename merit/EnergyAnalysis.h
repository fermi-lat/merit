// EnergyAnalysis.h

#ifndef ENERGYANALYSIS_H
#define ENERGYANALYSIS_H

#include "Analyze.h"
#include "Statistic.h"

#include "analysis/Histogram.h"


class EnergyAnalysis : public Analyze , public Histogram {
    // Accumualte and analyze the reconstructed energy
public:
    EnergyAnalysis(const Tuple& t);

    virtual void    report(std::ostream& out);

    float mean_generated()const{return m_MC_energy.stat().mean();}
private:
    virtual bool    apply();
    const Tuple*    m_tuple;
    Statistic m_MC_energy; // keep track of generated energy
};

#endif
