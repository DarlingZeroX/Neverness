/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*/

#pragma once
//#include "../Include/EngineConfig.h"
#include "../../Interface/SceneInterface.h"
#include <NNKernel/Interface/HVector.h>

namespace VisionGal
{
	////////////////		LuaScript Event
	enum class LuaScriptEventType
	{
		None = 0,
		ScriptError,
	};

	struct LuaScriptEvent
	{
		LuaScriptEventType EventType = LuaScriptEventType::None;
		std::string ScriptPath;
		std::string ErrorMessage;
		int ErrorLineNumber = 0;
	};

	////////////////		LuaScript Event
	enum class UISystemEventType
	{
		None = 0,
		UIFileOpen,
		UIFileClose,
	};

	struct UISystemEvent
	{
		UISystemEventType EventType = UISystemEventType::None;
		std::string UIFilePath;
	};

	////////////////		Engine Event
	enum class EngineEventType
	{
		None = 0,
		MainSceneChanged,
		EnterScenePlayMode,
		ExitScenePlayMode
	};

	struct EngineEvent
	{
		EngineEventType EventType = EngineEventType::None;
		IScene* Scene = nullptr;
		IScene* PrevScene = nullptr;
	};

	////////////////		Scene Event
	enum class SceneEventType
	{
		None = 0,
		ActorCreating,
		ActorRemoved,
		ActorSelected
	};

	struct SceneEvent
	{
		SceneEventType EventType = SceneEventType::None;
		IGameActor* Actor = nullptr;
		VGActorID ActorID = 0;
	};

	////////////////		 Viewport Event

	enum class ViewportEventType
	{
		None,
		SizeChanged,
		Hovered,

		Focused,
		UnFocused,

		PlacePrefabActor,
		PlaceAsset,
		MouseMove,

		MouseClicked,
		MouseDragging,
	};

	struct ViewportEvent
	{
		ViewportEventType Type;
		float2 NewViewportSize;
		float2 MousePosition;
		String PrefabActorType;
		String  Path;

		ViewportEvent()
			: Type(ViewportEventType::None),
			NewViewportSize(0),
			MousePosition(0)
		{
		}
	};
}