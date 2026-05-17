enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
	add_compile_options(/utf-8)
endif()

add_executable(NNRuntimnTest-Scene NNRuntimeSceneTest.cpp)

target_include_directories(NNRuntimnTest-Scene PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(NNRuntimnTest-Scene PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Core)
target_include_directories(NNRuntimnTest-Scene PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../Include)

find_package(GTest CONFIG REQUIRED)
target_link_libraries(NNRuntimnTest-Scene PRIVATE GTest::gtest GTest::gtest_main NevernessRuntime-Scene)

add_test(NAME NNRuntimnTest-Scene COMMAND $<TARGET_FILE:NNRuntimnTest-Scene>)
