//$Header: /nfs/slac/g/glast/ground/cvs/merit/src/meritAlg/MeritRootTuple.h,v 1.1 2002/08/31 21:55:20 burnett Exp $
// Original author T. Burnett 
#ifndef MERITROOTTUPLE_H
#define MERITROOTTUPLE_H

#include <string>
class Tuple;
class TNtuple;
class TFile;
class TTree;

/* 
 * @brief Creates a ROOT tree from the merit list
 *
 * @author T. Burnett
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/merit/src/meritAlg/MeritRootTuple.h,v 1.1 2002/08/31 21:55:20 burnett Exp $
 */

class MeritRootTuple  {

public:
    /// define the associated root tuple and a file to write it too
    MeritRootTuple::MeritRootTuple(Tuple* tuple, std::string filename);
    ~MeritRootTuple();

    /// fill it
    void fill();

    /// ask for number of entries
    int entries()const;

private:
    Tuple* m_tuple;

    /// the ROOT stuff
    //TNtuple  * m_tnt;
    TTree * m_tree;
    TFile *  m_tf;

};

#endif