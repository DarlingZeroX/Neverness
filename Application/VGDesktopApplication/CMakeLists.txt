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
target_include_directories(VGDesktopApplication PRIVATE ${PROJECT_SOURCE_DIR}/Runtime)
target_include_directories(VGDesktopApplication PRIVATE ${PROJECT_SOURCE_DIR}/Editor)
target_include_directories(VGDesktopApplication PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

# 寻找CMake包
#find_package(glad CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(RmlUi REQUIRED)

# 链接库
#target_link_libraries(VGEditor PRIVATE glad::glad)
target_link_libraries(VGDesktopApplication PRIVATE RmlUi::RmlUi)
target_link_libraries(VGDesktopApplication PRIVATE SDL3::SDL3)
target_link_libraries(VGDesktopApplication PRIVATE HCore)
target_link_libraries(VGDesktopApplication PRIVATE HCorePlatform)
target_link_libraries(VGDesktopApplication PRIVATE VGPackage)
target_link_libraries(VGDesktopApplication PRIVATE VGImgui)
target_link_libraries(VGDesktopApplication PRIVATE VGEngine)
target_link_libraries(VGDesktopApplication PRIVATE VGEditorFramework)

# TODO: 如有需要，请添加测试并安装目标。
