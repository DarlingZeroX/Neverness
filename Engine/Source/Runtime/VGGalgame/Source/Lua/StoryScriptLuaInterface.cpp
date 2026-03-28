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

#include "Lua/StoryScriptLuaInterface.h"
#include "Game.h"
#include "ArchiveSystem.h"
#include "Lua/GameLua.h"
#include "VGEngine/Include/Lua/LuaInterface.h"
#include "VGEngine/Include/Animation/Interface/Animation2DScript.h"
#include <sol/sol.hpp>

#include "VGCore/Include/Core/EventBus.h"
#include "VGLuaCore/LuaErrorManager.h"

namespace VisionGal::GalGame
{
	static sol::coroutine* s_StoryScriptCoroutine = nullptr;
	static std::string s_StoryScriptPath = "";

	int StoryScriptLuaInterface::Continue(ContinueType type, int number, const std::string& str)
	{
		if (s_StoryScriptCoroutine == nullptr)
			return 0;

		if (s_StoryScriptCoroutine->status() == sol::call_status::ok) {
			s_StoryScriptCoroutine = nullptr;
		}

		if (s_StoryScriptCoroutine == nullptr)
			return 0;

		try
		{
			if (s_StoryScriptCoroutine && s_StoryScriptCoroutine->lua_state()) {

				// 调用协程
				sol::protected_function_result result;
				switch (type)
				{
				case ContinueType::None:
					result = (*s_StoryScriptCoroutine)();
					break;
				case ContinueType::Number:
					result = (*s_StoryScriptCoroutine)(number);
					break;
				case ContinueType::String:
					result = (*s_StoryScriptCoroutine)(str);
					break;
				}

				if (s_StoryScriptCoroutine->error())
				{
					sol::error err = result;
					s_StoryScriptCoroutine = nullptr;
					H_LOG_ERROR(err.what());

					int lineNumber = VGLuaInterface::ExtractErrorLineNumber(err.what());

					// 事件
					LuaScriptEvent evt;
					evt.EventType = LuaScriptEventType::ScriptError;
					evt.ScriptPath = s_StoryScriptPath;
					evt.ErrorMessage = err.what();
					evt.ErrorLineNumber = lineNumber;
					EngineEventBus::Get().OnLuaScriptEvent.Invoke(evt);
				}
			}
		}
		catch (const sol::error& e) {
			H_LOG_ERROR(e.what());
			s_StoryScriptCoroutine = nullptr;

			// 事件
			LuaScriptEvent evt;
			evt.EventType = LuaScriptEventType::ScriptError;
			evt.ScriptPath = s_StoryScriptPath;
			evt.ErrorMessage = e.what();
			evt.ErrorLineNumber = VGLuaInterface::ExtractErrorLineNumber(e.what());
			EngineEventBus::Get().OnLuaScriptEvent.Invoke(evt);
		}

		return 0;
	}

	void StoryScriptLuaInterface::ResetStoryScript()
	{
		s_StoryScriptCoroutine = nullptr;
	}

	void StoryScriptLuaInterface::SetStoryScriptCoroutine(sol::coroutine* co)
	{
		s_StoryScriptCoroutine = co;
	}

	sol::coroutine* StoryScriptLuaInterface::GetStoryScriptCoroutine()
	{
		return s_StoryScriptCoroutine;
	}

	void StoryScriptLuaInterface::SetCurrentStoryScriptPath(const std::string& path)
	{
		s_StoryScriptPath = path;
	}

	void StoryScriptLuaInterface::Initialise(sol::state& lua)
	{
		sol::table galgame = lua.create_named_table("GalGame");

		lua.open_libraries(sol::lib::base,
			sol::lib::math,
			sol::lib::string,
			sol::lib::table); // 默认已加载这些库
		VGLuaInterface::Initialise(lua);
		GalGameLuaInterface::Initialise(galgame);

		galgame.new_usertype<IGalCharacter>("IGalCharacter",
			"说", sol::yielding(&IGalCharacter::Say),
			"语音", &IGalCharacter::Voice,
			"添加立绘", &IGalCharacter::AddFigure,
			"显示立绘", &IGalCharacter::ShowFigure,
			"隐藏立绘", &IGalCharacter::HideFigure,
			"添加立绘显示回调", &IGalCharacter::AddShowFigureCallback,
			"清除全部立绘显示回调", &IGalCharacter::ClearShowFigureCallbacks,
			"添加立绘隐藏回调", &IGalCharacter::AddHideFigureCallback,
			"清除全部立绘隐藏回调", &IGalCharacter::ClearHideFigureCallbacks,
			"名称", sol::property(
				[](IGalCharacter& self) -> std::string { return self.GetName(); },
				[](IGalCharacter& self, const std::string& value) { self.SetName(value); }
			),
			"当前立绘", sol::property(
				[](IGalCharacter& self) -> IGalSprite* { return self.GetCurrentFigure(); }
			),
			"当前语音", sol::property(
				[](IGalCharacter& self) -> IGalAudio* { return self.GetCurrentVoice(); }
			)
		);

		galgame.new_usertype<Animation2DScript>("Animation2DScript",
			"添加动画关键帧", &Animation2DScript::AddAnimationKeyLua
			);

		galgame.new_usertype<IGalSprite>("IGalSprite",
			//中文
			"随着", &IGalSprite::With,
			"开始动画", sol::overload(
				[](IGalSprite& self, const sol::table& targetValue, float duration, std::string tween) { return self.Animate(targetValue, duration, tween); },
				[](IGalSprite& self, const sol::table& targetValue, float duration, std::string tween, int numIterations) { return self.Animate(targetValue, duration, tween, numIterations); },
				[](IGalSprite& self, const sol::table& targetValue, float duration, std::string tween, int numIterations, bool alternateDirection) { return self.Animate(targetValue, duration, tween, numIterations, alternateDirection); }
			),
			"转场", &IGalSprite::Cut,
			"设置缩放", &IGalSprite::SetScale,
			"设置位置偏移X", &IGalSprite::SetPosOffsetX,
			"设置位置偏移Y", &IGalSprite::SetPosOffsetY,
			"设置位置X", &IGalSprite::SetPosX,
			"设置位置Y", &IGalSprite::SetPosY,
			"底部对齐", & IGalSprite::AlignBottom,
			//属性
			"位置X", sol::property(
				[](IGalSprite& self) -> float { return self.GetPosX(); },
				[](IGalSprite& self, float value) { self.SetPosX(value); }
			),
			"位置Y", sol::property(
				[](IGalSprite& self) -> float { return self.GetPosY(); },
				[](IGalSprite& self, float value) { self.SetPosY(value); }
			),
			"宽度缩放", sol::property(
				[](IGalSprite& self) -> float { return self.GetScaleWidth(); },
				[](IGalSprite& self, float value) { self.SetScaleWidth(value); }
			),
			"高度缩放", sol::property(
				[](IGalSprite& self) -> float { return self.GetScaleHeight(); },
				[](IGalSprite& self, float value) { self.SetScaleHeight(value); }
			),
			"路径",sol::property(
				[](IGalSprite& self) -> const std::string& { return self.GetResourcePath(); }
			)
		);

		galgame.new_usertype<IGalAudio>("IGalAudio",
			"设置循环播放", &IGalAudio::SetLoop,
			"停止播放", &IGalAudio::Stop,
			"是否正在播放", &IGalAudio::IsPlayingAudio,
			"是否循环播放", &IGalAudio::IsLooping,
			"设置音量", &IGalAudio::SetVolume,
			"获取音量", &IGalAudio::GetVolume,
			"随着", &IGalAudio::With,
			"循环播放", sol::property(
				[](IGalAudio& self) -> bool { return self.IsLooping(); },
				[](IGalAudio& self, bool value) { self.SetLoop(value); }
			),
			"音量", sol::property(
				[](IGalAudio& self) -> float { return self.GetVolume(); },
				[](IGalAudio& self, float value) { self.SetVolume(value); }
			),
			"路径", sol::property(
				[](IGalAudio& self) -> const std::string& { return self.GetResourcePath(); }
			)
		);

		galgame.new_usertype<IGalVideo>("IGalVideo",
			"设置循环播放", &IGalVideo::SetLoop,
			"停止播放", &IGalVideo::Stop,
			"是否正在播放", &IGalVideo::IsPlaying,
			"是否循环播放", &IGalVideo::IsLooping,
			"设置音量", &IGalVideo::SetVolume,
			"获取音量", &IGalVideo::GetVolume,

			"循环播放", sol::property(
				[](IGalVideo& self) -> bool { return self.IsLooping(); },
				[](IGalVideo& self, bool value) { self.SetLoop(value); }
			),
			"音量", sol::property(
				[](IGalVideo& self) -> float { return self.GetVolume(); },
				[](IGalVideo& self, float value) { self.SetVolume(value); }
			),
			"路径", sol::property(
				[](IGalVideo& self) -> const std::string& { return self.GetResourcePath(); }
			)
		);
	}
}

