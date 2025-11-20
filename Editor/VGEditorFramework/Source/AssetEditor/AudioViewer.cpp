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
#include <VGEngine/Include/Interface/Loader.h>
#include "EditorCore/Localization.h"

namespace VisionGal::Editor
{
	AudioViewer::AudioViewer(const VGPath& path)
	{
		m_Path = path;
		if (auto audioClip = LoadObject<AudioClip>(path))
		{
			m_AudioPlayer = AudioPlayer::CreatePlayer(audioClip);
			m_AudioPlayer->Play();
		}
	}

	AudioViewer::~AudioViewer()
	{
	}

	void AudioViewer::RenderUI(TaskContext& context)
	{
		ImGuiEx::ScopedStyleVar winBorder(ImGuiStyleVar_WindowBorderSize, 1.0f);
		ImGuiEx::ScopedStyleColor winbg(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 1.0f));

		if (m_AudioPlayer == nullptr)
			return;

		ImGui::SetNextWindowSize(ImVec2(1024, 768), ImGuiCond_FirstUseEver);

		bool open = true;
		//const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar;
		const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize;
		EditorText text(m_Path, ICON_FA_IMAGE);
		if (ImGui::Begin(text.c_str(), &open, windowFlags))
		{
			
			// 播放/停止场景按钮
			//if (GetSceneManager()->IsPlayMode() == false)
			if (m_AudioPlayer->IsPlaying() == false)
			{
				if (ImGui::Button(ICON_FA_PLAY "##AudioViewer"))
				{
					m_AudioPlayer->Restore();
				}
			}
			else
			{
				if (ImGui::Button(ICON_FA_STOP "##AudioViewer"))
				{
					m_AudioPlayer->Pause();
				}
			}
		}

		ImGui::End();

		if (!open)
		{
			m_AudioPlayer->Stop();
			context.IsFinished = true;
		}
	}
}
