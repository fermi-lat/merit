// $Header: /nfs/slac/g/glast/ground/cvs/merit/src/meritAlg/GaudiTuple.cxx,v 1.1 2001/03/23 19:52:02 burnett Exp $
// Original author: T. Burnett

#include "GaudiTuple.h"
#include "Gaudi/Interfaces/INTuple.h"
#include "Gaudi/NTupleSvc/NTuple.h"

GaudiTuple::GaudiTuple(const std::string& title, NTuplePtr* nt)
: Tuple(title)
, m_nt(nt)
{}

GaudiTuple::~GaudiTuple()
{}


//! override this to create from the Gaudi-style n-tuple
const TupleItem* GaudiTuple::tupleItem(const std::string& name)
{
// dummy for now
    return new TupleItem;
}
