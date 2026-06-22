set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

add_executable(NevernessHost
    main.cpp
)

# 必须要注意引用，以自动编译C++ 端的DLL，并且在运行时能够正确加载。
target_link_libraries(NevernessHost PRIVATE NevernessRuntime-EngineServices)

# C# Managed Editor 编译（默认关闭，手动 dotnet build 管理）
option(VISIONGAL_BUILD_MANAGED_EDITOR "Build C# Managed Editor via CMake (calls dotnet build)" OFF)

if (VISIONGAL_BUILD_MANAGED_EDITOR)
    add_custom_target(NevernessEditorBuild ALL
        COMMAND dotnet build
            ${CMAKE_SOURCE_DIR}/Engine/Source/Managed/Editor/NevernessEditor/NevernessEditor.csproj
            -c Debug
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

    add_dependencies(NevernessHost NevernessEditorBuild)
endif()