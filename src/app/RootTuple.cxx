//$Header: /nfs/slac/g/glast/ground/cvs/merit/src/app/RootTuple.cxx,v 1.4 2001/12/18 23:28:30 usher Exp $
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
    gSystem->Load("libTree.dll");
#endif
    
    // Open the file, and get at the  TTree containing the data
    TFile* tfile =  new TFile(file.c_str(), "read");
    if( tfile==0 ) {
        std::cerr << "file \""<< file << "\" not found." << std::endl;
        exit(1);
    }
    m_tree =  (TTree*)tfile->Get(treeName.c_str());
    if( m_tree==0)
        m_tree = getTree(tfile);
   if( m_tree ==0 ) {
        std::cerr << "tree \""<<treeName<< "\" not found." << std::endl;
        tfile->ls();
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

    
bool RootTuple::getEvent(int idx)
{
    if (idx >= 0 && idx < m_numEvents)
    {
        m_tree->GetEvent(idx);
        return true;
    }
    return false;
}