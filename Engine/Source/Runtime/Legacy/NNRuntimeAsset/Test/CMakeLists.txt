enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
	add_compile_options(/utf-8)
endif()

# NNAssetFormat 测试
add_executable(NNAssetFormatTest NNAssetFormatTest.cpp)
target_include_directories(NNAssetFormatTest PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(NNAssetFormatTest PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Core)
target_include_directories(NNAssetFormatTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../Include)
find_package(GTest CONFIG REQUIRED)
target_link_libraries(NNAssetFormatTest PRIVATE GTest::gtest GTest::gtest_main NevernessRuntime-Asset)
add_test(NAME NNAssetFormatTest COMMAND $<TARGET_FILE:NNAssetFormatTest>)

# NNHandleTable 测试
add_executable(NNHandleTableTest NNHandleTableTest.cpp)
target_include_directories(NNHandleTableTest PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(NNHandleTableTest PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Core)
target_include_directories(NNHandleTableTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../Include)
target_link_libraries(NNHandleTableTest PRIVATE GTest::gtest GTest::gtest_main NevernessRuntime-Asset)
add_test(NAME NNHandleTableTest COMMAND $<TARGET_FILE:NNHandleTableTest>)

# NNAssetManager + GuidHashMap 测试
add_executable(NNAssetManagerTest NNAssetManagerTest.cpp)
target_include_directories(NNAssetManagerTest PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(NNAssetManagerTest PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Core)
target_include_directories(NNAssetManagerTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../Include)
target_link_libraries(NNAssetManagerTest PRIVATE GTest::gtest GTest::gtest_main NevernessRuntime-Asset)
add_test(NAME NNAssetManagerTest COMMAND $<TARGET_FILE:NNAssetManagerTest>)
