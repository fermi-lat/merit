// $Header: /nfs/slac/g/glast/ground/cvs/merit/src/PSFtailCuts.h,v 1.1.1.1 1999/12/20 22:29:13 burnett Exp $
// PSFtailCuts.h: interface for the PSFtailCuts class.
//
//////////////////////////////////////////////////////////////////////

#ifndef PSFTAILCUTS_H
#define PSFTAILCUTS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AnalysisList.h"

class PSFtailCuts : public AnalysisList  
{
public:
    PSFtailCuts(const Tuple& t);

};

#endif 
