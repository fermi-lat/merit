#include "MeritRootTuple.h"

#include "analysis/Tuple.h"

// root includes
#include "TNtuple.h"
#include "TFile.h"

#include <vector>
#include <string>
#include <cassert>

#ifndef DEFECT_NO_STRINGSTREAM
# include <sstream>
#else
# include <strstream>
#endif


MeritRootTuple::MeritRootTuple(const Tuple* tuple, std::string filename)
: m_tuple(tuple)
{
#ifndef DEFECT_NO_STRINGSTREAM
    std::stringstream list;
#else
    std::strstream list;
#endif

    bool first = true;
    for(Tuple::const_iterator tit =m_tuple->begin(); tit != m_tuple->end(); ++tit){
        const TupleItem& item = **tit;
        if( first ) {  first=false;}else list << ":";
        list << item.name();
    }
    std::string namestring=list.str();
    const char * names = namestring.c_str();
    static char buffer[1024];
    ::strncpy(buffer, names, sizeof(buffer));

    m_tf = new TFile(filename.c_str(),"RECREATE");
    m_tnt = new TNtuple("MeritTuple", m_tuple->title().c_str(), buffer);
    assert( tuple->size() == m_tnt->GetNvar() ); // must be the same
}

void MeritRootTuple::fill()
{
    std::vector<float> floatarray(m_tuple->size());

    std::vector<float>::iterator fit = floatarray.begin();

    for(Tuple::const_iterator tit =m_tuple->begin(); tit != m_tuple->end(); ++tit){
        const TupleItem& item = **tit;
        *fit++ = float(item);
    }

    const Float_t* x = floatarray.begin();
    m_tnt->Fill(x);
}

int MeritRootTuple::entries() const { 
    return m_tnt->GetEntries();
}

MeritRootTuple::~MeritRootTuple()
{
    m_tf->Write();
#if 0 // crashes ROOT???
    delete m_tf;
    delete m_tnt;
#endif
}
