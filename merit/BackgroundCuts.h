// $Header: /cvs/glastsim/merit/BackgroundCuts.h,v 1.1 1999/08/23 21:14:38 burnett Exp $
// Initial author Steve Ritz

#ifndef BackgroundCuts_H
#define BackgroundCuts_H
#include "AnalysisList.h"

class BackgroundCuts : public AnalysisList {
public:
    BackgroundCuts::BackgroundCuts(const Tuple& t);
};

#endif //BackgroundCuts_H