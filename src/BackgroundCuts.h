// $Header: /nfs/slac/g/glast/ground/cvs/merit/src/BackgroundCuts.h,v 1.1.1.1 1999/12/20 22:29:12 burnett Exp $
// Initial author Steve Ritz

#ifndef BackgroundCuts_H
#define BackgroundCuts_H
#include "AnalysisList.h"

class BackgroundCuts : public AnalysisList {
public:
    BackgroundCuts::BackgroundCuts(const Tuple& t);
};

#endif //BackgroundCuts_H
