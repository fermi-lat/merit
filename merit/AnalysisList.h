// AnalysisList

#ifndef ANALYSISLIST_H
#define ANALYSISLIST_H

#include "Analyze.h"

#include <iostream>

class AnalysisList : public Analyze , public std::vector<Analyze*>{
public:
    AnalysisList(std::string name="AnalysisList", bool noline=false)
        : Analyze(name), m_noline(noline){}
	AnalysisList(AnalysisList&);

    void report(std::ostream& out);

    void clear();

private:
    bool apply();
    bool m_noline;
};
#endif
