//$Header: /nfs/slac/g/glast/ground/cvs/merit/src/app/AsciiTuple.h,v 1.1 2001/03/23 19:52:01 burnett Exp $
// Original author T. Burnett 
#ifndef AsciiTuple_H
#define AsciiTuple_H
#include "analysis/Tuple.h"


#include <string>
#include <iostream>


class AsciiTuple : public Tuple {

public:
    ///
    AsciiTuple::AsciiTuple(std::istream& file);
    ~AsciiTuple(){};

    bool nextEvent();
    
private:
    std::istream& m_file;
};

#endif
