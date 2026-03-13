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


file(GLOB HCORE_GEN_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/.Generated/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/.Generated/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/.Generated/Event/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/.Generated/Event/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/.Generated/System/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/.Generated/System/*.cpp"
)

# 将源代码添加到此项目的可执行文件。
file(GLOB HCORE_SRC_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Asset/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/File/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/File/XML/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/GI/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Scene/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/System/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/System/Input/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Utils/*.cpp"
)

file(GLOB HCORE_HEADER_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Asset/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Core/*.h"

	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Event/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Event/*.cpp"

	"${CMAKE_CURRENT_SOURCE_DIR}/Include/File/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/File/nlohmann/*.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/File/XML/*.hpp"

	"${CMAKE_CURRENT_SOURCE_DIR}/Include/GI/*.h"

	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Math/GLM/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Math/GLM/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Math/GLM/detail/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Math/GLM/detail/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Math/GLM/ext/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Math/GLM/ext/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Math/GLM/gtc/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Math/GLM/gtc/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Math/GLM/gtx/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Math/GLM/gtx/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Math/GLM/simd/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Math/GLM/simd/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Math/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Math/*.cpp"

	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/Core/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/Core/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/Core/Argument/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/Core/Argument/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/Core/Array/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/Core/Array/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/Core/Constructor/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/Core/Constructor/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/Core/Destructor/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/Core/Destructor/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/Core/Enum/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/Core/Enum/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/Core/FieldInfo/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/Core/FieldInfo/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/Core/MethodInfo/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/Core/MethodInfo/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/Core/ParameterInfo/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/Core/ParameterInfo/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/Core/Type/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/Core/Type/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/Core/Variant/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/Core/Variant/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/Deps/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/Deps/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Meta/Impl/*.hpp"

	#"${PROJECT_SOURCE_DIR}/HCore/Misc/*.h"
	#"${PROJECT_SOURCE_DIR}/HCore/Misc/*.cpp"

	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Scene/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Stb/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/System/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/System/Input/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Utils/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/VFS/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/VFS/*.hpp"

	"${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/*.cpp"

	"${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

# 库
add_library (HCore SHARED ${HCORE_GEN_FILES} ${HCORE_HEADER_FILES} ${HCORE_SRC_FILES})

# 仅对动态库目标定义 CORE_MODULE_EXPORT
target_compile_definitions(HCore PRIVATE CORE_MODULE_EXPORT)

# 添加头文件搜索路径
target_include_directories(HCore PRIVATE ${PROJECT_SOURCE_DIR}/Runtime)
target_include_directories(HCore PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(HCore PRIVATE SDL3::SDL3)
