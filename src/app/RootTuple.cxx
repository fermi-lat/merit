/** @file RootTuple.cxx
    @brief implement class RootTuple

 $Header: /nfs/slac/g/glast/ground/cvs/merit/src/app/RootTuple.cxx,v 1.10 2005/07/04 11:25:10 burnett Exp $
  Original author T. Burnett (w/ help from H. Kelley)
*/
#include "RootTuple.h"

// root includes
#include "TROOT.h"
#include "TFile.h"
#include "TBranch.h"
#include "TEventList.h"
#include "TTree.h"
#include "TSystem.h"
#include "TLeafF.h"
#include "TIterator.h"
#include "TKey.h"

#include "TFile.h"
#include "TDirectory.h"
#include "TTree.h"
#include "TKey.h"
#include "TIterator.h"
#include "TString.h"
namespace {
    // convenient utility from Heather
TTree* getTree(TFile *f) {
  // Create an iterator on the list of keys
  TIter nextTopLevelKey(f->GetListOfKeys());
  TKey *keyTopLevel, *curDirKey;
  TTree* t=0; // return 

  // loop on keys, and search for the TTree named "t1"
  while  ( keyTopLevel=(TKey*)nextTopLevelKey() ) {
    // I'm assuming we know the name of the TTree is "t1"
    TString name(keyTopLevel->GetName());
    TString className(keyTopLevel->GetClassName());

    if ((name.CompareTo("t1")==0) && (className.CompareTo("TTree")==0))  {
      // Found It
      t = (TTree*)f->Get(keyTopLevel->GetName());
      return t;
    }
    // If we find a directory - then we search it as well
    // Here I'm assuming that our directory structure only goes down one-level
    if (className.CompareTo("TDirectory")==0) {
      TDirectory *curDir = (TDirectory*)f->Get(name);
      TIter dirKeys(curDir->GetListOfKeys());
      while ( (curDirKey = (TKey*)dirKeys() ) ) {
        TString name(curDirKey->GetName());
        TString className(curDirKey->GetClassName());
        if ( (name.CompareTo("t1")==0) && (className.CompareTo("TTree")==0) ) {
          // Found it
          t = (TTree*)curDir->Get(curDirKey->GetName());
          return t;
        }
      }
    }
  }
  return t;
}

} // anonymous namespace

RootTuple::RootTuple(std::string title, std::string file, std::string treeName)
: Tuple(title), m_event(0) {

    // Initialize Root
    if ( 0 == gROOT )   {
        static TROOT meritRoot("root","ROOT I/O");
    } 
#ifdef WIN32
    int ret=gSystem->Load("libTree");
    if( ret==1) TTree dummy;

#endif
    
    // Open the file, and get at the  TTree containing the data
    m_file =  new TFile(file.c_str(), "read");
    if( m_file==0 ) {
        std::cerr << "file \""<< file << "\" not found." << std::endl;
        exit(1);
    }
    // first try the requested
    m_tree =  (TTree*)m_file->Get(treeName.c_str());
    // if doesn't work, try the old standby
    if( m_tree==0)  m_tree = (TTree*)m_file->Get("1");
   if( m_tree ==0 ) {
        std::cerr << "tree \""<<treeName<< "\" not found." << std::endl;
        m_file->ls();
        exit(1);
    }

    m_numEvents = m_tree->GetEntries();

#if 0 // save this for reference
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
#endif


}
void RootTuple::loadBranches()
{
    // load all the branches into our tuple
    TObjArray* ta = m_tree->GetListOfBranches();

    // now iterate.
    int entries = ta->GetEntries();
    for( int i = 0; i<entries; ++i) { // should try a TIter
        TBranch * b = (TBranch*)(*ta)[i];
        TLeafF* leaf = (TLeafF*)(*b->GetListOfLeaves())[0];
        const char * name = leaf->GetName();
        if( leaf->GetLenType()==4) {
            float * pf = (float*)leaf->GetValuePointer();
            new TupleItem(name,pf);
            m_float = true;
        }else{
            double* pf = (double*)leaf->GetValuePointer();
            new TupleItem(name,pf);
            m_float = false;
        }
    }
}

const TupleItem* RootTuple::tupleItem(const std::string& name)const
{
    Tuple::const_iterator it = find(name);
    if( it != end() ) return *it;


    TBranch* b = m_tree->GetBranch(name.c_str());
    if( b==0 ) {
        return 0;
    }
    TLeafF* leaf = (TLeafF*)(*b->GetListOfLeaves())[0];
    if( leaf->GetLenType()==4) {
        float * pf = (float*)leaf->GetValuePointer();
        return new TupleItem(name,pf);
    }else{
        double* pf = (double*)leaf->GetValuePointer();
        return new TupleItem(name,pf);
    }

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
}    
    
bool RootTuple::nextEvent(){
    if(m_event<m_numEvents) {
        m_tree->GetEvent(m_event++);
        m_file->cd();
        return true;
    }
    return false;
}

    
bool RootTuple::getEvent(int idx)
{
    if (idx >= 0 && idx < m_numEvents)
    {
        m_tree->GetEvent(idx);
        return true;
    }
    return false;
}