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

#include "Galgame/GameLua.h"
#include <string>
#include "Galgame/GalGameEngine.h"
#include "Galgame/Game.h"
#include "Galgame/GameEngineCore.h"
#include "Galgame/ArchiveSystem.h"

namespace VisionGal::GalGame
{
	void GalGameLuaInterface::Initialise(sol::table& galgame)
	{
		// 引擎
		{
			galgame.set("GetEngine", []() -> GalGameEngine*
				{
					return dynamic_cast<GalGameEngine*>(GameEngineCore::GetCurrentEngine());
				});
			galgame.set("获取引擎", []() -> GalGameEngine*
				{
					return dynamic_cast<GalGameEngine*>(GameEngineCore::GetCurrentEngine());
				});
		}

		// 注册引擎存档类
		galgame.new_usertype<DialogueSystem>("GalGameDialogueSystem",
			//中文
			"继续对话", & DialogueSystem::ContinueDialogue,
			"完成打印对话", &DialogueSystem::FinishTyping,
			"获取对话人物", &DialogueSystem::GetDialogCharacter,
			"获取对话文本", &DialogueSystem::GetDialogText,

			"是否正在打印对话", sol::property(
				[](DialogueSystem& self) -> bool { return self.IsTypingText(); }
			),
			"对话数目", sol::property(
				[](DialogueSystem& self) -> unsigned int { return self.GetDialogNumber(); }
			),
			"自动对话", sol::property(
				[](DialogueSystem& self) -> bool { return self.IsAutoDialogue(); },
				[](DialogueSystem& self, bool value) { self.AutoDialogue(value); }
			),
			"快进", sol::property(
				[](DialogueSystem& self) -> bool { return self.IsFastForward(); },
				[](DialogueSystem& self, bool value) { self.FastForward(value); }
			),
			"快进间隔时间", sol::property(
				[](DialogueSystem& self) -> float { return self.GetFastForwardDelay(); },
				[](DialogueSystem& self, float value) { self.SetFastForwardDelay(value); }
			),
			"文字显示速度", sol::property(
				[](DialogueSystem& self) -> float { return self.GetTypingDelay(); },
				[](DialogueSystem& self, float value) { self.SetTypingDelay(value); }
			)
			//"跳到对话", & DialogueSystem::JumpToDialog
		);

		// 注册存档系统
		galgame.new_usertype<ArchiveSystem>("GalGameArchiveSystem",
			//中文
			"保存存档", &ArchiveSystem::SaveArchiveByNumber,
			"获取存档", &ArchiveSystem::GetArchiveByNumber,
			"是否存在存档", &ArchiveSystem::HasArchiveByNumber
		);

		// 注册场景管理系统
		galgame.new_usertype<LayeredSceneManager::AudioLayer>("GalGameLayeredSceneManagerAudioLayer",
			"音量", sol::property(
				[](LayeredSceneManager::AudioLayer& self) -> float { return self.GetVolume(); },
				[](LayeredSceneManager::AudioLayer& self, float value) { self.SetVolume(value); }
			)
			);
		galgame.new_usertype<LayeredSceneManager::SpriteLayer>("GalGameLayeredSceneManagerSpriteLayer");

		galgame.new_usertype<LayeredSceneManager>("GalGameLayeredSceneManager",
			"获取音频层", &LayeredSceneManager::GetAudioLayer,
			"获取精灵层", &LayeredSceneManager::GetSpriteLayer
		);

		// 注册引擎类
		galgame.new_usertype<GalGameEngine>("GalGameEngine",
			"LoadArchive", &GalGameEngine::LoadArchive,
			"DialogueSystem", sol::property(
				[](GalGameEngine& self) -> DialogueSystem* { return dynamic_cast<DialogueSystem*>(self.GetDialogueSystem()); }
			),
			"ArchiveSystem", sol::property(
				[](GalGameEngine& self) -> ArchiveSystem* { return dynamic_cast<ArchiveSystem*>(self.GetArchiveSystem()); }
			),

			//中文
			"等待", sol::yielding(&GalGameEngine::Wait),
			"转场命令", &GalGameEngine::TransitionCommand,
			"图片转场命令", &GalGameEngine::TransitionCommandWithCustomImage,
			"加载剧情脚本", &GalGameEngine::LoadStoryScriptOnUpdate,
			"加载存档", &GalGameEngine::LoadArchive,
			"创建人物", &GalGameEngine::CreateCharacter,
			"显示背景", sol::overload(
				[](GalGameEngine& self, const std::string& path) ->GalSprite* { return self.ShowSprite("Background", path); },
				[](GalGameEngine& self, const float4& color) ->GalSprite* { return self.ShowColor("Background", color); }
			),
			"显示前景", sol::overload(
				[](GalGameEngine& self, const std::string& path) ->GalSprite* { return self.ShowSprite("Foreground", path); },
				[](GalGameEngine& self, const float4& color) ->GalSprite* { return self.ShowColor("Foreground", color); }
			),
			"显示屏幕", sol::overload(
				[](GalGameEngine& self, const std::string& path) ->GalSprite* { return self.ShowSprite("Screen", path); },
				[](GalGameEngine& self, const float4& color) ->GalSprite* { return self.ShowColor("Screen", color); }
			),
			"播放背景音乐", [](GalGameEngine& self, const std::string& path) ->GalAudio* { return self.PlayAudio("BGM", path); },
			"播放效果音乐", [](GalGameEngine& self, const std::string& path) ->GalAudio* { return self.PlayAudio("Effect", path); },
			"隐藏全部人物立绘", &GalGameEngine::HideAllCharacterSprite,
			"场景截图", &GalGameEngine::CaptureSceneImage,

			//属性
			"对话系统", sol::property(
				[](GalGameEngine& self) -> DialogueSystem* { return dynamic_cast<DialogueSystem*>(self.GetDialogueSystem()); }
			),
			"存档系统", sol::property(
				[](GalGameEngine& self) -> ArchiveSystem* { return dynamic_cast<ArchiveSystem*>(self.GetArchiveSystem()); }
			),
			"场景系统", sol::property(
				[](GalGameEngine& self) -> LayeredSceneManager* { return dynamic_cast<LayeredSceneManager*>(self.GetLayeredSceneManager()); }
			)
		);

		// 注册存档类
		galgame.new_usertype<SaveArchive>("GalGameSaveArchive",
			sol::constructors<SaveArchive()>(),
			"isGalGameArchive", &SaveArchive::isGalGameArchive,
			"isValid", &SaveArchive::isValid,
			"version", &SaveArchive::version,
			"scriptPath", &SaveArchive::scriptPath,
			"line", &SaveArchive::line,
			"saveNumberString", &SaveArchive::saveNumberString,
			"date", &SaveArchive::date,
			"time", &SaveArchive::time,
			"dateTime", &SaveArchive::dateTime,
			"description", &SaveArchive::description,
			"screenshotPath", &SaveArchive::screenshotPath,

			"是否为Galgame存档", &SaveArchive::isGalGameArchive,
			"是否有效", &SaveArchive::isValid,
			"版本", &SaveArchive::version,
			"脚本路径", &SaveArchive::scriptPath,
			"行号", &SaveArchive::line,
			"存档编号字符串", &SaveArchive::saveNumberString,
			"日期", &SaveArchive::date,
			"时间", &SaveArchive::time,
			"日期时间", &SaveArchive::dateTime,
			"描述", &SaveArchive::description,
			"截图路径", &SaveArchive::screenshotPath
		);
	}
}

