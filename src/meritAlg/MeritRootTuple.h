/** @file MeritRootTuple.h
@brief Declaration of the class MeritRootTuple

  $Header: /nfs/slac/g/glast/ground/cvs/merit/src/meritAlg/MeritRootTuple.h,v 1.4 2003/03/15 03:40:32 burnett Exp $
  
    Original author T. Burnett 
    
*/  
#ifndef MERITROOTTUPLE_H
#define MERITROOTTUPLE_H

#include <string>
#include <vector>
class Tuple;
class TFile;
class TTree;

/* 
* @brief Creates a ROOT tree from the merit list
*
* @author T. Burnett
*
* $Header: /nfs/slac/g/glast/ground/cvs/merit/src/meritAlg/MeritRootTuple.h,v 1.4 2003/03/15 03:40:32 burnett Exp $
*/

class MeritRootTuple  {
    
public:
    /// define the associated root tuple and a file to write it to
    MeritRootTuple::MeritRootTuple(Tuple* tuple, std::string filename);
    ~MeritRootTuple();
    
    /// fill it
    void fill();
    
    /// ask for number of entries
    int entries()const;
    
private:
    Tuple* m_tuple;
    
    /// the ROOT stuff: a file and a tree to put into it
    TTree * m_tree;
    TFile *  m_tf;
    
    
    std::vector<float> m_floats; // needed for communication with ROOT's float branches.
    
};

#endif