#include "Galgame/GameLua.h"
#include <string>
#include "Galgame/GalGameEngine.h"
#include "Galgame/Game.h"
#include "Galgame/GameEngineCore.h"
#include "Galgame/ArchiveSystem.h"

namespace VisionGal::GalGame
{

	bool DialogSystemLuaInterface::IsTypingText()
	{
		return GameEngineCore::GetCurrentEngine()->GetDialogueSystem()->IsTypingText();
	}

	void DialogSystemLuaInterface::FinishTyping()
	{
		return dynamic_cast<GalGameEngine*>(GameEngineCore::GetCurrentEngine())->GetDialogueSystem()->FinishTyping();
	}

	uint DialogSystemLuaInterface::GetDialogNumber()
	{
		DialogueSystem* ds = dynamic_cast<DialogueSystem*>( GameEngineCore::GetCurrentEngine()->GetDialogueSystem() );

		return ds->GetDialogNumber();
	}

	String DialogSystemLuaInterface::GetDialogCharacter(uint index)
	{
		DialogueSystem* ds = dynamic_cast<DialogueSystem*>(GameEngineCore::GetCurrentEngine()->GetDialogueSystem());

		return ds->GetDialogCharacter(index);
	}

	String DialogSystemLuaInterface::GetDialogText(uint index)
	{
		DialogueSystem* ds = dynamic_cast<DialogueSystem*>(GameEngineCore::GetCurrentEngine()->GetDialogueSystem());

		return ds->GetDialogText(index);
	}

	void DialogSystemLuaInterface::AutoDialogue(bool enable)
	{
		DialogueSystem* ds = dynamic_cast<DialogueSystem*>(GameEngineCore::GetCurrentEngine()->GetDialogueSystem());

		ds->AutoDialogue(enable);
	}

	void DialogSystemLuaInterface::FastForward(bool enable)
	{
		DialogueSystem* ds = dynamic_cast<DialogueSystem*>(GameEngineCore::GetCurrentEngine()->GetDialogueSystem());

		ds->FastForward(enable);
	}

	SaveArchive ArchiveSystemLuaInterface::SaveArchiveByNumber(std::string number)
	{
		ArchiveSystem* as = dynamic_cast<ArchiveSystem*>(GameEngineCore::GetCurrentEngine()->GetArchiveSystem());

		return as->SaveArchiveByNumber(number);
	}

	SaveArchive ArchiveSystemLuaInterface::GetArchiveByNumber(std::string number)
	{
		ArchiveSystem* as = dynamic_cast<ArchiveSystem*>(GameEngineCore::GetCurrentEngine()->GetArchiveSystem());

		return as->GetArchiveByNumber(number);
	}

	bool ArchiveSystemLuaInterface::HasArchiveByNumber(std::string number)
	{
		ArchiveSystem* as = dynamic_cast<ArchiveSystem*>(GameEngineCore::GetCurrentEngine()->GetArchiveSystem());

		return as->HasArchiveByNumber(number);
	}

	bool GalGameEngineLuaInterface::LoadArchive(const SaveArchive& archive)
	{
		GalGameEngine* engine = dynamic_cast<GalGameEngine*>(GameEngineCore::GetCurrentEngine());
		return engine->LoadArchive(archive);
	}

	void GalGameLuaInterface::Initialise(sol::state& lua)
	{
		sol::table galgame = lua.create_named_table("GalEngine");
		
		// 引擎
		{
			sol::table engine = galgame.create_named("Engine");
			engine.set_function("LoadArchive", &GalGameEngineLuaInterface::LoadArchive);
		
			galgame.set("GetEngine", []() -> GalGameEngine*
				{
					return dynamic_cast<GalGameEngine*>(GameEngineCore::GetCurrentEngine());
				});
			galgame.set("获取引擎", []() -> GalGameEngine*
				{
					return dynamic_cast<GalGameEngine*>(GameEngineCore::GetCurrentEngine());
				});
		}

		// 对话系统
		//{
		//	sol::table dialogueSystem = galgame.create_named("DialogueSystem");
		//
		//	dialogueSystem.set_function("Continue", DialogSystemLuaInterface::Continue);
		//	dialogueSystem.set_function("IsTypingText", DialogSystemLuaInterface::IsTypingText);
		//	dialogueSystem.set_function("FinishTyping", DialogSystemLuaInterface::FinishTyping);
		//	dialogueSystem.set_function("GetDialogNumber", DialogSystemLuaInterface::GetDialogNumber);
		//	dialogueSystem.set_function("GetDialogCharacter", DialogSystemLuaInterface::GetDialogCharacter);
		//	dialogueSystem.set_function("GetDialogText", DialogSystemLuaInterface::GetDialogText);
		//	dialogueSystem.set_function("AutoDialogue", DialogSystemLuaInterface::AutoDialogue);
		//	dialogueSystem.set_function("FastForward", DialogSystemLuaInterface::FastForward);
		//}

		// 注册引擎存档类
		lua.new_usertype<DialogueSystem>("GalGameDialogueSystem",
			"ContinueDialogue", &DialogueSystem::ContinueDialogue,
			"isTypingText", sol::property(
				[](DialogueSystem& self) -> bool { return self.IsTypingText(); }
			),
			"FinishTyping", &DialogueSystem::FinishTyping,

			"GetDialogCharacter", &DialogueSystem::GetDialogCharacter,
			"GetDialogText", &DialogueSystem::GetDialogText,
			"dialogNumber", sol::property(
				[](DialogueSystem& self) -> unsigned int { return self.GetDialogNumber(); }
			),
			"autoDialogue", sol::property(
				[](DialogueSystem& self) -> bool { return self.IsAutoDialogue(); },
				[](DialogueSystem& self, bool value) { self.AutoDialogue(value); }
			),
			"fastForward", sol::property(
				[](DialogueSystem& self) -> bool { return self.IsFastForward(); },
				[](DialogueSystem& self, bool value) { self.FastForward(value); }
			),
			"fastForwardDelay", sol::property(
				[](DialogueSystem& self) -> float { return self.GetFastForwardDelay(); },
				[](DialogueSystem& self, float value) { self.SetFastForwardDelay(value); }
			),

			//中文
			"继续对话", & DialogueSystem::ContinueDialogue,
			"是否正在打印对话", sol::property(
				[](DialogueSystem& self) -> bool { return self.IsTypingText(); }
			),
			"完成打印对话", &DialogueSystem::FinishTyping,
			"对话数目", sol::property(
				[](DialogueSystem& self) -> unsigned int { return self.GetDialogNumber(); }
			),
			"获取对话人物", &DialogueSystem::GetDialogCharacter,
			"获取对话文本", &DialogueSystem::GetDialogText,
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
			)
			//"跳到对话", & DialogueSystem::JumpToDialog
		);

		// 注册存档系统
		lua.new_usertype<ArchiveSystem>("GalGameArchiveSystem",
			"SaveArchiveByNumber", &ArchiveSystem::SaveArchiveByNumber,
			"GetArchiveByNumber", &ArchiveSystem::GetArchiveByNumber,
			"HasArchiveByNumber", &ArchiveSystem::HasArchiveByNumber,
			//中文
			"保存存档", &ArchiveSystem::SaveArchiveByNumber,
			"获取存档", &ArchiveSystem::GetArchiveByNumber,
			"是否存在存档", &ArchiveSystem::HasArchiveByNumber
		);

		// 注册引擎类
		lua.new_usertype<GalGameEngine>("GalGameEngine",
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

			"对话系统", sol::property(
				[](GalGameEngine& self) -> DialogueSystem* { return dynamic_cast<DialogueSystem*>(self.GetDialogueSystem()); }
			),
			"存档系统", sol::property(
				[](GalGameEngine& self) -> ArchiveSystem* { return dynamic_cast<ArchiveSystem*>(self.GetArchiveSystem()); }
			)
		);

		//// 存档系统
		//{
		//	sol::table archiveSystem = galgame.create_named("ArchiveSystem");
		//
		//	archiveSystem.set_function("SaveArchiveByNumber", ArchiveSystemLuaInterface::SaveArchiveByNumber);
		//	archiveSystem.set_function("GetArchiveByNumber", ArchiveSystemLuaInterface::GetArchiveByNumber);
		//	archiveSystem.set_function("HasArchiveByNumber", ArchiveSystemLuaInterface::HasArchiveByNumber);
		//}

		// 注册存档类
		lua.new_usertype<SaveArchive>("GalGameSaveArchive",
			sol::constructors<SaveArchive()>(),
			"isGalGameArchive", &SaveArchive::isGalGameArchive,
			"isValid", &SaveArchive::isValid,
			"version", &SaveArchive::version,
			"scriptPath", &SaveArchive::scriptPath,
			"line", &SaveArchive::line,
			"saveNumberString", &SaveArchive::saveNumberString,
			"data", &SaveArchive::date,
			"time", &SaveArchive::time,
			"dateTime", &SaveArchive::dateTime,
			"description", &SaveArchive::description
		);
	}
}

