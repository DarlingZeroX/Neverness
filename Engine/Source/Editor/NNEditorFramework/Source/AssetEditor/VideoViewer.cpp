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

#include "AssetEditor/VideoViewer.h"
#include "NNEditorFramework/Include/EditorCore/Localization.h"
#include <NNRuntimeCore/Interface/Loader.h>
#include <NNRuntimeCore/Include/Utils/TimeHelper.h>
#include <NNEngineLegacy/Include/Engine/VideoPlayer.h>

namespace NN::Editor
{
	VideoViewer::VideoViewer(const Runtime::VGPath& path)
	{
		m_Path = path;
		auto clip = MakeRef<Runtime::FVideoClip>();
		if (clip->Open(path))
		{
			m_VideoPlayer = Runtime::FVideoPlayer::CreatePlayer(clip);
			m_VideoPlayer->Play();

			m_VideoDuration = m_VideoPlayer->GetDuration();
			m_AudioDurationFormat = Runtime::TimeHelper::FloatToTimeFormatSprintf(m_VideoDuration);
		}
	}

	VideoViewer::~VideoViewer()
	{
		if (m_VideoPlayer)
		{
			m_VideoPlayer->Stop();
		}
	}

	void VideoViewer::RenderUI(TaskContext& context)
	{
		ImGuiEx::ScopedStyleVar winBorder(ImGuiStyleVar_WindowBorderSize, 1.0f);
		ImGuiEx::ScopedStyleColor winbg(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 1.0f));

		if (m_VideoPlayer == nullptr)
			return;
		m_VideoPlayer->Update();

		ImGui::SetNextWindowSize(ImVec2(1024, 768), ImGuiCond_FirstUseEver);

		bool open = true;
		//const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar;
		EditorText text(m_Path, ICON_FA_IMAGE);
		std::string windowName = text.GetText() + "##" + std::to_string(context.Index);
		if (ImGui::Begin(windowName.c_str(), &open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGuiEx::ScopedID winID(context.Index);

			RenderVideoTexture();
			RenderProgressBarUI();
			RenderPlayButtonUI();
			RenderLoopButtonUI();
			RenderVolumeButtonUI();
		}

		ImGui::End();

		if (!open)
		{
			m_VideoPlayer->Stop();
			context.IsFinished = true;
		}
	}

	void VideoViewer::RenderVideoTexture()
	{
		auto* viewportTexture = m_VideoPlayer->GetVideoTexture();
		auto actualSize = ImVec2(1024, 768);
		auto texSize = ImVec2(viewportTexture->GetDesc().Width, viewportTexture->GetDesc().Height);
		ImVec2 size;

		float actualAspect = actualSize.y / actualSize.x;
		float textureAspect = float(texSize.y) / float(texSize.x);

		if (texSize.x < actualSize.x && texSize.y < actualSize.y)
		{
			size = texSize;
		}
		else
		{
			if (textureAspect > actualAspect)
			{
				size.y = actualSize.y;
				size.x = texSize.x / (float(texSize.y) / float(768.0f));
			}
			else
			{
				size.x = actualSize.x;
				size.y = texSize.y / (float(texSize.x) / float(1024.0f));
			}
		}

		if (viewportTexture != nullptr)
		{
			m_CurrentVideoUISizeX = size.x;
			m_CurrentVideoUISizeY = size.y;
			ImGuiEx::ImageGL(viewportTexture->GetShaderResourceView(), size.x, size.y);
		}
	}

	void VideoViewer::RenderProgressBarUI()
	{
		auto* viewportTexture = m_VideoPlayer->GetVideoTexture();
		if (viewportTexture == nullptr)
			return;

		ImGui::Text(Runtime::TimeHelper::FloatToTimeFormatSprintf(m_CurrentPlayTime).c_str());
		ImGui::SameLine();
		ImGui::SetNextItemWidth(std::max(m_CurrentVideoUISizeX - 100, 100));
		ImGui::SliderFloat("##VideoTimeControl", &m_CurrentPlayTime, 0.f, m_VideoDuration);
		//std::cout << m_CurrentPlayTime << std::endl;
		if (ImGui::IsItemDeactivatedAfterEdit()) {	// 当鼠标释放滑条时触发 Seek
			m_VideoPlayer->Seek(m_CurrentPlayTime);
		}
		if (!ImGui::IsItemActive()) {	// 当用户没有拖动滑条时，同步播放进度
			m_CurrentPlayTime = m_VideoPlayer->GetPlaybackTime();
		}
		ImGui::SameLine();
		ImGui::Text(m_AudioDurationFormat.c_str());
	}

	void VideoViewer::RenderPlayButtonUI()
	{
		// 让按钮居中
		{
			float windowWidth = ImGui::GetWindowSize().x;
			float x = (windowWidth) * 0.5f;
			ImGui::SetCursorPosX(x);
		}

		// 播放/停止场景按钮
		if (m_VideoPlayer->IsPlaying() == false)
		{
			if (ImGui::Button(ICON_FA_PLAY "##VideoViewer"))
			{
				m_VideoPlayer->Restore();
			}
		}
		else
		{
			if (ImGui::Button(ICON_FA_PAUSE "##VideoViewer"))
			{
				m_VideoPlayer->Pause();
			}
		}
	}

	void VideoViewer::RenderLoopButtonUI()
	{
		ImGui::SameLine();

		// 播放/停止场景按钮
		if (m_VideoPlayer->IsLooping() == false)
		{
			if (ImGui::Button(ICON_FA_REPEAT "##VideoViewerLoopButton"))
			{
				m_VideoPlayer->SetLoop(true);
			}
		}
		else
		{
			if (ImGui::Button(ICON_FA_UNLINK "##VideoViewerLoopButton"))
			{
				m_VideoPlayer->SetLoop(false);
			}
		}
	}

	void VideoViewer::RenderVolumeButtonUI()
	{
		ImGui::SameLine();

		if (ImGui::BeginPopupContextItem("VideoViewerVolumeControllerPopup"))
		{
			float volume = m_VideoPlayer->GetVolume();
			ImGui::SliderFloat("##VideoVolumeController", &volume, 0.f, 1.0f);
			m_VideoPlayer->SetVolume(volume);
			ImGui::EndPopup();
		}

		if (ImGui::Button(ICON_FA_VOLUME "##VideoVolumeButton"))
			ImGui::OpenPopup("VideoViewerVolumeControllerPopup");
	}
}
