//$Header:  $
// Original author T. Burnett 
#ifndef MERITROOTTUPLE_H
#define MERITROOTTUPLE_H

#include <string>
class Tuple;
class TNtuple;
class TFile;

/* 
 * @brief Creates a ROOT Ntuple from the merit list
 *
 * @author T. Burnett
 *
 * $Header:  $
 */

class MeritRootTuple  {

public:
    /// define the associated root tuple and a file to write it too
    MeritRootTuple::MeritRootTuple(const Tuple* tuple, std::string filename);
    ~MeritRootTuple();

    /// fill it
    void fill();

    /// ask for number of entries
    int entries()const;

private:
    const Tuple* m_tuple;

    /// the ROOT stuff
    TNtuple  * m_tnt;
    TFile *  m_tf;

};

#endif