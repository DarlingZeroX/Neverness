# CMakeList.txt: GalEngine 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
# 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

# 添加头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
include_directories(${PROJECT_SOURCE_DIR}/Engine/Source/Kernel)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Include)

# 添加宏
add_definitions(-DIMGUI_EXPORT)
#add_definitions(-DIMGUI_IMPL_OPENGL_ES3)

# 开启utf-8编码支持
if (MSVC)
    add_compile_options(/utf-8)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
 
# 将源代码添加到此项目的可执行文件。
file(GLOB IMGUI_SRC_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Imgui/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Imgui/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Imgui/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/ImGuiColorTextEdit/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/ImGuiColorTextEdit/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/ImguiEx/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/ImguiEx/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/ImguiLayer/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/ImguiLayer/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/imGuizmo/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/imGuizmo/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/ImNodeEditor/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/ImNodeEditor/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Imnodes/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Imnodes/*.cpp"
)

# 库
add_library (VGImgui SHARED ${IMGUI_SRC_FILES})

find_package(SDL3 CONFIG REQUIRED)
find_package(Freetype REQUIRED)

target_link_libraries(VGImgui PRIVATE Freetype::Freetype)
target_link_libraries(VGImgui PRIVATE SDL3::SDL3)
target_link_libraries(VGImgui PRIVATE HCore)
target_link_libraries(VGImgui PRIVATE HCorePlatform)

# 链接OpenGL库
if (APPLE)
    # 在macOS上链接OpenGL框架
    target_link_libraries(VGImgui PRIVATE "-framework OpenGL")
elseif (UNIX)
    # 在Linux上链接OpenGL库
    target_link_libraries(VGImgui PRIVATE GL)
elseif (WIN32)
    # 在Windows上，可能需要根据具体情况设置OpenGL库路径和库名
    target_link_libraries(VGImgui PRIVATE opengl32)
endif ()

