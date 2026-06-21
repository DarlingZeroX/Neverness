# VisionGal Kernel — H* native foundation modules (HCore, platform, filesystem, media, node graph).
set(VISIONGAL_KERNEL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "")
add_subdirectory(NNCore)
add_subdirectory(NNPlatformCore)
add_subdirectory(NNMediaCore)
#add_subdirectory(NNNodeGraphCore)
#add_subdirectory(NNNodeGraphNodes)
# HMeta: sources live under Kernel/HMeta but are not part of the default build (see HMeta/CMakeLists.txt).
