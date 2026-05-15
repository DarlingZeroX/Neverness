/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
 * GitHub page: https://github.com/DarlingZeroX/VisionGal
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 *
 * Phase 7：运行态按语义分组为嵌套 struct，便于维护；均为内存态字段（非磁盘裸 memcpy 布局契约）。
 */

#pragma once
#include "VGCore/Include/Core/Core.h"
#include <VGRHI/Interface/Texture.h>

namespace VisionGal::GalGame
{
	/**
	 * @brief Gal 运行时 UI/剧情相关可变状态（POD 聚合，由 GalGameContext 持有）。
	 *
	 * 子结构仅表达逻辑分组，不隐含行为；业务逻辑仍在 VGGalgame / Runtime 各系统中实现。
	 */
	struct GalGameRuntimeState
	{
		/// 当前加载的主线脚本资源路径（脚本子系统维护）。
		std::string currentScriptPath;

		/** 对白展示：说话人、正文、引擎行号（与存档 line 对齐）。 */
		struct DialogueState
		{
			std::string currentDialogCharacter;
			std::string currentDialogText;
			uint32_t currentDialogLine = 0;
		} dialogue;

		Ref<VGFX::TexturePixels> screenshotPixels = nullptr;

		/** 文本渲染进度与打字机开关（UI / Dialogue 共用数据面）。 */
		struct TextDisplayState
		{
			float textShownProgress = 0.0f;
			bool isTextFullyShown = false;
			bool enableTyping = true;
		} textDisplay;

		/** 快进、自动播放、读档中、语音层状态等「播放控制」数据。 */
		struct PlaybackState
		{
			bool enableFastForward = false;
			float fastForwardDelay = 0.f;
			bool isCurrentLoadingArchive = false;
			bool enableAutoDialogue = false;
			float autoDialogueDelay = 2.0f;
			bool IsVoicing = false;
		} playback;
	};
}
