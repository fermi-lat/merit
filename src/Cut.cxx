// Cut.cxx: implementation of the Cut class.
//
// Original author: T. Burnett tburnett@u.washington.edu
// $Header: /nfs/slac/g/glast/ground/cvs/merit/src/Cut.cxx,v 1.2 2002/05/31 19:37:49 burnett Exp $
//////////////////////////////////////////////////////////////////////

#include "Cut.h"

#include <cstdio>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Cut::Cut(const Tuple&t,std::string item_name, Comparison op, double cut, std::string label) 
:Analyze(t,item_name,label),m_cut(cut),m_op(op)
{

    if( label.empty()) {
        std::string u;
        switch(op) {
        case LT: u = "<" ;break;
        case GT: u = ">" ;break;
        case EQ: u = "=" ;break; 
        case NE: u = "!=" ;break;
        case LE: u = "<=" ;break;
        case GE: u = ">=" ;break;
        }
        static char buf[20]; 
        sprintf(buf,"%g",m_cut);
        
        set_name(item_name+u+buf);
    }
}
Cut::Cut(const Tuple&t, const std::string& expression)
{
    std::string::const_iterator b(expression.begin());
    parse(t, b, expression.end() );
}

Cut::Cut(const Tuple&t, std::string::const_iterator& it, std::string::iterator end)
{
    parse(t, it, end);
    if( it != end && *it==')' ) it++;  // needed for caller
}

void Cut::parse(const Tuple&t, std::string::const_iterator& it, std::string::const_iterator end)
{
    std::string::const_iterator begin = it; // save first
    // parse the expression: expect name OP value
    std::string name;
    std::string value;
    bool first=true;
    for(;   it != end && *it!=')'; ++it) {
        if( first ) {
            if( isalnum(*it) || *it=='_' )name += *it;
            else { 
                switch( *it ) {
                case '<': if( *(it+1)=='=' ) {m_op=LE; ++it;} else m_op=LT; break;
                case '>': if( *(it+1)=='=' ) {m_op=GE; ++it;} else m_op=GT; break;
                case '=': m_op=EQ; if(*(it+1)=='=')++it; break;
                case '!': m_op=NE; ++it; break; // assume followed by =
                default: std::cerr << "Unexpected character '" << *it << "' found" << std::endl;
                    throw("Cut::parse: unexpected character");
                }
                first = false;
            }
        } else {
            value+= *it;
        }
    }
    m_cut = atof(value.c_str());
    set_tuple_item(t,name);
#if defined(__GNUC__) && ( __GNUC__ >= 3 ) // think the followingis right: need to test it
    set_name(std::string(begin,--it));
#else // this should be ok??? 
    set_name(std::string(begin,it-begin));  
#endif
}

bool Cut::apply()
{
    switch(m_op) {
    case LT: return item()< m_cut ;break;
    case GT: return item()> m_cut ;break;
    case EQ: return item()==m_cut ;break; 
    case NE: return item()!=m_cut ;break;
    case LE: return item()<=m_cut ;break;
    case GE: return item()>=m_cut ;break;
    default: return false;
    }
}

