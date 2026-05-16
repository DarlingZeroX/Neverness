# CMakeList.txt: VGEditor CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
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

add_executable (VGDesktopApplication ${EDITOR_HEADER_FILES} ${EDITOR_SRC_FILES})

# 设置为Windows应用程序
set_target_properties(VGDesktopApplication PROPERTIES
    WIN32_EXECUTABLE TRUE
)

target_compile_definitions(VGDesktopApplication PRIVATE EDITOR_PROJECT_ROOT_DIR="${PROJECT_SOURCE_DIR}")

# 添加头文件搜索路径
target_include_directories(VGDesktopApplication PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(VGDesktopApplication PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Kernel)
target_include_directories(VGDesktopApplication PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Editor)
target_include_directories(VGDesktopApplication PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
 
# 链接库
target_link_libraries(VGDesktopApplication PRIVATE VGEditorFramework)
if(VISIONGAL_BUILD_LEGACY_GALGAME)
	target_link_libraries(VGDesktopApplication PRIVATE VGEditorGalgame)
	target_compile_definitions(VGDesktopApplication PRIVATE VISIONGAL_BUILD_LEGACY_GALGAME=1)
else()
	target_compile_definitions(VGDesktopApplication PRIVATE VISIONGAL_NO_LEGACY_GALGAME=1)
endif()