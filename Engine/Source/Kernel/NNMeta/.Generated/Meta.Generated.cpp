/* ----------------------------------------------------------------------------
** GENERATED SOURCE FILE
**
** Meta Reflection Module Source 
** Generator v1.0
** --------------------------------------------------------------------------*/



#include "Meta.Generated.h"
#include <NNKernel/Include/Meta/MetaCommonDefinitions.h>

///////////////////////////////////////////////////////////////////////////////
// Module Files
///////////////////////////////////////////////////////////////////////////////


#include "NNKernel/.Generated\System\HFileWatcher.Generated.h"
#include "NNKernel/.Generated\Event\HWindowEvents.Generated.h"
#include "NNKernel/.Generated\HConfig.Generated.h"

namespace meta_namespace = META_NAMESPACE;

MetaGenerated::module::ModuleHCoreMeta::ModuleHCoreMeta(meta_namespace::ReflectionDatabase &db)
    : ReflectionModule( db )
{
    ///////////////////////////////////////////////////////////////////////////
    // External Allocation
    ///////////////////////////////////////////////////////////////////////////
    

    ///////////////////////////////////////////////////////////////////////////
    // Module File Allocation
    ///////////////////////////////////////////////////////////////////////////
     
    AllocateModuleFileHCoreMetaSystem_HFileWatcher( db ); 
    AllocateModuleFileHCoreMetaEvent_HWindowEvents( db ); 
    AllocateModuleFileHCoreMetaHConfig( db );
}

MetaGenerated::module::ModuleHCoreMeta::~ModuleHCoreMeta(void)
{
    ///////////////////////////////////////////////////////////////////////////
    // Module File Definition
    ///////////////////////////////////////////////////////////////////////////
    
    DefineModuleFileHCoreMetaSystem_HFileWatcher( db );
    DefineModuleFileHCoreMetaEvent_HWindowEvents( db );
    DefineModuleFileHCoreMetaHConfig( db );
}