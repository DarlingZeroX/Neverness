# CMakeList.txt: GalEngine 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
# 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

# 开启utf-8编码支持
if (MSVC)
    add_compile_options(/utf-8)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()


# 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

# 开启utf-8编码支持
if (MSVC)
    add_compile_options(/utf-8)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

# 将源代码添加到此项目的可执行文件。
file(GLOB HCORE_INTERFACE_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Interface/*.h"
)

file(GLOB HCORE_HEADER_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
)

file(GLOB HCORE_SRC_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
)

# 库
add_library (VGAsset SHARED ${HCORE_INTERFACE_FILES} ${HCORE_HEADER_FILES} ${HCORE_SRC_FILES})

# 仅对动态库目标定义 CORE_MODULE_EXPORT
target_compile_definitions(VGAsset PRIVATE VG_ASSET_EXPORT)

# 添加头文件搜索路径
target_include_directories(VGAsset PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(VGAsset PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Kernel)
target_include_directories(VGAsset PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(VGAsset PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Interface)

find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(VGAsset PRIVATE SDL3::SDL3)

find_package(SDL3_image CONFIG REQUIRED)
target_link_libraries(VGAsset PRIVATE $<IF:$<TARGET_EXISTS:SDL3_image::SDL3_image-shared>,SDL3_image::SDL3_image-shared,SDL3_image::SDL3_image-static>)

target_link_libraries(VGAsset PRIVATE HCore)
target_link_libraries(VGAsset PRIVATE HCorePlatform)
target_link_libraries(VGAsset PRIVATE HFileSystem)
target_link_libraries(VGAsset PRIVATE VGCore)
target_link_libraries(VGAsset PRIVATE HMedia)
