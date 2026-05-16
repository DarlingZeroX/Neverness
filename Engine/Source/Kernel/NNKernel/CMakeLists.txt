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


file(GLOB GEN_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/.Generated/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/.Generated/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/.Generated/Event/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/.Generated/Event/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/.Generated/System/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/.Generated/System/*.cpp"
)

# 将源代码添加到此项目的可执行文件。
file(GLOB SRC_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Asset/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/File/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/File/XML/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Scene/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Utils/*.cpp"
)

file(GLOB HEADER_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Interface/*.h"

	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Event/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Event/*.cpp"

	"${CMAKE_CURRENT_SOURCE_DIR}/Include/File/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/File/nlohmann/*.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/File/XML/*.hpp"

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

	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Scene/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Stb/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Utils/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/VFS/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/VFS/*.hpp"

	"${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/*.cpp"

	"${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

# 库
add_library (NevernessKernel-Kernel STATIC ${GEN_FILES} ${HEADER_FILES} ${SRC_FILES})

# 仅对动态库目标定义 CORE_MODULE_EXPORT
# target_compile_definitions(NevernessKernel-Kernel PRIVATE CORE_MODULE_EXPORT)

# 添加头文件搜索路径
target_include_directories(NevernessKernel-Kernel PUBLIC ${VISIONGAL_KERNEL_ROOT})
target_include_directories(NevernessKernel-Kernel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(NevernessKernel-Kernel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Interface)

find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(NevernessKernel-Kernel PUBLIC SDL3::SDL3)
