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

#include "Galgame/StoryScriptLuaInterface.h"
#include "Galgame/GameLua.h"
#include <string>
#include "Lua/CoreLuaInterface.h"
#include "Animation/Interface/Animation2DScript.h"

namespace VisionGal::GalGame
{
	static sol::coroutine* s_StoryScriptCoroutine = nullptr;

	int StoryScriptLuaInterface::Continue()
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
				auto result = (*s_StoryScriptCoroutine)();

				if (s_StoryScriptCoroutine->error())
				{
					sol::error err = result;
					s_StoryScriptCoroutine = nullptr;
					H_LOG_ERROR(err.what());
				}
			}
		}
		catch (const sol::error& e) {
			H_LOG_ERROR(e.what());
			s_StoryScriptCoroutine = nullptr;
		}

		return 0;
	}

	void StoryScriptLuaInterface::SetStoryScriptCoroutine(sol::coroutine* co)
	{
		s_StoryScriptCoroutine = co;
	}

	sol::coroutine* StoryScriptLuaInterface::GetStoryScriptCoroutine()
	{
		return s_StoryScriptCoroutine;
	}

	void StoryScriptLuaInterface::Initialise(sol::state& lua)
	{
		sol::table galgame = lua.create_named_table("GalGame");

		Lua::CoreTypesLuaInterface::Initialize(lua);
		GalGameLuaInterface::Initialise(galgame);

		//lua.new_usertype<Transform>("GalTransform",
		//	sol::constructors<Transform()>()
		//	//"SlideDown", &Transform::SlideDown
		//);

		galgame.new_usertype<GalCharacter>("GalCharacter",
			sol::constructors<GalCharacter(const std::string&)>(),
			"Say", sol::yielding(&GalCharacter::Say),
			"name", &GalCharacter::m_Name,

			//中文
			//"创建", &GalCharacter::Create,
			//"说着", sol::yielding(&GalCharacter::Say),
			"说", sol::yielding(&GalCharacter::Say),
			"语音", &GalCharacter::Voice,
			"添加立绘", &GalCharacter::AddFigure,
			"显示立绘", &GalCharacter::ShowFigure,
			"隐藏立绘", &GalCharacter::HideFigure,
			"添加立绘显示回调", &GalCharacter::AddShowFigureCallback,
			"清除全部立绘显示回调", &GalCharacter::ClearShowFigureCallbacks,
			"添加立绘隐藏回调", &GalCharacter::AddHideFigureCallback,
			"清除全部立绘隐藏回调", &GalCharacter::ClearHideFigureCallbacks,

			"名称", & GalCharacter::m_Name,
			"当前立绘", sol::property(
				[](GalCharacter& self) -> GalSprite* { return self.GetCurrentFigure(); }
			),
			"当前语音", sol::property(
				[](GalCharacter& self) -> GalAudio* { return self.GetCurrentVoice(); }
			)
		);

		galgame.new_usertype<Animation2DScript>("Animation2DScript",
			"添加动画关键帧", &Animation2DScript::AddAnimationKeyLua
			);

		galgame.new_usertype<GalSprite>("GalSprite",
			//sol::constructors<Background(const std::string&)>(),
			"Show", &GalSprite::Show,
			"With", sol::overload(
				//static_cast<GalSprite * (GalSprite::*)(Transform)>(&GalSprite::With),
				static_cast<GalSprite * (GalSprite::*)(const std::string&)>(&GalSprite::With)
			),
			"Cut", &GalSprite::Cut,
			"SetScale", &GalSprite::SetScale,
			"scaleWidth", sol::property(
				[](GalSprite& self) -> float { return self.GetScaleWidth(); },
				[](GalSprite& self, float value) { self.SetScaleWidth(value); }
			),
			"scaleHeight", sol::property(
				[](GalSprite& self) -> float { return self.GetScaleHeight(); },
				[](GalSprite& self, float value) { self.SetScaleHeight(value); }
			),
			"AlignBottom", &GalSprite::AlignBottom,
			"path", &GalSprite::m_Path,

			//中文
			"随着", sol::overload(
				//static_cast<GalSprite * (GalSprite::*)(Transform)>(&GalSprite::With),
				static_cast<GalSprite * (GalSprite::*)(const std::string&)>(&GalSprite::With)
			),
			"开始动画", sol::overload(
				[](GalSprite& self, const sol::table& targetValue, float duration, std::string tween) { return self.Animate(targetValue, duration, tween); },
				[](GalSprite& self, const sol::table& targetValue, float duration, std::string tween, int numIterations) { return self.Animate(targetValue, duration, tween, numIterations); },
				[](GalSprite& self, const sol::table& targetValue, float duration, std::string tween, int numIterations, bool alternateDirection) { return self.Animate(targetValue, duration, tween, numIterations, alternateDirection); }
			),
			"转场", &GalSprite::Cut,
			"设置缩放", &GalSprite::SetScale,
			"设置位置偏移X", &GalSprite::SetPosOffsetX,
			"设置位置偏移Y", &GalSprite::SetPosOffsetY,
			"设置位置X", &GalSprite::SetPosX,
			"设置位置Y", &GalSprite::SetPosY,
			"设置缩放", &GalSprite::SetScale,
			"底部对齐", & GalSprite::AlignBottom,
			//属性
			"位置X", sol::property(
				[](GalSprite& self) -> float { return self.GetPosX(); },
				[](GalSprite& self, float value) { self.SetPosX(value); }
			),
			"位置Y", sol::property(
				[](GalSprite& self) -> float { return self.GetPosY(); },
				[](GalSprite& self, float value) { self.SetPosY(value); }
			),
			"宽度缩放", sol::property(
				[](GalSprite& self) -> float { return self.GetScaleWidth(); },
				[](GalSprite& self, float value) { self.SetScaleWidth(value); }
			),
			"高度缩放", sol::property(
				[](GalSprite& self) -> float { return self.GetScaleHeight(); },
				[](GalSprite& self, float value) { self.SetScaleHeight(value); }
			),
			"路径", &GalSprite::m_Path
		);

		galgame.new_usertype<GalAudio>("GalAudio",
			"设置循环播放", &GalAudio::SetLoop,
			"停止播放", &GalAudio::Stop,
			"是否正在播放", &GalAudio::IsPlayingAudio,
			"是否循环播放", &GalAudio::IsLooping,
			"设置音量", &GalAudio::SetVolume,
			"获取音量", &GalAudio::GetVolume,
			"随着", &GalAudio::With,

			"循环播放", sol::property(
				[](GalAudio& self) -> bool { return self.IsLooping(); },
				[](GalAudio& self, bool value) { self.SetLoop(value); }
			),
			"音量", sol::property(
				[](GalAudio& self) -> float { return self.GetVolume(); },
				[](GalAudio& self, float value) { self.SetVolume(value); }
			),
			"路径", &GalAudio::m_Path
		);
	}
}

