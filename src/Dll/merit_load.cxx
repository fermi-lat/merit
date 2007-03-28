/*
* @file merit_load.cxx
* @brief This is needed for forcing the linker to load all components
* of the library.
*
*  $Header: /nfs/slac/g/glast/ground/cvs/merit/src/Dll/merit_load.cxx,v 1.6 2006/10/23 19:04:55 burnett Exp $
*/

#include "GaudiKernel/DeclareFactoryEntries.h"

DECLARE_FACTORY_ENTRIES(merit) {
    DECLARE_ALGORITHM( meritAlg );

    //DECLARE_ALGORITHM( FT1Alg );
} 



