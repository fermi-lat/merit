/*
* @file merit_load.cxx
* @brief This is needed for forcing the linker to load all components
* of the library.
*
*  $Header: /nfs/slac/g/glast/ground/cvs/merit/src/Dll/merit_load.cxx,v 1.2 2002/05/26 03:35:09 burnett Exp $
*/

#include "GaudiKernel/DeclareFactoryEntries.h"

DECLARE_FACTORY_ENTRIES(merit) {
    DECLARE_ALGORITHM( meritAlg );

} 



