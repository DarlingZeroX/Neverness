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

#include "Render/Transition.h"
#include "Engine/Manager.h"
#include "Render/Renderer.h"
//#include "Graphics/Interface/VGFX.h"
#include <VGRHI/Interface/VGFX.h>

namespace VisionGal
{
	SceneTransitionBase::SceneTransitionBase()
	{
		m_Layer = "Screen";
		m_FullQuad = FullScreenQuad::Create();
	}

	void SceneTransitionBase::Start()
	{
		m_IsBegin = true;
		m_Helper = CreateRef<TransitionHelper>(m_Duration);
	}

	void SceneTransitionBase::Finish()
	{
		m_IsBegin = false;
		m_Helper->Finish();
	}

	bool SceneTransitionBase::IsFinish()
	{
		return m_Helper->IsFinished();
	}

	float SceneTransitionBase::GetProgress() const noexcept
	{
		return m_Helper->Progress();
	}

	void SceneTransitionBase::Transition()
	{
		//ImGui::ShowDemoWindow();

		if (m_FramePrev == nullptr)
			return;

		if (m_FrameNext == nullptr)
			return;

		if (m_IsBegin == false)
			return;

		if (m_Helper->IsFinished())
		{
			Finish();
			return;
		}

		m_Helper->Update();

		// 转场渲染
		OnRenderTransition();

		VGFX::RenderMesh(m_FullQuad->GetMesh());

		// 恢复到纹理单元0（关键！）
		VGFX::BindTexture(0);
	}

	void SceneTransitionBase::SetPrevFrame(VGFX::ITexture* texture)
	{
		m_FramePrev = texture;
	}

	void SceneTransitionBase::SetNextFrame(VGFX::ITexture* texture)
	{
		m_FrameNext = texture;
	}

	void SceneTransitionBase::SetLayer(const String& layer)
	{
		m_Layer = layer;
	}

	const String& SceneTransitionBase::GetLayer()
	{
		return m_Layer;
	}

	void SceneTransitionBase::SetDuration(float duration)
	{
		m_Duration = std::max(duration, 0.f);
	}

	VGFX::ITexture* SceneTransitionBase::GetPrevFrame()
	{
		return m_FramePrev;
	}

	VGFX::ITexture* SceneTransitionBase::GetNextFrame()
	{
		return m_FrameNext; 
	}

	/////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////	DissolveSceneTransition
	/////////////////////////////////////////////////////////////////////////////////////

	DissolveSceneTransition::DissolveSceneTransition()
	{
		m_ShaderProgram = ShaderManager::Get()->GetBuiltinProgram("TransitionFade");
	}

	void DissolveSceneTransition::OnRenderTransition()
	{
		VGFX::UseProgram(m_ShaderProgram);
		VGFX::SetUniformInt("fadeType", 0); // Dissolve
		VGFX::SetTexture(0, "prevScene", GetPrevFrame());
		VGFX::SetTexture(1, "nextScene", GetNextFrame());
		VGFX::SetUniformFloat("progress", GetProgress());
	}

	/////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////	FadeSceneTransition
	/////////////////////////////////////////////////////////////////////////////////////

	FadeSceneTransition::FadeSceneTransition(FadeType type)
	{
		m_FadeType = type;
		m_ShaderProgram = ShaderManager::Get()->GetBuiltinProgram("TransitionFade");
	}
	void FadeSceneTransition::OnRenderTransition()
	{
		VGFX::UseProgram(m_ShaderProgram);

		switch (m_FadeType)
		{
		case FadeType::FadeIn:
			VGFX::SetUniformInt("fadeType", 1); // Fade In
			break;
		case FadeType::FadeOut:
			VGFX::SetUniformInt("fadeType", 2); // Fade Out
			break;
		}

		VGFX::SetTexture(0, "prevScene", GetPrevFrame());
		VGFX::SetTexture(1, "nextScene", GetNextFrame());

		VGFX::SetUniformFloat("progress", GetProgress());
	}

	/////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////	PushSceneTransition
	/////////////////////////////////////////////////////////////////////////////////////

	PushSceneTransition::PushSceneTransition(PushType type)
	{
		m_PushType = type;
		m_ShaderProgram = ShaderManager::Get()->GetBuiltinProgram("TransitionPush");
	}

	void PushSceneTransition::OnRenderTransition()
	{
		VGFX::UseProgram(m_ShaderProgram);
		switch (m_PushType)
		{
		case PushType::PushLeft:
			VGFX::SetUniformFloat2("direction", float2(-1, 0));
			break;
		case PushType::PushRight:
			VGFX::SetUniformFloat2("direction", float2(1, 0));
			break;
		case PushType::PushUp:
			VGFX::SetUniformFloat2("direction", float2(0, 1));
			break;
		case PushType::PushDown:
			VGFX::SetUniformFloat2("direction", float2(0, -1));
			break;
		}

		VGFX::SetTexture(0, "prevScene", GetPrevFrame());
		VGFX::SetTexture(1, "nextScene", GetNextFrame());
		VGFX::SetUniformFloat("progress", GetProgress());
	}

	/////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////	CustomImageSceneTransition
	/////////////////////////////////////////////////////////////////////////////////////

	CustomImageSceneTransition::CustomImageSceneTransition(const Ref<Texture2D>& texture)
	{
		m_ShaderProgram = ShaderManager::Get()->GetBuiltinProgram("TransitionCustomImage");
		m_TransitionTexture = texture;
	}

	void CustomImageSceneTransition::OnRenderTransition()
	{
		VGFX::UseProgram(m_ShaderProgram);
		VGFX::SetTexture(0, "prevScene", GetPrevFrame());
		VGFX::SetTexture(1, "nextScene", GetNextFrame());
		VGFX::SetTexture(2, "transition", m_TransitionTexture->GetTexture().get());
		VGFX::SetUniformFloat("progress", GetProgress());
	}

	void CustomImageSceneTransition::SetTransitionImage(const Ref<Texture2D>& texture)
	{
		m_TransitionTexture = texture;
	}
}
