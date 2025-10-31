#pragma once
#include "Interface.h"
#include "../Animation/Interface/Animation2DScript.h"
#include "../Lua/sol2/sol.hpp"
#include "../Scene/Components.h"

namespace VisionGal::GalGame
{
	enum class GalSpritePosition
	{
		Center,
		Left,
		Right,
	};

	class GalSprite : public IGalGameSprite
	{
	public:
		GalSprite(const std::string& layer, const std::string& path);
		GalSprite(const GalSprite&) = delete;
		GalSprite& operator=(const GalSprite&) = delete;
		GalSprite(GalSprite&&) noexcept = default;
		GalSprite& operator=(GalSprite&&) noexcept = default;
		~GalSprite() override;

		const std::string& GetResourcePath() override;
		GameActor* GetResourceActor() override;
		const std::string& GetResourceLayer() override;
		void SetResourceLayer(const std::string& layer) override;

		virtual GalSprite* Show();
		virtual GalSprite* With(const std::string& transform);

		Animation2DScript* Animate(const sol::table& targetValue, float duration, std::string tween, int numIterations = 1, bool alternateDirection = true);

		virtual GalSprite* SetPosX(float offset);
		virtual GalSprite* SetPosY(float offset);

		virtual float GetPosX();
		virtual float GetPosY();

		virtual GalSprite* SetPosOffsetX(float offset);
		virtual GalSprite* SetPosOffsetY(float offset);

		virtual GalSprite* SetScale(float scale);
		virtual GalSprite* SetScaleWidth(float scale);
		virtual GalSprite* SetScaleHeight(float scale);

		virtual float GetScaleWidth();
		virtual float GetScaleHeight();

		virtual GalSprite* AlignBottom();
		virtual GalSprite* AlignBottomWithTexture(Texture2D* tex);

		virtual TransformComponent* GetTransformComponent();

		void Cut(const std::string& cut);

		std::string m_Path;
		std::string m_Layer;
		GameActor* m_Actor = nullptr;
	};

	class GalAudio: public IGalGameAudio
	{
	public:
		GalAudio(const std::string& layer, const std::string& path);
		GalAudio(const GalAudio&) = delete;
		GalAudio& operator=(const GalAudio&) = delete;
		GalAudio(GalAudio&&) noexcept = default;
		GalAudio& operator=(GalAudio&&) noexcept = default;
		~GalAudio() override;

		const std::string& GetResourcePath() override;
		GameActor* GetResourceActor() override;
		const std::string& GetResourceLayer() override;
		void SetResourceLayer(const std::string& layer) override;
		 
		// 循环播放
		GalAudio* SetLoop(bool enable);
		// 停止播放
		GalAudio* Stop();
		// 是否正在播放音频
		bool IsPlayingAudio();
		// 是否循环播放
		bool IsLooping();
		// 设置音量
		GalAudio* SetVolume(float v);
		// 获取音量
		float GetVolume();

		virtual GalAudio* With(const std::string& transform);

		std::string m_Path;
		std::string m_Layer;
		GameActor* m_Actor = nullptr;
	};

	class GalCharacter
	{
	public:
		struct FigureState
		{
			bool IsHide = false;
			String State = "";
			GalSprite* Sprite = nullptr;
			GalAudio* Voice = nullptr;
		};

		GalCharacter(const std::string& name);
		GalCharacter(const GalCharacter&) = delete;
		GalCharacter& operator=(const GalCharacter&) = delete;
		GalCharacter(GalCharacter&&) noexcept = default;
		GalCharacter& operator=(GalCharacter&&) noexcept = default;
		~GalCharacter() = default;

		static GalCharacter Create(const std::string& name);

		void Say(const std::string& text);
		GalAudio* Voice(const std::string& path);
		GalAudio* GetCurrentVoice();
		void AddFigure(const String& state, const String& spritePath);

		GalSprite* ShowFigure(const String& state);
		void HideFigure();
		GalSprite* GetCurrentFigure();
		
		void AddShowFigureCallback(sol::function callback);
		void AddHideFigureCallback(sol::function callback);
		void ClearShowFigureCallbacks();
		void ClearHideFigureCallbacks();

		std::unordered_map<String, String> m_CharacterSpriteStates;
		FigureState m_CurrentState;
		FigureState m_LastState;
		std::string m_Name;

		std::vector<sol::function> m_ShowFigureCallbacks;
		std::vector<sol::function> m_HideFigureCallbacks;
	};
}
