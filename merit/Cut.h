// Cut.h: interface for the Cut class.
//
//
// Original author: T. Burnett tburnett@u.washington.edu
// $Header: /cvs/glastsim/merit/Cut.h,v 1.5 1999/08/23 16:51:01 burnett Exp $
//////////////////////////////////////////////////////////////////////

#if !defined(CUT_H__INCLUDED_)
#define      CUT_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Analyze.h"


class Cut : public Analyze 
// 
// Purpose: define a simple cut 
{
public:

    enum Comparison{LT,GT,EQ,NE,LE,GE} ;

    Cut(const Tuple&t, std::string item_name, Comparison op, double c, std::string label="");
    // specify the tuple, item name in the tuple, operation, value to compare with, and 
    // an optional label--if it does not appear, a label is made automatically

    Cut(const Tuple&t, std::string::const_iterator& it, std::string::iterator end);
    // the tuple, and a expression to be parsed, of the form "item<99"

    Cut(const Tuple&t, const std::string& expression );
    // the tuple, and a expression to be parsed, of the form "item<99"

    virtual bool apply();

private:
    void parse(const Tuple&t, std::string::const_iterator& it,  std::string::const_iterator end);

    double m_cut;
    Comparison m_op;
};

#endif // !defined(AFX_ANALYSISCUT_H__CD2E7DD1_F406_11D2_83A8_000000000000__INCLUDED_)
;