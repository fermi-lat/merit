//$Header:$
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