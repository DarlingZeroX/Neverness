#pragma once
#include "Game.h"
#include "ArchiveSystem.h"
#include "../Core/Core.h"
#include "../Lua/sol2/sol.hpp"

namespace VisionGal::GalGame
{
	struct DialogSystemLuaInterface
	{
		//static int Continue();

		static bool IsTypingText();

		static void FinishTyping();

		static uint GetDialogNumber();

		static String GetDialogCharacter(uint index);

		static String GetDialogText(uint index);

		static void AutoDialogue(bool enable);

		static void FastForward(bool enable);
	};

	struct ArchiveSystemLuaInterface
	{
		static SaveArchive SaveArchiveByNumber(std::string number);
		static SaveArchive GetArchiveByNumber(std::string number);
		static bool HasArchiveByNumber(std::string number);
	};

	struct GalGameEngineLuaInterface
	{
		static bool LoadArchive(const SaveArchive& archive);
	};

	//struct StoryScriptLuaInterface
	//{
	//	static void SetPlotCoroutine(sol::coroutine* co);
	////
	////	static void Initialise(lua_State* L);
	////
	//	static void Initialise(sol::state& lua);
	//};

	//struct GameScriptLuaInterface
	//{
	//	static void Initialise(sol::state_view& lua);
	//};

	struct GalGameLuaInterface
	{
		static void Initialise(sol::table& galgame);
	};
}
