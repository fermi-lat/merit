// RebinHist.cxx

#include "analysis/RebinHist.h"

#include <algorithm>

using namespace std;

RebinHist::RebinHist(const char * title, double from, double to, double step)
:Histogram(title, from, to, step)
{
    m_data.reserve(500);
}

void RebinHist::fill(double x)
{
    Histogram::fill(x,1.0);
    m_data.push_back(x);
}

void RebinHist::rebin(double from, double to, double step )
{
    setRange(from, to ,step);
    FloatList::iterator dat = m_data.begin();
    for(; dat !=m_data.end(); Histogram::fill(*dat++) );
}

double RebinHist::percentile(double level)
{
    sort(m_data.begin(), m_data.end());
    int n = m_data.size();
    int m =  static_cast<int>(n * level/100.);
    if( m > n-1) m = n-1;
    return m_data[m];
}
