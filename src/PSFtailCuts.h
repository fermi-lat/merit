// $Header: /cvs/glastsim/merit/PSFtailCuts.h,v 1.1 1999/08/27 20:14:25 burnett Exp $
// PSFtailCuts.h: interface for the PSFtailCuts class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PSFTAILCUTS_H__D65E62E1_2A00_4F86_8067_192F4F821C66__INCLUDED_)
#define AFX_PSFTAILCUTS_H__D65E62E1_2A00_4F86_8067_192F4F821C66__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AnalysisList.h"

class PSFtailCuts : public AnalysisList  
{
public:
    PSFtailCuts(const Tuple& t);

};

#endif // !defined(AFX_PSFTAILCUTS_H__D65E62E1_2A00_4F86_8067_192F4F821C66__INCLUDED_)
