#include "MeritRootTuple.h"

#include "analysis/Tuple.h"

// root includes
#include "TTree.h"
#include "TFile.h"



MeritRootTuple::MeritRootTuple(Tuple* tuple, std::string filename)
: m_tuple(tuple)
{

    m_tf = new TFile(filename.c_str(),"RECREATE");
    float t=0;

    m_tree = new TTree("MeritTuple", m_tuple->title().c_str());
    m_floats.resize(m_tuple->size());
    std::vector<float>::iterator fit = m_floats.begin();
    for(Tuple::iterator tit =m_tuple->begin(); tit != m_tuple->end(); ++tit, ++fit ){
        TupleItem& item = **tit;
        float * d =  *&fit;
        m_tree->Branch(item.name().c_str(), d, item.name().c_str());
    }
}

void MeritRootTuple::fill()
{
    // go through the tuple, put doubles into the array for the tree.
    std::vector<float>::iterator fit = m_floats.begin();
    for(Tuple::iterator tit =m_tuple->begin(); tit != m_tuple->end(); ++tit, ++fit){
        TupleItem& item = **tit;
        *fit = static_cast<float>(item.value());
    }
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
