// $Header: $
//====================================================================
//  merit_load.cxx
//--------------------------------------------------------------------
//
//  Package    : merit
//
//  Description: Implementation of <Package>_load routine.
//               This routine is needed for forcing the linker
//               to load all the components of the library. 
//
//====================================================================

#include "Gaudi/Interfaces/ICnvFactory.h"
#include "Gaudi/Interfaces/ISvcFactory.h"
#include "Gaudi/Interfaces/IAlgFactory.h"


#define DLL_DECL_SERVICE(x)    extern const ISvcFactory& x##Factory; x##Factory.addRef();
#define DLL_DECL_CONVERTER(x)  extern const ICnvFactory& x##Factory; x##Factory.addRef();
#define DLL_DECL_ALGORITHM(x)  extern const IAlgFactory& x##Factory; x##Factory.addRef();
#define DLL_DECL_OBJECT(x)     extern const IFactory& x##Factory; x##Factory.addRef();

//! Load all  services: 
void merit_load() {
    DLL_DECL_ALGORITHM( meritAlg );
} 

extern "C" void merit_loadRef()    {
  merit_load();
}

