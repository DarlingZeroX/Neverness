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

#include "AssetEditor/AudioViewer.h"
#include "NNEditorFramework/Include/EditorCore/Localization.h"
#include <NNRuntimeCore/Interface/Loader.h>
#include <NNRuntimeCore/Include/Utils/TimeHelper.h>
#include <NNEngineLegacy/Include/Engine/AudioPlayer.h>

namespace VisionGal::Editor
{
	AudioViewer::AudioViewer(const VGPath& path)
	{
		m_Path = path;
		if (auto audioClip = LoadObject<VGAudioClip>(path))
		{
			m_AudioPlayer = VGAudioPlayer::CreatePlayer(audioClip);
			m_AudioPlayer->Play();

			m_AudioDuration = m_AudioPlayer->GetDuration();
			m_AudioDurationFormat = TimeHelper::FloatToTimeFormatSprintf(m_AudioDuration);
		}
	}

	AudioViewer::~AudioViewer()
	{
		m_AudioPlayer->Stop();
	}

	void AudioViewer::RenderUI(TaskContext& context)
	{
		ImGuiEx::ScopedStyleVar winBorder(ImGuiStyleVar_WindowBorderSize, 1.0f);
		ImGuiEx::ScopedStyleColor winbg(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 1.0f));

		if (m_AudioPlayer == nullptr)
			return;

		ImGui::SetNextWindowSize(ImVec2(1024, 768), ImGuiCond_FirstUseEver);

		bool open = true;
		EditorText text(m_Path, ICON_FA_IMAGE);
		std::string windowName = text.GetText() + "##" + std::to_string(context.Index);
		if (ImGui::Begin(windowName.c_str(), &open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGuiEx::ScopedID winID(context.Index);

			// 进度条
			RenderProgressBarUI();
			RenderPlayButtonUI();
			RenderLoopButtonUI();
			RenderVolumeButtonUI();
		}

		ImGui::End();

		if (!open)
		{
			m_AudioPlayer->Stop();
			context.IsFinished = true;
		}
	}

	void AudioViewer::RenderProgressBarUI()
	{
		ImGui::Text(TimeHelper::FloatToTimeFormatSprintf(m_CurrentPlayTime).c_str());
		ImGui::SameLine();
		ImGui::SliderFloat("##AudioTimeControl", &m_CurrentPlayTime, 0.f, m_AudioDuration);
		//std::cout << m_CurrentPlayTime << std::endl;
		if (ImGui::IsItemDeactivatedAfterEdit()) {	// 当鼠标释放滑条时触发 Seek
			m_AudioPlayer->Seek(m_CurrentPlayTime);
		}
		if (!ImGui::IsItemActive()) {	// 当用户没有拖动滑条时，同步播放进度
			m_CurrentPlayTime = m_AudioPlayer->GetPlaybackTime();
		}
		ImGui::SameLine();
		ImGui::Text(m_AudioDurationFormat.c_str());
	}

	void AudioViewer::RenderPlayButtonUI()
	{
		// 让按钮居中
		{
			float windowWidth = ImGui::GetWindowSize().x;
			float x = (windowWidth) * 0.5f;
			ImGui::SetCursorPosX(x);
		}

		// 播放/停止场景按钮
		if (m_AudioPlayer->IsPlaying() == false)
		{
			if (ImGui::Button(ICON_FA_PLAY "##AudioViewer"))
			{
				m_AudioPlayer->Restore();
			}
		}
		else
		{
			if (ImGui::Button(ICON_FA_PAUSE "##AudioViewer"))
			{
				m_AudioPlayer->Pause();
			}
		}
	}

	void AudioViewer::RenderLoopButtonUI()
	{
		ImGui::SameLine();

		// 播放/停止场景按钮
		if (m_AudioPlayer->IsLooping() == false)
		{
			if (ImGui::Button(ICON_FA_REPEAT "##AudioViewerLoopButton"))
			{
				m_AudioPlayer->SetLoop(true);
			}
		}
		else
		{
			if (ImGui::Button(ICON_FA_UNLINK "##AudioViewerLoopButton"))
			{
				m_AudioPlayer->SetLoop(false);
			}
		}
	}

	void AudioViewer::RenderVolumeButtonUI()
	{
		ImGui::SameLine();

		if (ImGui::BeginPopupContextItem("AudioViewerVolumeControllerPopup"))
		{
			float volume = m_AudioPlayer->GetVolume();
			ImGui::SliderFloat("##AudioVolumeController", &volume, 0.f, 1.0f);
			m_AudioPlayer->SetVolume(volume);
			ImGui::EndPopup();
		}

		if (ImGui::Button(ICON_FA_VOLUME "##AudioVolumeButton"))
			ImGui::OpenPopup("AudioViewerVolumeControllerPopup");
		
	}
}
