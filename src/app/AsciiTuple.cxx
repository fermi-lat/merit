//$Header:$
// Original author T. Burnett 
#include "AsciiTuple.h"



AsciiTuple::AsciiTuple(std::istream& file): Tuple(file), m_file(file) {

}

bool AsciiTuple::nextEvent()
{
    m_file >> *this;

    return m_file.good();
}
