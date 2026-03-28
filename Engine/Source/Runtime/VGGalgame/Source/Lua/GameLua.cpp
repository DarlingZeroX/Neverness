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

#include "Lua/GameLua.h"
#include <string>
#include "GalGameEngine.h"
#include "Game.h"
#include "VGGalgameCore/Interface/GameEngineCore.h"
#include "ArchiveSystem.h"
#include "VGEngine/Include/Lua/LuaDataBridge.h"

namespace VisionGal::GalGame
{
	struct GalGameLuaInterfaceImp
	{
		static void Choice(IGalGameEngine& self, const std::string& name, const sol::table& choices)
		{
			// 将选择转换为C++数据结构
			std::vector<std::string> options;
			choices.for_each([&options](sol::object key, sol::object value) {
				options.push_back(value.as<std::string>());
				});

			dynamic_cast<StoryScriptSystem*>(self.GetStoryScriptSystem())->DoChoice(name, options);
		}

		static void FullScreenText(IGalGameEngine& self, const sol::table& texts)
		{
			// 将选择转换为C++数据结构
			std::vector<std::string> textV;
			texts.for_each([&textV](sol::object key, sol::object value) {
				textV.push_back(value.as<std::string>());
				});

			self.GetGalGameUISystem()->ShowFullScreenTextUI(textV);
		}

		static void InputText(IGalGameEngine& self, const std::string& id, const std::string& title, const std::string& button)
		{
			dynamic_cast<StoryScriptSystem*>(self.GetStoryScriptSystem())->DoInput(id, title, button);
		}
	};

	void GalGameLuaInterface::Initialise(sol::table& galgame)
	{
		// 引擎
		{
			galgame.set("GetEngine", []() -> IGalGameEngine*
				{
					return GameEngineCore::GetCurrentEngine();
				});
			galgame.set("获取引擎", []() -> IGalGameEngine*
				{
					return GameEngineCore::GetCurrentEngine();
				});
		}

		// 注册引擎存档类
		galgame.new_usertype<IDialogueSystem>("GalGameDialogueSystem",
			//中文
			"继续对话", & IDialogueSystem::ContinueDialogue,
			"完成打印对话", &IDialogueSystem::FinishTyping,
			"获取对话人物", &IDialogueSystem::GetDialogCharacter,
			"获取对话文本", &IDialogueSystem::GetDialogText,
			"添加打印回调", &IDialogueSystem::AddTypingCallback,
			"清除全部打印回调", &IDialogueSystem::ClearAllTypingCallbacks,

			"当前对话人物", sol::property(
				[](IDialogueSystem& self) -> std::string { return self.GetCurrentCharacter(); }
			),
			"当前对话文本", sol::property(
				[](IDialogueSystem& self) -> std::string { return self.GetCurrentDialogText(); }
			),
			"是否正在打印对话", sol::property(
				[](IDialogueSystem& self) -> bool { return self.IsTypingText(); }
			),
			"对话数目", sol::property(
				[](IDialogueSystem& self) -> unsigned int { return self.GetDialogNumber(); }
			),
			"自动对话", sol::property(
				[](IDialogueSystem& self) -> bool { return self.IsAutoDialogue(); },
				[](IDialogueSystem& self, bool value) { self.AutoDialogue(value); }
			),
			"快进", sol::property(
				[](IDialogueSystem& self) -> bool { return self.IsFastForward(); },
				[](IDialogueSystem& self, bool value) { self.FastForward(value); }
			),
			"快进间隔时间", sol::property(
				[](IDialogueSystem& self) -> float { return self.GetFastForwardDelay(); },
				[](IDialogueSystem& self, float value) { self.SetFastForwardDelay(value); }
			),
			"文字显示速度", sol::property(
				[](IDialogueSystem& self) -> float { return self.GetTypingDelay(); },
				[](IDialogueSystem& self, float value) { self.SetTypingDelay(value); }
			)
			//"跳到对话", & IDialogueSystem::JumpToDialog
		);

		// 注册UI系统
		galgame.new_usertype<IGalGameUISystem>("GalGameUISystem",
			// 剧情选择
			"获取当前剧情选项文本", &IGalGameUISystem::GetChoiceOptionByIndex,
			"获取当前剧情选项数量", &IGalGameUISystem::GetChoiceOptionSize,
			"选择当前剧情选项", &IGalGameUISystem::SelectCurrentChoice,
			// 全屏文本
			"获取当前全屏文本项", & IGalGameUISystem::GetFullScreenTextItem,
			"获取当前全屏文本项数量", & IGalGameUISystem::GetFullScreenTextSize,
			// 玩家输入
			"获取当前玩家输入标题", &IGalGameUISystem::GetInputTitle,
			"获取当前玩家输入按键文本", &IGalGameUISystem::GetInputButtonText,
			"确定当前输入",&IGalGameUISystem::InputSubmitted
			);

		// 注册存档系统
		galgame.new_usertype<IArchiveSystem>("GalGameArchiveSystem",
			//中文
			"保存存档", &IArchiveSystem::SaveArchiveByNumber,
			"获取存档", &IArchiveSystem::GetArchiveByNumber,
			"是否存在存档", &IArchiveSystem::HasArchiveByNumber
		);

		// 注册场景管理系统
		galgame.new_usertype<ISceneAudioLayer>("GalGameSceneAudioManagerAudioLayer",
			"音量", sol::property(
				[](ISceneAudioLayer& self) -> float { return self.GetVolume(); },
				[](ISceneAudioLayer& self, float value) { self.SetVolume(value); }
			)
			);
		galgame.new_usertype<ISceneSpriteLayer>("GalGameSceneSpriteManagerSpriteLayer");

		galgame.new_usertype<ILayeredSceneManager>("GalGameLayeredSceneManager",
			"获取音频层", &ILayeredSceneManager::GetAudioLayer,
			"获取精灵层", &ILayeredSceneManager::GetSpriteLayer
		);

		ArchiveDataContainer::InitializeLuaBinding(galgame);

		// 注册引擎类
		galgame.new_usertype<IGalGameEngine>("IGalGameEngine",
			"LoadArchive", &IGalGameEngine::LoadArchive,
			"IDialogueSystem", sol::property(
				[](IGalGameEngine& self) -> IDialogueSystem* { return dynamic_cast<IDialogueSystem*>(self.GetDialogueSystem()); }
			),
			"ArchiveSystem", sol::property(
				[](IGalGameEngine& self) -> ArchiveSystem* { return dynamic_cast<ArchiveSystem*>(self.GetArchiveSystem()); }
			),

			//中文
			"剧情选择", sol::yielding([](IGalGameEngine& self, const std::string& name, const sol::table& choices) -> void
			{
				GalGameLuaInterfaceImp::Choice(self, name, choices);
			}),
			"全屏文字", [](IGalGameEngine& self, const sol::table& text) -> void
			{
				GalGameLuaInterfaceImp::FullScreenText(self, text);
			},
			"文本输入", sol::yielding([](IGalGameEngine& self, const std::string& id, const std::string& title, const std::string button) -> void
			{
				GalGameLuaInterfaceImp::InputText(self, id, title, button);
			}),
			"等待", sol::yielding(&IGalGameEngine::Wait),
			"转场命令", &IGalGameEngine::TransitionCommand,
			"图片转场命令", &IGalGameEngine::TransitionCommandWithCustomImage,
			"加载剧情脚本", &IGalGameEngine::LoadStoryScriptOnUpdate,
			"加载存档", &IGalGameEngine::LoadArchive,
			"创建人物", &IGalGameEngine::CreateCharacter,
			"显示背景", sol::overload(
				[](IGalGameEngine& self, const std::string& path) ->IGalSprite* { return self.ShowSprite("Background", path); },
				[](IGalGameEngine& self, const float4& color) ->IGalSprite* { return self.ShowColor("Background", color); }
			),
			"显示前景", sol::overload(
				[](IGalGameEngine& self, const std::string& path) ->IGalSprite* { return self.ShowSprite("Foreground", path); },
				[](IGalGameEngine& self, const float4& color) ->IGalSprite* { return self.ShowColor("Foreground", color); }
			),
			"显示屏幕", sol::overload(
				[](IGalGameEngine& self, const std::string& path) ->IGalSprite* { return self.ShowSprite("Screen", path); },
				[](IGalGameEngine& self, const float4& color) ->IGalSprite* { return self.ShowColor("Screen", color); }
			),
			"播放背景视频", sol::overload(
				[](IGalGameEngine& self, const std::string& path) ->IGalVideo* { return self.PlayVideo("Background", path); }
			),
			"播放背景音乐", [](IGalGameEngine& self, const std::string& path) ->IGalAudio* { return self.PlayAudio("BGM", path); },
			"播放效果音乐", [](IGalGameEngine& self, const std::string& path) ->IGalAudio* { return self.PlayAudio("Effect", path); },
			"隐藏全部人物立绘", &IGalGameEngine::HideAllCharacterSprite,
			"场景截图", &IGalGameEngine::CaptureSceneImage,
			"获取数据桥", [](IGalGameEngine & self, const std::string& name)-> LuaDataBridge* { return LuaDataBridgeManager::GetInstance()->GetDataBridge(name); },

			//属性
			"对话系统", sol::property(
				[](IGalGameEngine& self) -> IDialogueSystem* { return self.GetDialogueSystem(); }
			),
			"存档系统", sol::property(
				[](IGalGameEngine& self) -> IArchiveSystem* { return self.GetArchiveSystem(); }
			),
			"场景系统", sol::property(
				[](IGalGameEngine& self) -> ILayeredSceneManager* { return self.GetLayeredSceneManager(); }
			),
			"UI系统", sol::property(
				[](IGalGameEngine& self) -> IGalGameUISystem* { return self.GetGalGameUISystem(); }
			),
			"存档数据", sol::property(
				[](IGalGameEngine& self) -> ArchiveDataContainer* { return self.GetArchiveDataContainer(); }
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

