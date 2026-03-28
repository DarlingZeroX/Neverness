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
#include "VGGalgameCore/Interface/IGameObject.h"
#include "VGGalgameCore/Include/GalGameRuntimeState.h"

namespace VisionGal::GalGame
{
	enum class GalSpritePosition
	{
		Center,
		Left,
		Right,
	};

	class GalSprite : public IGalSprite
	{
	public:
		GalSprite(const std::string& layer, const std::string& path);
		GalSprite(const GalSprite&) = delete;
		GalSprite& operator=(const GalSprite&) = delete;
		GalSprite(GalSprite&&) noexcept = default;
		GalSprite& operator=(GalSprite&&) noexcept = default;
		~GalSprite() override;

		const std::string& GetResourcePath() override;
		IGameActor* GetResourceActor() override;
		const std::string& GetResourceLayer() override;
		void SetResourceLayer(const std::string& layer) override;

		IGalSprite* Show() override;
		IGalSprite* With(const std::string& transform) override;

		Animation2DScript* Animate(const sol::table& targetValue, float duration, std::string tween, int numIterations = 1, bool alternateDirection = true) override;

		IGalSprite* SetPosX(float offset) override;
		IGalSprite* SetPosY(float offset) override;

		float GetPosX() override;
		float GetPosY() override;

		IGalSprite* SetPosOffsetX(float offset) override;
		IGalSprite* SetPosOffsetY(float offset) override;
		IGalSprite* SetScale(float scale) override;
		IGalSprite* SetScaleWidth(float scale) override;
		IGalSprite* SetScaleHeight(float scale) override;

		float GetScaleWidth() override;
		float GetScaleHeight() override;

		IGalSprite* AlignBottom() override;
		IGalSprite* AlignBottomWithTexture(Texture2D* tex) override;
		TransformComponent* GetTransformComponent() override;

		void Cut(const std::string& cut) override;

		std::string m_Path;
		std::string m_Layer;
		IGameActor* m_Actor = nullptr;
		GalGameRuntimeState* m_GalState = nullptr;
	};

	class GalAudio: public IGalAudio
	{
	public:
		GalAudio(const std::string& layer, const std::string& path);
		GalAudio(const GalAudio&) = delete;
		GalAudio& operator=(const GalAudio&) = delete;
		GalAudio(GalAudio&&) noexcept = default;
		GalAudio& operator=(GalAudio&&) noexcept = default;
		~GalAudio() override;

		const std::string& GetResourcePath() override;
		IGameActor* GetResourceActor() override;
		const std::string& GetResourceLayer() override;
		void SetResourceLayer(const std::string& layer) override;

		IGalAudio* SetLoop(bool enable) override;
		IGalAudio* Stop() override;
		bool IsPlayingAudio() override;
		bool IsLooping() override;
		IGalAudio* SetVolume(float v) override;
		float GetVolume() override;

		IGalAudio* With(const std::string& transform) override;

		std::string m_Path;
		std::string m_Layer;
		IGameActor* m_Actor = nullptr;
	};

	class GalVideo: public IGalVideo
	{
	public:
		GalVideo(const std::string& layer, const std::string& path);
		GalVideo(const GalVideo&) = delete;
		GalVideo& operator=(const GalVideo&) = delete;
		GalVideo(GalVideo&&) noexcept = default;
		GalVideo& operator=(GalVideo&&) noexcept = default;
		~GalVideo() override;

		const std::string& GetResourcePath() override;
		IGameActor* GetResourceActor() override;
		const std::string& GetResourceLayer() override;
		void SetResourceLayer(const std::string& layer) override;

		IGalVideo* SetLoop(bool enable) override;
		IGalVideo* Stop() override;
		bool IsPlaying() override;
		bool IsLooping() override;
		IGalVideo* SetVolume(float v) override;
		float GetVolume() override;

		//virtual GalAudio* With(const std::string& transform);

		std::string m_Path;
		std::string m_Layer;
		IGameActor* m_Actor = nullptr;
	};

	class GalCharacter : public IGalCharacter
	{
	public:
		struct FigureState
		{
			bool IsHide = false;
			String State = "";
			IGalSprite* Sprite = nullptr;
			IGalAudio* Voice = nullptr;
		};

		GalCharacter(const std::string& name);
		GalCharacter(const GalCharacter&) = delete;
		GalCharacter& operator=(const GalCharacter&) = delete;
		GalCharacter(GalCharacter&&) noexcept = default;
		GalCharacter& operator=(GalCharacter&&) noexcept = default;
		~GalCharacter() override = default;

		std::string GetName() override;
		void SetName(const std::string& name) override;
		void Say(const std::string& text) override;
		IGalAudio* Voice(const std::string& path) override;
		IGalAudio* GetCurrentVoice() override;
		void AddFigure(const String& state, const String& spritePath) override;

		IGalSprite* ShowFigure(const String& stateOrPath) override;
		void HideFigure() override;
		IGalSprite* GetCurrentFigure() override;
		
		void AddShowFigureCallback(sol::function callback) override;
		void AddHideFigureCallback(sol::function callback) override;
		void ClearShowFigureCallbacks() override;
		void ClearHideFigureCallbacks() override;

		std::unordered_map<String, String> m_CharacterSpriteStates;
		FigureState m_CurrentState;
		FigureState m_LastState;
		std::string m_Name;

		std::vector<sol::function> m_ShowFigureCallbacks;
		std::vector<sol::function> m_HideFigureCallbacks;
	};
}
