// RebinHist.h

#ifndef REBINHIST_H
#define REBINHIST_H

#include "analysis/Histogram.h"

#include <vector>

class RebinHist : public Histogram {
public:
    RebinHist(const char * title="default", double from=0, double to=1, double step=0.1);
    void rebin(double from=0, double to=1, double step=0.1);
    void fill(double d);
    double percentile(double level); // value of entry

    //This done for use with root macros
    float getDataVal(int i) {return m_data[i];}
private:
    typedef std::vector<float> FloatList;
    FloatList m_data;
};

#endif
