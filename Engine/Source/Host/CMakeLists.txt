set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

add_executable(NevernessHost
    main.cpp
)

# 必须要注意引用，以自动编译C++ 端的DLL，并且在运行时能够正确加载。
target_link_libraries(NevernessHost PRIVATE NevernessRuntime-Managed)

add_custom_target(NevernessEditorBuild ALL
    COMMAND dotnet build
        ${CMAKE_SOURCE_DIR}/Engine/Source/Managed/Editor/NevernessEditor/NevernessEditor.csproj
        -c Debug
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

#set(NEVERNESS_EDITOR_CSPROJ
#    ${CMAKE_SOURCE_DIR}/Engine/Source/Managed/Editor/NevernessEditor/NevernessEditor.csproj
#)
#
#add_custom_target(NevernessEditorBuild
#    COMMAND dotnet build
#        ${NEVERNESS_EDITOR_CSPROJ}
#        --no-restore
#        -c $<CONFIG>
#
#    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#
#    COMMENT "Building NevernessEditor (.NET)"
#)


add_dependencies(NevernessHost NevernessEditorBuild)