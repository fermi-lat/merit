#include "MeritRootTuple.h"

#include "analysis/Tuple.h"

// root includes
#include "TTree.h"
#include "TFile.h"



MeritRootTuple::MeritRootTuple(Tuple* tuple, std::string filename)
: m_tuple(tuple)
{

    m_tf = new TFile(filename.c_str(),"RECREATE");

    m_tree = new TTree("MeritTuple", m_tuple->title().c_str());
    for(Tuple::iterator tit =m_tuple->begin(); tit != m_tuple->end(); ++tit){
        TupleItem& item = **tit;
        m_tree->Branch(item.name().c_str(), &item.value(), item.name().c_str());
    }
}

void MeritRootTuple::fill()
{
    m_tree->Fill();
}

int MeritRootTuple::entries() const { 
    return m_tree->GetEntries();
}

MeritRootTuple::~MeritRootTuple()
{
    m_tree->Write();
#if 0 // crashes ROOT???
    delete m_tf;
    delete m_tree;
#endif
}
