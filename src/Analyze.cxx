// Analyze.cxx

#include "Analyze.h"

#include <iomanip>
using namespace std;

bool    Analyze::s_showperc = false;    // show percentages in readout flag

//=============================================================================

Analyze::Analyze(const Tuple& t, const string& itemname, const string& name)
: m_count(0),m_tupleItem(t.tupleItem(itemname)), m_name(name), m_seen(0) {}

Analyze::Analyze(const TupleItem& item, const string& name)
: m_count(0), m_tupleItem(&item), m_name(name), m_seen(0) {}

void Analyze::report(ostream& out)
{
    out << endl << make_label(name());    
    out << setw(6) << count();
    if (showperc()) {
        float   cnt = static_cast<float>(count()), sen = static_cast<float>(seen());
        out << "   |   " << setprecision(3) << ((sen == 0.) ? 0. : 100.*cnt/sen) << '%';
    }
}

string Analyze::make_label(const string& in)
{
    const unsigned col = 24;
    string	namestr(in);
    if (namestr.length() < col)  namestr.insert(namestr.begin(),col - namestr.length(),' ');
    namestr += " : ";
    return  namestr ;
}

void Analyze::separator(ostream& out, char s)
{
    out << "\n" << string(55, s );
}

void Analyze::set_tuple_item(const Tuple& t, std::string item_name)
{
    m_tupleItem = t.tupleItem(item_name);
}

const char* Analyze::getItem()
{
    const char* pItem = 0;

    if (m_tupleItem)
    {
        std::string item(m_tupleItem->name());
        pItem = item.data();
    }

    return pItem;
}