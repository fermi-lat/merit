// EnergyAnalysis.cxx

#include "EnergyAnalysis.h"

#include "analysis/Tuple.h"
#include <iomanip>

using namespace std;

EnergyAnalysis::EnergyAnalysis(const Tuple& t)
: Analyze(t, "EvtEnergyCorr", //"TkrEnergyCorr",
          "Energy analysis")
, Histogram("Normalized Energy", 0.5, 1.5, 0.01)
, m_tuple(&t)
, m_McEnergy(t,"McEnergy", "MC energy")
{};

bool    EnergyAnalysis::apply()
{
    m_McEnergy(); //run apply
    double emeas = item(), egen= m_McEnergy.item();
    if(emeas>0 && egen>0) fill(emeas/egen); 
    return true;
}

void    EnergyAnalysis::report(ostream& out)
{
    //Analyze::separator(out);
    out << "\n" << Analyze::make_label("Energy: meas/gen")
        << setw(6) << setprecision(3) << mean()
        << "\n" << Analyze::make_label("             std")
        << setw(6) << setprecision(3) << stdDev()
        << "\n" << Analyze::make_label("events w/ no data")
        << setw(6) << (*this)[0] ;
    //m_McEnergy.report(out);
}
