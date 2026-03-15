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
#include <VGCore/Include/Core/CoreTypes.h>
//#include <VGEngine/Include/Resource/Interface/AudioInterface.h>
#include <HMedia/Interface/AudioInterface.h>

namespace VisionGal::Editor
{
	class AudioViewer : public IEditorTaskPanel
	{
	public:
		AudioViewer(const VGPath& path);
		AudioViewer(const AudioViewer&) = default;
		AudioViewer& operator=(const AudioViewer&) = default;
		AudioViewer(AudioViewer&&) noexcept = default;
		AudioViewer& operator=(AudioViewer&&) noexcept = default;
		~AudioViewer() override;

		void RenderUI(TaskContext& context) override;
	private:
		void RenderProgressBarUI();
		void RenderPlayButtonUI();
		void RenderLoopButtonUI();
		void RenderVolumeButtonUI();

		VGPath m_Path;
		Ref<Horizon::IAudioPlayer> m_AudioPlayer = nullptr;

		float m_CurrentPlayTime = 0.f;
		float m_AudioDuration = 0.f;
		std::string m_AudioDurationFormat;
	};
}
