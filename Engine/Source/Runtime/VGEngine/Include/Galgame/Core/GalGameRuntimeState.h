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
#include "../../Core/Core.h"
#include "../Interface/GalgameInterface.h"

namespace VisionGal::GalGame
{
	struct GalGameRuntimeState
	{
		std::string currentScriptPath;

		std::string currentDialogCharacter;
		std::string currentDialogText;
		uint32_t currentDialogLine = 0;			// 当前对话行（script engine 决定的）

		Ref<VGFX::TexturePixels> screenshotPixels = nullptr;

		// 文本显示状态（UI 决定）
		float textShownProgress = 0.0f; // 0~1
		bool  isTextFullyShown = false;

		// 打字效果
		bool enableTyping = true;

		// 快进模式（UI 控制）
		bool enableFastForward = false;
		float fastForwardDelay = 0.f;

		// 当前正在加载存档
		bool isCurrentLoadingArchive = false;

		// 自动模式（UI 控制）
		bool enableAutoDialogue = false;
		float autoDialogueDelay = 2.0f;

		// 语音是否完成
		bool IsVoicing = false;
	};
}
