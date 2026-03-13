/* ----------------------------------------------------------------------------
** GENERATED SOURCE FILE
**
** Meta Reflection Module File Source
** Generator v1.0
** --------------------------------------------------------------------------*/
#include "pch.h"


#include "HCore/.Generated\HConfig.Generated.h"
#include "HCore/Include/HConfig.h"

#include <HCore/Include/Meta/ReflectionDatabase.h>

namespace m = META_NAMESPACE;

void MetaGenerated::AllocateModuleFileHCoreMetaHConfig(m::ReflectionDatabase &db)
{
    ///////////////////////////////////////////////////////////////////////////
    // Class Allocation
    ///////////////////////////////////////////////////////////////////////////
    
    ///////////////////////////////////////////////////////////////////////////
    // Enum Allocation
    ///////////////////////////////////////////////////////////////////////////
    
    m::ReflectTypeRegister<PlatformType>(db, "", "PlatformType", true);
    
    
    
    m::ReflectTypeRegister<EWindow>(db, "", "EWindow", true);
    
    
    
    m::ReflectTypeRegister<HTYPE_GRAPHICS_API>(db, "", "HTYPE_GRAPHICS_API", true);
    
    
    
}

void MetaGenerated::DefineModuleFileHCoreMetaHConfig(m::ReflectionDatabase &db)
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
    
    m::DefineReflectType<PlatformType>(db,[](auto& db, auto& type, auto& typeID)
    {
        type.meta = {
            
        };

        type.SetEnum<PlatformType>( "PlatformType", {
             
                { "Windows", PlatformType::Windows ,{  } }, 
             
                { "Mac", PlatformType::Mac ,{  } }, 
             
                { "Linux", PlatformType::Linux ,{  } }, 
             
                { "Unix", PlatformType::Unix ,{  } }, 
             
                { "Andriod", PlatformType::Andriod ,{  } }, 
             
                { "Other", PlatformType::Other ,{  } } 
            
        } );
     });
    
    m::DefineReflectType<EWindow>(db,[](auto& db, auto& type, auto& typeID)
    {
        type.meta = {
            
        };

        type.SetEnum<EWindow>( "EWindow", {
             
                { "WIN32_NATIVE", EWindow::WIN32_NATIVE ,{  } }, 
             
                { "GLFW", EWindow::GLFW ,{  } }, 
             
                { "SDL2", EWindow::SDL2 ,{  } } 
            
        } );
     });
    
    m::DefineReflectType<HTYPE_GRAPHICS_API>(db,[](auto& db, auto& type, auto& typeID)
    {
        type.meta = {
            
        };

        type.SetEnum<HTYPE_GRAPHICS_API>( "HTYPE_GRAPHICS_API", {
             
                { "D3D11", HTYPE_GRAPHICS_API::D3D11 ,{  } }, 
             
                { "D3D12", HTYPE_GRAPHICS_API::D3D12 ,{  } }, 
             
                { "OPENGL2", HTYPE_GRAPHICS_API::OPENGL2 ,{  } }, 
             
                { "VULKAN", HTYPE_GRAPHICS_API::VULKAN ,{  } } 
            
        } );
     });
    
    ///////////////////////////////////////////////////////////////////////////
    // Class Definitions
    ///////////////////////////////////////////////////////////////////////////
    
}
