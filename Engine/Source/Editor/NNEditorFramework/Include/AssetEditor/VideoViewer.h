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
#include "../Config.h"
#include <NNRuntimeCore/Include/Core/CoreTypes.h>
#include <NNEngineLegacy/Include/Engine/VideoPlayer.h>

namespace VisionGal::Editor
{
	class VideoViewer : public IEditorTaskPanel
	{
	public:
		VideoViewer(const VGPath& path);
		VideoViewer(const VideoViewer&) = default;
		~VideoViewer() override;
		VideoViewer& operator=(const VideoViewer&) = default;
		VideoViewer(VideoViewer&&) noexcept = default;
		VideoViewer& operator=(VideoViewer&&) noexcept = default;

		void RenderUI(TaskContext& context) override;
	private:
		void RenderVideoTexture();
		void RenderProgressBarUI();
		void RenderPlayButtonUI();
		void RenderLoopButtonUI();
		void RenderVolumeButtonUI();

		VGPath m_Path;
		Ref<IVideoPlayer> m_VideoPlayer = nullptr;

		float m_CurrentPlayTime = 0.f;
		float m_VideoDuration = 0.f;
		std::string m_AudioDurationFormat;

		int m_CurrentVideoUISizeX = 0;
		int m_CurrentVideoUISizeY = 0;
	};
}
