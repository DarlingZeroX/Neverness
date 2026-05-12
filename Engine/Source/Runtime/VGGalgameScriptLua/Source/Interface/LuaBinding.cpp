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

#include "LuaBinding.h"

#include "VGGalgameCore/Include/GalGameEngineAccess.h"
#include "VGGalgameCore/Interface/ISubsystemBus.h"
#include "VGCore/Include/Core/Core.h"
#include "VGEngine/Include/Lua/LuaDataBridge.h"
#include "VGEngine/Include/Lua/LuaInterface.h"
#include "VGGalgameCore/Interface/IGameObject.h"
#include "VGGalgameCore/Interface/IGameSystem.h"
#include "VGGalgameCore/Interface/IGameEngine.h"
#include "VGGalgameCore/Interface/IScriptSubsystem.h"
#include "VGGalgameCore/Interface/IUISubsystem.h"
#include "VGGalgameCore/Interface/ISceneSubsystem.h"
#include "VGGalgameCore/Interface/IAudioSubsystem.h"
#include "VGGalgameCore/Interface/IDialogueSubsystem.h"
#include "VGGalgameCore/Interface/IArchiveSubsystem.h"

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

			self.GetSubsystemBus()->Script()->GetStoryScriptSystem()->DoChoice(name, options);
		}

		static void FullScreenText(IGalGameEngine& self, const sol::table& texts)
		{
			// 将选择转换为C++数据结构
			std::vector<std::string> textV;
			texts.for_each([&textV](sol::object key, sol::object value) {
				textV.push_back(value.as<std::string>());
				});

			self.GetSubsystemBus()->UI()->GetGalGameUISystem()->ShowFullScreenTextUI(textV);
		}

		static void InputText(IGalGameEngine& self, const std::string& id, const std::string& title, const std::string& button)
		{
			self.GetSubsystemBus()->Script()->GetStoryScriptSystem()->DoInput(id, title, button);
		}
	};

	void GalGameLuaBinding::Register(sol::state& state)
	{
		sol::table galgame = state.create_named_table("GalGame");

		// 引擎
		galgame.set("GetEngine", []() -> IGalGameEngine*
			{
				return GalGameEngineAccess::Current();
			});
		galgame.set("获取引擎", []() -> IGalGameEngine*
			{
				return GalGameEngineAccess::Current();
			});
		galgame.set_function("引擎", []() -> IGalGameEngine* {
			return GalGameEngineAccess::Current();
		});

		galgame.new_usertype<ISubsystemBus>("GalSubsystemBus",
			"Scene", &ISubsystemBus::Scene,
			"UI", &ISubsystemBus::UI,
			"Audio", &ISubsystemBus::Audio,
			"Script", &ISubsystemBus::Script,
			"Archive", &ISubsystemBus::Archive,
			"Dialogue", &ISubsystemBus::Dialogue
		);

		{
			const auto bus = []() -> ISubsystemBus* {
				IGalGameEngine* e = GalGameEngineAccess::Current();
				return e ? e->GetSubsystemBus() : nullptr;
			};
			sol::table engineScene = state.create_table();
			engineScene.set_function("ShowSprite", [bus](const std::string& layer, const std::string& path) -> IGalSprite* {
				auto* b = bus();
				return b ? b->Scene()->ShowSprite(layer, path) : nullptr;
			});
			engineScene.set_function("ShowColor", [bus](const std::string& layer, const float4& color) -> IGalSprite* {
				auto* b = bus();
				return b ? b->Scene()->ShowColor(layer, color) : nullptr;
			});
			engineScene.set_function("PlayVideo", [bus](const std::string& layer, const std::string& path) -> IGalVideo* {
				auto* b = bus();
				return b ? b->Scene()->PlayVideo(layer, path) : nullptr;
			});
			engineScene.set_function("RemoveSprite", [bus](IGalSprite* s) -> bool {
				auto* b = bus();
				return b ? b->Scene()->RemoveSprite(s) : false;
			});
			engineScene.set_function("TransitionCommand", [bus](const String& layer, const String& cmd) -> bool {
				auto* b = bus();
				return b ? b->Scene()->TransitionCommand(layer, cmd) : false;
			});
			engineScene.set_function("CreateCharacter", [bus](const String& name) -> IGalCharacter* {
				auto* b = bus();
				return b ? b->Scene()->CreateCharacter(name) : nullptr;
			});
			sol::table engineAudio = state.create_table();
			engineAudio.set_function("PlayAudio", [bus](const std::string& layer, const std::string& path) -> IGalAudio* {
				auto* b = bus();
				return b ? b->Audio()->PlayAudio(layer, path) : nullptr;
			});
			engineAudio.set_function("RemoveAudio", [bus](IGalAudio* a) -> bool {
				auto* b = bus();
				return b ? b->Audio()->RemoveAudio(a) : false;
			});
			sol::table engineScript = state.create_table();
			engineScript.set_function("LoadStoryScript", [bus](const String& path) -> bool {
				auto* b = bus();
				return b ? b->Script()->LoadStoryScript(path) : false;
			});
			engineScript.set_function("LoadStoryScriptOnUpdate", [bus](const String& path) {
				if (auto* b = bus())
					b->Script()->LoadStoryScriptOnUpdate(path);
			});
			engineScript.set_function("ReloadStoryScript", [bus]() {
				if (auto* b = bus())
					b->Script()->ReloadStoryScript();
			});
			engineScript.set_function("Wait", [bus](float d) {
				if (auto* b = bus())
					b->Script()->Wait(d);
			});
			sol::table engineRoot = state.create_table();
			engineRoot["Scene"] = engineScene;
			engineRoot["Audio"] = engineAudio;
			engineRoot["Script"] = engineScript;
			galgame["Engine"] = engineRoot;
		}
		galgame.new_usertype<IGalCharacter>("IGalCharacter",
			"说", sol::yielding(&IGalCharacter::Say),
			"语音", &IGalCharacter::Voice,
			"添加立绘", [](IGalCharacter& self, const String& state, const String& spritePath) -> void
			{
				return self.AddFigure(state, spritePath);
			},
			"显示立绘", [](IGalCharacter& self, const String& stateOrPath) -> IGalSprite*
			{
				return self.ShowFigure(stateOrPath);
			},
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

		/// Phase 7：Lua 侧优先通过子系统总线门面访问对白；与 GalGameDialogueSystem 键名对齐。
		galgame.new_usertype<IDialogueSubsystem>("GalDialogueSubsystem",
			"继续对话", [](IDialogueSubsystem& s) {
				if (auto* d = s.GetDialogueSystem())
					d->ContinueDialogue();
			},
			"完成打印对话", [](IDialogueSubsystem& s) {
				if (auto* d = s.GetDialogueSystem())
					d->FinishTyping();
			},
			"获取对话人物", [](IDialogueSubsystem& s, uint idx) -> String {
				auto* d = s.GetDialogueSystem();
				return d ? d->GetDialogCharacter(idx) : String{};
			},
			"获取对话文本", [](IDialogueSubsystem& s, uint idx) -> String {
				auto* d = s.GetDialogueSystem();
				return d ? d->GetDialogText(idx) : String{};
			},
			"添加打印回调", [](IDialogueSubsystem& s, sol::function cb) {
				if (auto* d = s.GetDialogueSystem())
					d->AddTypingCallback(cb);
			},
			"清除全部打印回调", [](IDialogueSubsystem& s) {
				if (auto* d = s.GetDialogueSystem())
					d->ClearAllTypingCallbacks();
			},
			"当前对话人物", sol::property(
				[](IDialogueSubsystem& s) -> std::string {
					auto* d = s.GetDialogueSystem();
					return d ? std::string(d->GetCurrentCharacter()) : std::string{};
				}
			),
			"当前对话文本", sol::property(
				[](IDialogueSubsystem& s) -> std::string {
					auto* d = s.GetDialogueSystem();
					return d ? std::string(d->GetCurrentDialogText()) : std::string{};
				}
			),
			"是否正在打印对话", sol::property(
				[](IDialogueSubsystem& s) -> bool {
					auto* d = s.GetDialogueSystem();
					return d && d->IsTypingText();
				}
			),
			"对话数目", sol::property(
				[](IDialogueSubsystem& s) -> unsigned int {
					auto* d = s.GetDialogueSystem();
					return d ? d->GetDialogNumber() : 0u;
				}
			),
			"自动对话", sol::property(
				[](IDialogueSubsystem& s) -> bool {
					auto* d = s.GetDialogueSystem();
					return d && d->IsAutoDialogue();
				},
				[](IDialogueSubsystem& s, bool v) {
					if (auto* d = s.GetDialogueSystem())
						d->AutoDialogue(v);
				}
			),
			"快进", sol::property(
				[](IDialogueSubsystem& s) -> bool {
					auto* d = s.GetDialogueSystem();
					return d && d->IsFastForward();
				},
				[](IDialogueSubsystem& s, bool v) {
					if (auto* d = s.GetDialogueSystem())
						d->FastForward(v);
				}
			),
			"快进间隔时间", sol::property(
				[](IDialogueSubsystem& s) -> float {
					auto* d = s.GetDialogueSystem();
					return d ? d->GetFastForwardDelay() : 0.f;
				},
				[](IDialogueSubsystem& s, float v) {
					if (auto* d = s.GetDialogueSystem())
						d->SetFastForwardDelay(v);
				}
			),
			"文字显示速度", sol::property(
				[](IDialogueSubsystem& s) -> float {
					auto* d = s.GetDialogueSystem();
					return d ? d->GetTypingDelay() : 0.f;
				},
				[](IDialogueSubsystem& s, float v) {
					if (auto* d = s.GetDialogueSystem())
						d->SetTypingDelay(v);
				}
			)
		);

		galgame.new_usertype<IArchiveSubsystem>("GalArchiveSubsystem",
			"保存存档", [](IArchiveSubsystem& s, const String& n) -> SaveArchive {
				auto* a = s.GetArchiveSystem();
				return a ? a->SaveArchiveByNumber(n) : SaveArchive{};
			},
			"获取存档", [](IArchiveSubsystem& s, const String& n) -> SaveArchive {
				auto* a = s.GetArchiveSystem();
				return a ? a->GetArchiveByNumber(n) : SaveArchive{};
			},
			"是否存在存档", [](IArchiveSubsystem& s, const String& n) -> bool {
				auto* a = s.GetArchiveSystem();
				return a && a->HasArchiveByNumber(n);
			}
		);

		galgame.new_usertype<IUISubsystem>("GalUISubsystem",
			"获取当前剧情选项文本", [](IUISubsystem& s, int i) {
				auto* u = s.GetGalGameUISystem();
				return u ? u->GetChoiceOptionByIndex(i) : std::string{};
			},
			"获取当前剧情选项数量", [](IUISubsystem& s) {
				auto* u = s.GetGalGameUISystem();
				return u ? u->GetChoiceOptionSize() : 0;
			},
			"选择当前剧情选项", [](IUISubsystem& s, int i) {
				if (auto* u = s.GetGalGameUISystem())
					u->SelectCurrentChoice(i);
			},
			"获取当前全屏文本项", [](IUISubsystem& s, int i) {
				auto* u = s.GetGalGameUISystem();
				return u ? u->GetFullScreenTextItem(i) : std::string{};
			},
			"获取当前全屏文本项数量", [](IUISubsystem& s) {
				auto* u = s.GetGalGameUISystem();
				return u ? u->GetFullScreenTextSize() : 0;
			},
			"获取当前玩家输入标题", [](IUISubsystem& s) {
				auto* u = s.GetGalGameUISystem();
				return u ? u->GetInputTitle() : std::string{};
			},
			"获取当前玩家输入按键文本", [](IUISubsystem& s) {
				auto* u = s.GetGalGameUISystem();
				return u ? u->GetInputButtonText() : std::string{};
			},
			"确定当前输入", [](IUISubsystem& s, const std::string& text) {
				if (auto* u = s.GetGalGameUISystem())
					u->InputSubmitted(text);
			}
		);

		galgame.new_usertype<ISceneSubsystem>("GalSceneSubsystem",
			"获取分层场景管理器", [](ISceneSubsystem& s) { return s.GetLayeredSceneManager(); },
			"获取音频层", [](ISceneSubsystem& s, const String& name) -> ISceneAudioLayer* {
				auto* m = s.GetLayeredSceneManager();
				return m ? m->GetAudioLayer(name) : nullptr;
			},
			"获取精灵层", [](ISceneSubsystem& s, const String& name) -> ISceneSpriteLayer* {
				auto* m = s.GetLayeredSceneManager();
				return m ? m->GetSpriteLayer(name) : nullptr;
			}
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

		// 注册引擎类（Phase 7：子系统经总线暴露，避免在引擎 userdata 上挂裸 I*System*）
		galgame.new_usertype<IGalGameEngine>("IGalGameEngine",
			"LoadArchive", &IGalGameEngine::LoadArchive,
			"SubsystemBus", sol::property(
				[](IGalGameEngine& self) -> ISubsystemBus* { return self.GetSubsystemBus(); }
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
				[](IGalGameEngine& self, const std::string& path) ->IGalSprite* { return self.ShowSprite("Background", Core::GetAssetsPathVFS() + path); },
				[](IGalGameEngine& self, const float4& color) ->IGalSprite* { return self.ShowColor("Background", color); }
			),
			"显示前景", sol::overload(
				[](IGalGameEngine& self, const std::string& path) ->IGalSprite* { return self.ShowSprite("Foreground", Core::GetAssetsPathVFS() + path); },
				[](IGalGameEngine& self, const float4& color) ->IGalSprite* { return self.ShowColor("Foreground", color); }
			),
			"显示屏幕", sol::overload(
				[](IGalGameEngine& self, const std::string& path) ->IGalSprite* { return self.ShowSprite("Screen", Core::GetAssetsPathVFS() + path); },
				[](IGalGameEngine& self, const float4& color) ->IGalSprite* { return self.ShowColor("Screen", color); }
			),
			"播放背景视频", sol::overload(
				[](IGalGameEngine& self, const std::string& path) ->IGalVideo* { return self.PlayVideo("Background", Core::GetAssetsPathVFS() + path); }
			),
			"播放背景音乐", [](IGalGameEngine& self, const std::string& path) ->IGalAudio* { return self.PlayAudio("BGM", Core::GetAssetsPathVFS() + path); },
			"播放效果音乐", [](IGalGameEngine& self, const std::string& path) ->IGalAudio* { return self.PlayAudio("Effect", Core::GetAssetsPathVFS() + path); },
			"隐藏全部人物立绘", &IGalGameEngine::HideAllCharacterSprite,
			"场景截图", &IGalGameEngine::CaptureSceneImage,
			"获取数据桥", [](IGalGameEngine & self, const std::string& name)-> LuaDataBridge* { return LuaDataBridgeManager::GetInstance()->GetDataBridge(name); },

			"对话系统", sol::property(
				[](IGalGameEngine& self) -> IDialogueSubsystem* {
					auto* b = self.GetSubsystemBus();
					return b ? b->Dialogue() : nullptr;
				}
			),
			"存档系统", sol::property(
				[](IGalGameEngine& self) -> IArchiveSubsystem* {
					auto* b = self.GetSubsystemBus();
					return b ? b->Archive() : nullptr;
				}
			),
			"场景系统", sol::property(
				[](IGalGameEngine& self) -> ISceneSubsystem* {
					auto* b = self.GetSubsystemBus();
					return b ? b->Scene() : nullptr;
				}
			),
			"UI系统", sol::property(
				[](IGalGameEngine& self) -> IUISubsystem* {
					auto* b = self.GetSubsystemBus();
					return b ? b->UI() : nullptr;
				}
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

	void GalGameLuaBinding::RegisterScript(sol::state& state)
	{
		state.open_libraries(sol::lib::base,
			sol::lib::math,
			sol::lib::string,
			sol::lib::table); // 默认已加载这些库

		VGLuaInterface::Initialise(state);
		Register(state);
	}
}
