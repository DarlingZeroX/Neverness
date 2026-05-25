# NNRuntimeScene — ECS-first 运行时场景（Phase 1–3）

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

if (MSVC)
	add_compile_options(/utf-8)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

add_library(NevernessRuntime-Scene SHARED)

target_sources(NevernessRuntime-Scene
	PRIVATE
		NNRuntimeSceneExport.h
		Include/Scene/NNEntity.h
		Include/Scene/NNEntityHandle.h
		Include/Scene/NNWorld.h
		Include/Scene/NNRuntimeScene.h
		Include/Components/NNTransformComponent.h
		Include/Components/NNRelationshipComponent.h
		Include/Components/NNTagComponent.h
		Include/Components/NNCameraComponent.h
		Include/Components/NNSpriteRendererComponent.h
		Include/Query/NNEntityQuery.h
		Include/Reflection/NNComponentTypeId.h
		Include/Reflection/NNComponentFieldType.h
		Include/Reflection/NNComponentRegistry.h
		Include/Systems/NNSceneTickGroup.h
		Include/Systems/ISceneSystem.h
		Include/Systems/NNSceneSystemScheduler.h
		Include/Systems/NNTransformSystem.h
		Include/Systems/NNHierarchySystem.h
		Include/Systems/NNSceneUpdateSystem.h
		Include/Systems/NNCameraSystem.h
		Include/Runtime/NNSceneEventBus.h
		Include/Runtime/NNDirtyTracker.h
		Include/Serialization/NNSceneSerializer.h
		Include/Serialization/NNJsonSceneSerializer.h
		Include/Snapshot/NNHierarchySnapshotBuilder.h
		Include/Snapshot/NNReflectionSnapshotBuilder.h
		Include/Prefab/NNPrefab.h
		Include/Scene/NNSceneAssetLoader.h
		Include/Scene/NNStreamingZone.h
		Include/Runtime/NNSceneRuntime.h
		Source/Scene/NNEntity.cpp
		Source/Scene/NNEntityHandle.cpp
		Source/Scene/NNRuntimeScene.cpp
		Source/Reflection/NNComponentRegistry.cpp
		Source/Reflection/BuiltinComponentRegistration.cpp
		Source/Systems/NNSceneSystemScheduler.cpp
		Source/Systems/NNTransformSystem.cpp
		Source/Systems/NNHierarchySystem.cpp
		Source/Systems/NNSceneUpdateSystem.cpp
		Source/Systems/NNCameraSystem.cpp
		Source/Runtime/NNSceneEventBus.cpp
		Source/Runtime/NNDirtyTracker.cpp
		Source/Serialization/NNSceneSerializer.cpp
		Source/Serialization/NNJsonSceneSerializer.cpp
		Source/Snapshot/NNHierarchySnapshotBuilder.cpp
		Source/Snapshot/NNReflectionSnapshotBuilder.cpp
		Source/Prefab/NNPrefab.cpp
)

target_compile_definitions(NevernessRuntime-Scene PRIVATE NN_RUNTIME_SCENE_EXPORT NN_RUNTIME_SCENE_DYNAMIC)

target_include_directories(NevernessRuntime-Scene PUBLIC
	${PROJECT_SOURCE_DIR}/Engine/Source/Runtime
	${CMAKE_CURRENT_SOURCE_DIR}/Include
)
target_include_directories(NevernessRuntime-Scene PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Core)

target_link_libraries(NevernessRuntime-Scene PUBLIC NevernessCore-Core)
target_include_directories(NevernessRuntime-Scene PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime/NNNativeEngineAPI/Include)

if (ENABLE_TESTS)
	find_package(GTest CONFIG QUIET)
	if (GTest_FOUND)
		add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Test")
	endif()
endif()
