// Level1.cxx: implementation of the Level1 class.
//
// Author: T. Burnett, tburnett@u.washington.edu
// $Id: Level1.cxx,v 1.1 1999/07/04 04:06:05 burnett Exp $
//////////////////////////////////////////////////////////////////////

#include "Level1.h"

#include <iostream>
#include <iomanip>

using namespace std;

Level1::Level1(const Tuple&t , bool useACD)
: Analyze(t, "Trig_Bits", useACD? "L1T w ACD": " Level 1")
, m_useACD(useACD)
{
    clear();
}


bool Level1::apply () 
{
    int bits = static_cast<unsigned>(item());
    m_counts[bits] = m_counts[bits]+1;

    int trigger =  bits & (4|32|64);
    if( trigger & 4) m_track++;
    if( trigger & 64) m_cal++;
    if( trigger & 32) m_hi_cal++;
    if( (trigger &(4|64))==(4|64)) m_both++;
    
    if( !m_useACD) return (trigger!=0);

    // basic ACD
     return ((bits & 32)!=0 )  // always if HICAL
       || (bits &(4+64))!=0  // track or low cal
          && (bits&(2+16))==0;   // but not if nACD>2 or ACD track veto
} 

void Level1::report(ostream& out)
{
    out << endl << name();
    out << endl << "\t track: " << setw(8) << m_track;
    out << endl << "\t   cal: " << setw(8) << m_cal;
    out << endl << "\thi cal: " << setw(8) << m_hi_cal;
    out << endl << "\t  both: " << setw(8) << m_both;
    out << endl << " bit frequency table";
    out << endl << setw(6) << "value"<< setw(6) << "count" ;
    int j, grand_total=0;
    for(j=0; j<7; ++j) out << setw(6) << (1<<j);
    out << endl << setw(6) <<" "<< setw(6) << "------"; 
    for( j=0; j<7; ++j) out << setw(6) << "-----";
    vector<int>total(7);
    for( map<int,int>::const_iterator it = m_counts.begin(); it != m_counts.end(); ++it){
        int i = (*it).first, n = (*it).second;
        grand_total += n;
        out << endl << setw(6)<< i << setw(6)<< n ;
        for(j=0; j<7; ++j){
            int m = ((i&(1<<j))!=0)? n :0;
            total[j] += m;
            out << setw(6) << m ;
        }
    }
    out << endl << setw(6) <<" "<< setw(6) << "------"; 
    for( j=0; j<7; ++j) out << setw(6) << "-----";
    out << endl << setw(6) << "tot:" << setw(6)<< grand_total;
    for( j=0; j<7; ++j) out << setw(6) << total[j];

    Analyze::report(out);
    separator(out);
    
}
