// $Header: $
//====================================================================
//  merit_dll.cpp
//--------------------------------------------------------------------
//
//  Package    : merit
//
//  Description: Implementation of DllMain routine.
//               The DLL initialisation must be done seperately for 
//               each DLL. 
//
//
//====================================================================

// DllMain entry point
#include "Gaudi/System/DllMain.icpp"
#include <iostream>
void GaudiDll::initialize(void*) 
{
}

void GaudiDll::finalize(void*) 
{
}
extern void merit_load();
#include "Gaudi/Kernel/FactoryTable.h"

extern "C" FactoryTable::EntryList* getFactoryEntries() {
  static bool first = true;
  if ( first ) {  // Don't call for UNIX
    merit_load();
    first = false;
  }
  return FactoryTable::instance()->getEntries();
} 

