/* ----------------------------------------------------------------------------
** GENERATED SOURCE FILE
**
** Meta Reflection Module File Source
** Generator v1.0
** --------------------------------------------------------------------------*/



#include "NNCore/.Generated\Event\HWindowEvents.Generated.h"
#include "NNCore/Include/Event/HWindowEvents.h"

#include <NNCore/Include/Meta/ReflectionDatabase.h>

namespace m = META_NAMESPACE;

void MetaGenerated::AllocateModuleFileHCoreMetaEvent_HWindowEvents(m::ReflectionDatabase &db)
{
    ///////////////////////////////////////////////////////////////////////////
    // Class Allocation
    ///////////////////////////////////////////////////////////////////////////
    
    ///////////////////////////////////////////////////////////////////////////
    // Enum Allocation
    ///////////////////////////////////////////////////////////////////////////
    
    m::ReflectTypeRegister<NN::Core::Events::HWindowEventType>(db, "", "NN::Core::Events::HWindowEventType", true);
    
    
    
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
    
    m::DefineReflectType<NN::Core::Events::HWindowEventType>(db,[](auto& db, auto& type, auto& typeID)
    {
        type.meta = {
            
        };

        type.SetEnum<NN::Core::Events::HWindowEventType>( "NN::Core::Events::HWindowEventType", {
             
                { "NONE", NN::Core::Events::HWindowEventType::NONE ,{  } }, 
             
                { "SHOWN", NN::Core::Events::HWindowEventType::SHOWN ,{  } }, 
             
                { "HIDDEN", NN::Core::Events::HWindowEventType::HIDDEN ,{  } }, 
             
                { "EXPOSED", NN::Core::Events::HWindowEventType::EXPOSED ,{  } }, 
             
                { "MOVED", NN::Core::Events::HWindowEventType::MOVED ,{  } }, 
             
                { "RESIZED", NN::Core::Events::HWindowEventType::RESIZED ,{  } }, 
             
                { "SIZE_CHANGED", NN::Core::Events::HWindowEventType::SIZE_CHANGED ,{  } }, 
             
                { "MINIMIZED", NN::Core::Events::HWindowEventType::MINIMIZED ,{  } }, 
             
                { "MAXIMIZED", NN::Core::Events::HWindowEventType::MAXIMIZED ,{  } }, 
             
                { "RESTORED", NN::Core::Events::HWindowEventType::RESTORED ,{  } }, 
             
                { "ENTER", NN::Core::Events::HWindowEventType::ENTER ,{  } }, 
             
                { "LEAVE", NN::Core::Events::HWindowEventType::LEAVE ,{  } }, 
             
                { "FOCUS_GAINED", NN::Core::Events::HWindowEventType::FOCUS_GAINED ,{  } }, 
             
                { "FOCUS_LOST", NN::Core::Events::HWindowEventType::FOCUS_LOST ,{  } }, 
             
                { "CLOSE", NN::Core::Events::HWindowEventType::CLOSE ,{  } }, 
             
                { "TAKE_FOCUS", NN::Core::Events::HWindowEventType::TAKE_FOCUS ,{  } }, 
             
                { "HIT_TEST", NN::Core::Events::HWindowEventType::HIT_TEST ,{  } }, 
             
                { "ICCPROF_CHANGED", NN::Core::Events::HWindowEventType::ICCPROF_CHANGED ,{  } }, 
             
                { "DISPLAY_CHANGED", NN::Core::Events::HWindowEventType::DISPLAY_CHANGED ,{  } }, 
             
                { "DROP_BEGIN", NN::Core::Events::HWindowEventType::DROP_BEGIN ,{  } }, 
             
                { "DROP_FILE", NN::Core::Events::HWindowEventType::DROP_FILE ,{  } }, 
             
                { "DROP_TEXT", NN::Core::Events::HWindowEventType::DROP_TEXT ,{  } }, 
             
                { "DROP_COMPLETE", NN::Core::Events::HWindowEventType::DROP_COMPLETE ,{  } } 
            
        } );
     });
    
    ///////////////////////////////////////////////////////////////////////////
    // Class Definitions
    ///////////////////////////////////////////////////////////////////////////
    
}
