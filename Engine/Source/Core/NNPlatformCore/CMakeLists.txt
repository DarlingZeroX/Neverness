# ���� C++ ��׼Ϊ C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

# ����utf-8����֧��
if (MSVC)
    add_compile_options(/utf-8)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/NNPlatformCoreSources.cmake)

add_library(NevernessCore-PlatformCore STATIC)
target_sources(NevernessCore-PlatformCore PRIVATE ${NNPlatformCore_SOURCES})

# ���Զ�̬��Ŀ�궨�� CORE_MODULE_EXPORT
#target_compile_definitions(NevernessCore-PlatformCore PRIVATE H_CORE_PLATFORM_EXPORT)

# ����ͷ�ļ�����·��
target_include_directories(NevernessCore-PlatformCore PUBLIC ${VISIONGAL_KERNEL_ROOT})
target_include_directories(NevernessCore-PlatformCore PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(NevernessCore-PlatformCore PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Interface)
target_include_directories(NevernessCore-PlatformCore PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Interface/FileSystem)

find_package(SDL3 CONFIG REQUIRED)
# ����ͷ���� Include/SDL3/SDL3Window.h��ֱ�� #include <SDL3/...>�������������� SDL ͷ·�������ӽӿ�
target_link_libraries(NevernessCore-PlatformCore PUBLIC NevernessCore-Core SDL3::SDL3)
