// Statistic.cxx
// $Id: Statistic.cxx,v 1.1.1.1 1999/12/20 22:29:13 burnett Exp $
#include "Statistic.h"

#include <iomanip>

using namespace std;

Statistic::Statistic(const Tuple&t, std::string::const_iterator& it, std::string::iterator end)
{
    std::string::const_iterator begin = it; // save first
    // parse the expression: expect name,
    std::string name;
    for(;   it != end && *it!=','; ++it) name += *it;
    set_tuple_item(t,name);
    set_name(std::string(begin,it));
    if( *it==',')++it;
}


void    Statistic::report(ostream& out)
{
    Analyze::separator(out);
    double mean, rms;
    out	<< "\n" << Analyze::make_label(name()+"--mean")
		<< std::setw(6) <<  (mean=m_stat.mean());
    out	<< "\n" << Analyze::make_label("rms")
		<< std::setw(6) <<  (rms=m_stat.stdDev());
    out	<< "\n" << Analyze::make_label("min")
		<< std::setw(6) <<  m_stat.min();
    if( rms>1e-6*mean) {
        out << " (=" << std::setprecision(3) 
        << (m_stat.min()-mean)/rms << " std)";
         out << std::setprecision(6);
    }

    out	<< "\n" << Analyze::make_label("max")
		<< std::setw(6) <<  m_stat.max() ;
    if( rms>1e-6*mean){
        out <<  " (=" << std::setprecision(3) 
        << (m_stat.max()-mean)/rms << " std)";
        out << std::setprecision(6);
    }
}
