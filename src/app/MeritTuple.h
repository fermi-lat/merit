//$Header:$
// Original author T. Burnett 
#ifndef MeritTuple_H
#define MeritTuple_H
#include "MeritTuple.h"

/** Define this for Merit
*/



class  MeritTuple {

public:
    ///
    MeritTuple::MeritTuple(std::string file);
    ~MeritTuple(){};

    //! subclass override this 
    virtual const TupleItem* tupleItem(const std::string& name)=0;
    
private:
};

#endif