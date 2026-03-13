/* ----------------------------------------------------------------------------
** GENERATED SOURCE FILE
**
** Meta Reflection Module File Source
** Generator v1.0
** --------------------------------------------------------------------------*/
#include "pch.h"


#include "HCore/.Generated\Event\HWindowEvents.Generated.h"
#include "HCore/Include/Event/HWindowEvents.h"

#include <HCore/Include/Meta/ReflectionDatabase.h>

namespace m = META_NAMESPACE;

void MetaGenerated::AllocateModuleFileHCoreMetaEvent_HWindowEvents(m::ReflectionDatabase &db)
{
    ///////////////////////////////////////////////////////////////////////////
    // Class Allocation
    ///////////////////////////////////////////////////////////////////////////
    
    ///////////////////////////////////////////////////////////////////////////
    // Enum Allocation
    ///////////////////////////////////////////////////////////////////////////
    
    m::ReflectTypeRegister<Horizon::Events::HWindowEventType>(db, "", "Horizon::Events::HWindowEventType", true);
    
    
    
}

void MetaGenerated::DefineModuleFileHCoreMetaEvent_HWindowEvents(m::ReflectionDatabase &db)
{
    ///////////////////////////////////////////////////////////////////////////
    // Global Definitions
    ///////////////////////////////////////////////////////////////////////////
    
    ///////////////////////////////////////////////////////////////////////////
    // Global Functions
    ///////////////////////////////////////////////////////////////////////////
    
    ///////////////////////////////////////////////////////////////////////////
    // Enum Definitions
    ///////////////////////////////////////////////////////////////////////////
    
    m::DefineReflectType<Horizon::Events::HWindowEventType>(db,[](auto& db, auto& type, auto& typeID)
    {
        type.meta = {
            
        };

        type.SetEnum<Horizon::Events::HWindowEventType>( "Horizon::Events::HWindowEventType", {
             
                { "NONE", Horizon::Events::HWindowEventType::NONE ,{  } }, 
             
                { "SHOWN", Horizon::Events::HWindowEventType::SHOWN ,{  } }, 
             
                { "HIDDEN", Horizon::Events::HWindowEventType::HIDDEN ,{  } }, 
             
                { "EXPOSED", Horizon::Events::HWindowEventType::EXPOSED ,{  } }, 
             
                { "MOVED", Horizon::Events::HWindowEventType::MOVED ,{  } }, 
             
                { "RESIZED", Horizon::Events::HWindowEventType::RESIZED ,{  } }, 
             
                { "SIZE_CHANGED", Horizon::Events::HWindowEventType::SIZE_CHANGED ,{  } }, 
             
                { "MINIMIZED", Horizon::Events::HWindowEventType::MINIMIZED ,{  } }, 
             
                { "MAXIMIZED", Horizon::Events::HWindowEventType::MAXIMIZED ,{  } }, 
             
                { "RESTORED", Horizon::Events::HWindowEventType::RESTORED ,{  } }, 
             
                { "ENTER", Horizon::Events::HWindowEventType::ENTER ,{  } }, 
             
                { "LEAVE", Horizon::Events::HWindowEventType::LEAVE ,{  } }, 
             
                { "FOCUS_GAINED", Horizon::Events::HWindowEventType::FOCUS_GAINED ,{  } }, 
             
                { "FOCUS_LOST", Horizon::Events::HWindowEventType::FOCUS_LOST ,{  } }, 
             
                { "CLOSE", Horizon::Events::HWindowEventType::CLOSE ,{  } }, 
             
                { "TAKE_FOCUS", Horizon::Events::HWindowEventType::TAKE_FOCUS ,{  } }, 
             
                { "HIT_TEST", Horizon::Events::HWindowEventType::HIT_TEST ,{  } }, 
             
                { "ICCPROF_CHANGED", Horizon::Events::HWindowEventType::ICCPROF_CHANGED ,{  } }, 
             
                { "DISPLAY_CHANGED", Horizon::Events::HWindowEventType::DISPLAY_CHANGED ,{  } }, 
             
                { "DROP_BEGIN", Horizon::Events::HWindowEventType::DROP_BEGIN ,{  } }, 
             
                { "DROP_FILE", Horizon::Events::HWindowEventType::DROP_FILE ,{  } }, 
             
                { "DROP_TEXT", Horizon::Events::HWindowEventType::DROP_TEXT ,{  } }, 
             
                { "DROP_COMPLETE", Horizon::Events::HWindowEventType::DROP_COMPLETE ,{  } } 
            
        } );
     });
    
    ///////////////////////////////////////////////////////////////////////////
    // Class Definitions
    ///////////////////////////////////////////////////////////////////////////
    
}
