// Level1.h: interface for the Level1 class.
//
// Author: T. Burnett, tburnett@u.washington.edu
// $Id: Level1.h,v 1.1 1999/07/04 04:06:06 burnett Exp $
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LEVEL1_H__FDC684B1_2BDF_11D3_842C_006008B7A02D__INCLUDED_)
#define AFX_LEVEL1_H__FDC684B1_2BDF_11D3_842C_006008B7A02D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#pragma warning(disable:4786)
#endif // _MSC_VER > 1000

#include "Analyze.h"
#include <map>

class Level1 : public Analyze {
public:
    explicit Level1(const Tuple&t, bool useACD=false);
    // constructor, needs a tuple, can specify to use ACD

    void report(std::ostream& out);

    void clear(){m_track=m_cal=m_both=m_hi_cal=0; Analyze::clear();}
private:
    virtual bool apply (); 

    int m_track;
    int m_cal;
    int m_hi_cal;
    int m_both;
    std::map<int,int> m_counts; //map of values for each bit pattern
    bool m_useACD;
};

#endif // !defined(AFX_LEVEL1_H__FDC684B1_2BDF_11D3_842C_006008B7A02D__INCLUDED_)
