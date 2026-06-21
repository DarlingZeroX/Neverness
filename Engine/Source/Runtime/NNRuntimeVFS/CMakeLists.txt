add_definitions(-DVGCORE_DYNAMIC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

if (MSVC)
    add_compile_options(/utf-8)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

add_library(NevernessRuntime-VFS SHARED)

target_sources(NevernessRuntime-VFS
  PRIVATE
    RuntimeVFSExport.h
    Include/VFS/VFSConfig.h
    Include/VFS/VFS.h
    Include/VFS/VFSNative.h
    Include/VFS/VFSMemory.h
    Include/VFS/VFSZip.h
    Include/VFS/VFSBackends.h
    Include/VFS/IFile.h
    Include/VFS/IFileSystem.h
    Include/VFS/FileInfo.h
    Include/VFS/StringUtils.h
    Include/VFS/VirtualFileSystem.h
    Include/VFS/NativeFile.h
    Include/VFS/NativeFileSystem.h
    Include/VFS/MemoryFile.h
    Include/VFS/MemoryFileSystem.h
    Include/VFS/ZipFile.h
    Include/VFS/ZipFileSystem.h
    Include/VFS/zip_file.h
    Include/VFSService.h
    Include/VfsApiExport.h
    ABI/VfsRuntimeApi.cpp
    Source/FileInfo.cpp
    Source/StringUtils.cpp
    Source/VFSService.cpp
)

target_compile_definitions(NevernessRuntime-VFS PRIVATE VG_VFS_API_EXPORT)

target_include_directories(NevernessRuntime-VFS PUBLIC ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(NevernessRuntime-VFS PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Core)
target_include_directories(NevernessRuntime-VFS PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

target_link_libraries(NevernessRuntime-VFS PUBLIC NevernessRuntime-NativeEngineAPI)
target_link_libraries(NevernessRuntime-VFS PRIVATE NevernessCore-Core)
target_link_libraries(NevernessRuntime-VFS PRIVATE NevernessCore-PlatformCore)
