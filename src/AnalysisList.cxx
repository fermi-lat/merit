// AnalysisList.cxx

#include "AnalysisList.h"

using namespace std;

AnalysisList::AnalysisList(AnalysisList& a) : Analyze(a)
{
	const_iterator it = a.begin();
	while (it != a.end()) {
		push_back(new Analyze(**it));
		it++;
	}
}

bool AnalysisList::apply()
{
    iterator it = begin();
    bool last = false;
    while( it !=end() && ( last=(**it++)() ) );
    return last;
}

void AnalysisList::report(ostream& out)
{
    if( !name().empty() ) {
        	out << endl << name();
    }
    for( iterator it = begin(); it !=end() ; ++it)
	(*it)->report(out);
    if(  ! m_noline) separator(out); // draw separator unless 
}
void AnalysisList::clear()
{
    Analyze::clear();
    for( iterator it = begin(); it !=end() ; ++it)
	(*it)->clear();
}
