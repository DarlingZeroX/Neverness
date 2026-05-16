# VisionGal Kernel — H* native foundation modules (HCore, platform, filesystem, media, node graph).
set(VISIONGAL_KERNEL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "")
add_subdirectory(HCore)
add_subdirectory(HCorePlatform)
add_subdirectory(HFileSystem)
add_subdirectory(HNGRuntimeCore)
add_subdirectory(HMedia)
add_subdirectory(HNGRuntimeNodes)
# HMeta: sources live under Kernel/HMeta but are not part of the default build (see HMeta/CMakeLists.txt).
