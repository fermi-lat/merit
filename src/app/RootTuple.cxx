//$Header:$
// Original author T. Burnett (w/ help from H. Kelley)
#include "RootTuple.h"

// root includes
#include "TROOT.h"
#include "TFile.h"
#include "TBranch.h"
#include "TEventList.h"
#include "TTree.h"
#include "TSystem.h"
#include "TLeafF.h"


RootTuple::RootTuple(std::string title, std::string file, std::string treeName)
: Tuple(title), m_event(0) {

    // Initialize Root
    if ( 0 == gROOT )   {
        static TROOT meritRoot("root","ROOT I/O");
        gSystem->Load("libTree.dll");
    } 
    
    // Open the file, and get at the  TTree containing the data
    TFile* tfile =  new TFile(file.c_str(), "read");
    tfile->ls();
    m_tree =  (TTree*)tfile->Get(treeName.c_str());
    if( m_tree ==0 ) return; // sorry

    // get the list of branches
    TObjArray* ta = m_tree->GetListOfBranches();

    // now iterate.
    int entries = ta->GetEntries();
    for( int i = 0; i<entries; ++i) { // should try a TIter
        TBranch * b = (TBranch*)(*ta)[i];
        TLeafF* leaf = (TLeafF*)(*b->GetListOfLeaves())[0];
        const char * name = leaf->GetName();
        float * pf = (float*)leaf->GetValuePointer();
    }

    // Determine the # of events stored in ntuple
    m_numEvents = m_tree->GetEntries();

}

const TupleItem* RootTuple::tupleItem(const std::string& name)const
{
    Tuple::const_iterator it = find(name);
    if( it != end() ) return *it;


    TBranch* b = m_tree->GetBranch(name.c_str());
    if( b==0 ) {
         std::cerr << "Sorry, did not find '" << name << "' in the tuple\n";
         exit(-1);
         return *it;
    }
    TLeafF* leaf = (TLeafF*)(*b->GetListOfLeaves())[0];
    const char * fname = leaf->GetName();
    float * pf = (float*)leaf->GetValuePointer();

    /*
    // setup the ntuple to access just this list of events
    myNtuple->SetEventList(list);
    
    myNtuple->SetEventList(0);  // will now be able to access all events again
    
    
    // iterate over all events
    for (i = 0; i < numEvents; i++) 
    {
        // setup L1T to contain the data in the L1T column of the ntuple
        Float_t L1T;
        TBranch* L1Tbranch = myNtuple->GetBranch("L1T");	
        L1Tbranch->SetAddress(&L1T);
        // retrieve the data for event i
        myNtuple->GetEvent(i);
    }
    */
    return new TupleItem(name,pf);
}    
    
bool RootTuple::nextEvent(){
    if(m_event<m_numEvents) {
        m_tree->GetEvent(m_event++);
        return true;
    }
    return false;
}