# CMakeList.txt: VisionGal 的 CMake 项目，在此处包括源代码并定义
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
file(GLOB EDITOR_FRMAEWORK_HEADER_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"

	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Asset/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Asset/ImportStarter/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Asset/ImporterUITask/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/AssetEditor/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/AssetImporter/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/EditorComponents/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/EditorComponents/DetailBrowser/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/EditorComponents/ContentBrowser/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/EditorComponents/CodeStudio/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/MainEditor/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Setting/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/UITask/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/EditorCore/*.h"

	"${CMAKE_CURRENT_SOURCE_DIR}/Interface/*.h"
)

# 将源代码添加到此项目的可执行文件。
file(GLOB EDITOR_FRMAEWORK_SRC_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Asset/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Asset/ImportStarter/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Asset/ImporterUITask/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/AssetEditor/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/AssetImporter/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/EditorComponents/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/EditorComponents/DetailBrowser/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/EditorComponents/ContentBrowser/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/EditorComponents/CodeStudio/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/MainEditor/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Setting/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/UITask/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/EditorCore/*.cpp"
)

add_library (VGEditorFramework SHARED ${EDITOR_FRMAEWORK_HEADER_FILES} ${EDITOR_FRMAEWORK_SRC_FILES})

# 添加宏
target_compile_definitions(VGEditorFramework PRIVATE VG_EDITOR_FRAMEWORK_API_EXPORT)

# 添加头文件搜索路径
target_include_directories(VGEditorFramework PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(VGEditorFramework PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

find_package(SDL3 CONFIG REQUIRED)

target_link_libraries(VGEditorFramework PRIVATE SDL3::SDL3)
target_link_libraries(VGEditorFramework PRIVATE HCore)
target_link_libraries(VGEditorFramework PRIVATE HCorePlatform)
target_link_libraries(VGEditorFramework PRIVATE HFileSystem)
target_link_libraries(VGEditorFramework PRIVATE VGAsset)
target_link_libraries(VGEditorFramework PRIVATE HMedia)
target_link_libraries(VGEditorFramework PRIVATE VGImgui)
target_link_libraries(VGEditorFramework PRIVATE VGEngine)
