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

# 文件图标
if(WIN32)
     set(APP_RESOURCES resources.rc)
endif()
add_executable (VGLauncher ${EDITOR_HEADER_FILES} ${EDITOR_SRC_FILES} ${APP_RESOURCES})

# 设置为Windows应用程序
set_target_properties(VGLauncher PROPERTIES
    WIN32_EXECUTABLE TRUE
)

target_compile_definitions(VGLauncher PRIVATE EDITOR_PROJECT_ROOT_DIR="${PROJECT_SOURCE_DIR}")

# 添加头文件搜索路径
target_include_directories(VGLauncher PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(VGLauncher PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Editor)
target_include_directories(VGLauncher PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

# 链接库
target_link_libraries(VGLauncher PRIVATE VGEditorFramework)

