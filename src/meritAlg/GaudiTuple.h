// 
#ifndef GGTUPLE_H
#define GGTUPLE_H
#include "analysis/Tuple.h"
#include "Gaudi/NTupleSvc/NTuple.h"


#include <string>


class GaudiTuple : public Tuple {

public:
    ///
    GaudiTuple(const std::string& title, NTuplePtr* nt);
    ~GaudiTuple();

    //! override this to create from the Gaudi-style n-tuple
    const TupleItem* tupleItem(const std::string& name);
    
private:
    NTuplePtr* m_nt;
};

#endif