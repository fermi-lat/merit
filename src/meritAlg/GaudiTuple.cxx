// $Header:$
// Original author: T. Burnett

#include "GaudiTuple.h"
#include "Gaudi/interfaces/INTuple.h"
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
