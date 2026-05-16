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
#include "../Scene/Components.h"
#include "NNRuntimeCore/Interface/RenderInterface.h"
#include "NNRuntimeCore/Include/Utils/TransitionHelper.h"
#include <NNRuntimeRHI/Include/OpenGL/RenderTarget.h>

namespace VisionGal
{
	class SceneTransitionBase: public ISceneTransition
	{
	public:
		SceneTransitionBase();
		SceneTransitionBase(const SceneTransitionBase&) = delete;
		SceneTransitionBase& operator=(const SceneTransitionBase&) = delete;
		SceneTransitionBase(SceneTransitionBase&&) noexcept = default;
		SceneTransitionBase& operator=(SceneTransitionBase&&) noexcept = default;
		~SceneTransitionBase() override = default;
	public:
		void Transition() override;

		void SetPrevFrame(VGFX::ITexture* texture) override;
		void SetNextFrame(VGFX::ITexture* texture) override;

		void SetLayer(const String& layer) override;
		const String& GetLayer() override;
		void SetDuration(float duration) override;
		void Start() override;
		void Finish() override;
		bool IsFinish() override;
		float GetProgress() const noexcept override;
	protected:
		virtual void OnRenderTransition() = 0;
		VGFX::ITexture* GetPrevFrame();
		VGFX::ITexture* GetNextFrame();
		 
	private:
		static Ref<OpenGL::RenderTarget2D> s_RenderTargetFinal;

		VGFX::ITexture* m_FramePrev = nullptr;
		VGFX::ITexture* m_FrameNext = nullptr;

		Ref<FullScreenQuad> m_FullQuad;

		Ref<TransitionHelper> m_Helper;
		bool m_IsBegin = false;
		String m_Layer = "Screen";
		float m_Duration = 0.f;
	};

	/// 溶解转场
	class DissolveSceneTransition: public  SceneTransitionBase
	{
	public:
		DissolveSceneTransition();
		DissolveSceneTransition(const DissolveSceneTransition&) = delete;
		DissolveSceneTransition& operator=(const DissolveSceneTransition&) = delete;
		DissolveSceneTransition(DissolveSceneTransition&&) noexcept = default;
		DissolveSceneTransition& operator=(DissolveSceneTransition&&) noexcept = default;
		~DissolveSceneTransition() override = default;

		void OnRenderTransition() override;
	private:
		VGFX::IShaderProgram* m_ShaderProgram;
	};

	/// 淡入淡出转场
	class FadeSceneTransition: public SceneTransitionBase
	{
	public:
		enum class FadeType
		{
			None = 0,		// No Fade
			FadeIn = 1,
			FadeOut = 2
		};

		FadeSceneTransition(FadeType type);
		FadeSceneTransition(const FadeSceneTransition&) = delete;
		FadeSceneTransition& operator=(const FadeSceneTransition&) = delete;
		FadeSceneTransition(FadeSceneTransition&&) noexcept = default;
		FadeSceneTransition& operator=(FadeSceneTransition&&) noexcept = default;
		~FadeSceneTransition() override = default;

		void OnRenderTransition() override;
	private:
		//static Ref<VGFX::IShaderProgram> s_ShaderProgram;
		VGFX::IShaderProgram* m_ShaderProgram;
		FadeType m_FadeType;
	};

	/// 推入转场
	class PushSceneTransition : public SceneTransitionBase
	{
	public:
		enum class PushType
		{
			None = 0,		// No Fade
			PushLeft = 1,
			PushRight = 2,
			PushUp = 3,
			PushDown = 4,
		};

		PushSceneTransition(PushType type);
		PushSceneTransition(const PushSceneTransition&) = delete;
		PushSceneTransition& operator=(const PushSceneTransition&) = delete;
		PushSceneTransition(PushSceneTransition&&) noexcept = default;
		PushSceneTransition& operator=(PushSceneTransition&&) noexcept = default;
		~PushSceneTransition() override = default;

		void OnRenderTransition() override;
	private:
		VGFX::IShaderProgram* m_ShaderProgram;
		PushType m_PushType;
	};

	/// 自定义图片转场
	class CustomImageSceneTransition: public SceneTransitionBase
	{
	public:
		CustomImageSceneTransition(const Ref<Texture2D>& texture);
		CustomImageSceneTransition(const CustomImageSceneTransition&) = delete;
		CustomImageSceneTransition& operator=(const CustomImageSceneTransition&) = delete;
		CustomImageSceneTransition(CustomImageSceneTransition&&) noexcept = default;
		CustomImageSceneTransition& operator=(CustomImageSceneTransition&&) noexcept = default;
		~CustomImageSceneTransition() override = default;

		void SetTransitionImage(const Ref<Texture2D>& texture);
		void OnRenderTransition() override;
	private:
		VGFX::IShaderProgram* m_ShaderProgram;
		Ref<Texture2D> m_TransitionTexture;
	};
}
