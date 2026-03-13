# CMakeList.txt: 在此处包括源代码并定义
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
file(GLOB EDITOR_HEADER_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
)

file(GLOB EDITOR_SRC_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
)

add_executable (VGPackageTool ${EDITOR_HEADER_FILES} ${EDITOR_SRC_FILES})

target_compile_definitions(VGPackageTool PRIVATE EDITOR_PROJECT_ROOT_DIR="${PROJECT_SOURCE_DIR}")

# 添加头文件搜索路径
target_include_directories(VGPackageTool PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(VGPackageTool PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Editor)
target_include_directories(VGPackageTool PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

# 寻找CMake包
find_package(SDL3 CONFIG REQUIRED)

# 链接库
target_link_libraries(VGPackageTool PRIVATE SDL3::SDL3)
target_link_libraries(VGPackageTool PRIVATE HCore)
target_link_libraries(VGPackageTool PRIVATE HCorePlatform)
target_link_libraries(VGPackageTool PRIVATE VGPackage)
target_link_libraries(VGPackageTool PRIVATE VGImgui)
target_link_libraries(VGPackageTool PRIVATE VGEngine)
target_link_libraries(VGPackageTool PRIVATE VGEditorFramework)
