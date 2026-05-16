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

#include "Input/HKeyboardBase.h"
#include <unordered_map>

//======================================================================================
// HKBState
//======================================================================================

namespace NN::Core
{
	static std::unordered_map<HKeycode, std::string> HKEYCODE_MAP_STRING =
	{
		{HKeycode::None							,"None"					  },

		{HKeycode::Tab 							,"Tab" 					  },

		{HKeycode::Enter 						 ,"Enter" 				  },

		{HKeycode::Pause 						 ,"Pause" 				  },
		{HKeycode::CapsLock 					 ,"CapsLock" 			  },

		{HKeycode::Escape 						 ,"Escape" 				  },

		{HKeycode::LShift						 ,"LShift"				  },
		{HKeycode::LControl						 ,"LControl"			  },
		{HKeycode::LWindows						 ,"LWindows"			  },
		{HKeycode::LAlt							 ,"LAlt"				  },
		{HKeycode::RShift						 ,"RShift"				  },
		{HKeycode::RControl						 ,"RControl"			  },
		{HKeycode::RWindows						 ,"RWindows"			  },
		{HKeycode::RAlt							 ,"RAlt"				  },

		{HKeycode::Space 						 ,"Space" 				  },
		{HKeycode::PageUp 						 ,"PageUp" 				  },
		{HKeycode::PageDown 					 ,"PageDown" 			  },
		{HKeycode::End 							,"End" 					  },
		{HKeycode::Home 						 ,"Home" 				  },
		{HKeycode::Left 						 ,"Left" 				  },
		{HKeycode::Up 							 ,"Up" 					  },
		{HKeycode::Right						 ,"Right"				  },
		{HKeycode::Down							,"Down"					  },
		{HKeycode::PrintScreen 					,"PrintScreen" 			  },
		{HKeycode::Insert 						 ,"Insert" 				  },
		{HKeycode::Delete 						 ,"Delete" 				  },

		{HKeycode::D0							 ,"D0"					  },
		{HKeycode::D1							 ,"D1"					  },
		{HKeycode::D2							 ,"D2"					  },
		{HKeycode::D3							 ,"D3"					  },
		{HKeycode::D4							 ,"D4"					  },
		{HKeycode::D5							 ,"D5"					  },
		{HKeycode::D6							 ,"D6"					  },
		{HKeycode::D7							 ,"D7"					  },
		{HKeycode::D8							 ,"D8"					  },
		{HKeycode::D9							 ,"D9"					  },

		{HKeycode::A 							 ,"A" 					  },
		{HKeycode::B 							 ,"B" 					  },
		{HKeycode::C 							 ,"C" 					  },
		{HKeycode::D 							 ,"D" 					  },
		{HKeycode::E 							 ,"E" 					  },
		{HKeycode::F 							 ,"F" 					  },
		{HKeycode::G 							 ,"G" 					  },
		{HKeycode::H 							 ,"H" 					  },
		{HKeycode::I 							 ,"I" 					  },
		{HKeycode::J 							 ,"J" 					  },
		{HKeycode::K 							 ,"K" 					  },
		{HKeycode::L 							 ,"L" 					  },
		{HKeycode::M 							 ,"M" 					  },
		{HKeycode::N 							 ,"N" 					  },
		{HKeycode::O 							 ,"O" 					  },
		{HKeycode::P 							 ,"P" 					  },
		{HKeycode::Q 							 ,"Q" 					  },
		{HKeycode::R 							 ,"R" 					  },
		{HKeycode::S 							 ,"S" 					  },
		{HKeycode::T 							 ,"T" 					  },
		{HKeycode::U 							 ,"U" 					  },
		{HKeycode::V 							 ,"V" 					  },
		{HKeycode::W 							 ,"W" 					  },
		{HKeycode::X 							 ,"X" 					  },
		{HKeycode::Y 							 ,"Y" 					  },
		{HKeycode::Z 							 ,"Z" 					  },

		{HKeycode::F1 							 ,"F1" 					  },
		{HKeycode::F2 							 ,"F2" 					  },
		{HKeycode::F3 							 ,"F3" 					  },
		{HKeycode::F4 							 ,"F4" 					  },
		{HKeycode::F5 							 ,"F5" 					  },
		{HKeycode::F6 							 ,"F6" 					  },
		{HKeycode::F7 							 ,"F7" 					  },
		{HKeycode::F8 							 ,"F8" 					  },
		{HKeycode::F9 							 ,"F9" 					  },
		{HKeycode::F10							 ,"F10"					  },
		{HKeycode::F11							 ,"F11"					  },
		{HKeycode::F12							 ,"F12"					  },
		{HKeycode::F13							 ,"F13"					  },
		{HKeycode::F14							 ,"F14"					  },
		{HKeycode::F15							 ,"F15"					  },
		{HKeycode::F16							 ,"F16"					  },
		{HKeycode::F17							 ,"F17"					  },
		{HKeycode::F18							 ,"F18"					  },
		{HKeycode::F19							 ,"F19"					  },
		{HKeycode::F20							 ,"F20"					  },
		{HKeycode::F21							 ,"F21"					  },
		{HKeycode::F22							 ,"F22"					  },
		{HKeycode::F23							 ,"F23"					  },
		{HKeycode::F24							 ,"F24"					  },

		{HKeycode::PrintScreen					 ,"PrintScreen"			  },
		{HKeycode::ScrollLock					 ,"ScrollLock"			  },
		{HKeycode::Pause						 ,"Pause"				  },
		{HKeycode::Insert						 ,"Insert"				  },
		{HKeycode::Home							 ,"Home"				  },
		{HKeycode::PageUp						 ,"PageUp"				  },
		{HKeycode::Delete						 ,"Delete"				  },
		{HKeycode::End							 ,"End"				 	  },
		{HKeycode::PageDown						 ,"PageDown"			  },
		{HKeycode::Left							 ,"Left"				  },
		{HKeycode::Up							 ,"Up"				 	  },
		{HKeycode::Right						 ,"Right"				  },
		{HKeycode::Down							 ,"Down"				  },

		{HKeycode::NumLock 						 ,"NumLock"				  },
		{HKeycode::KeyPadDivide 				 ,"KeyPadDivide"		  },
		{HKeycode::KeyPadMultiply 				 ,"KeyPadMultiply"		  },
		{HKeycode::KeyPadMinus 					 ,"KeyPadMinus"			  },
		{HKeycode::KeyPadPlus 					 ,"KeyPadPlus"			  },
		{HKeycode::KeyPadEnter 					 ,"KeyPadEnter"			  },
		{HKeycode::KeyPad0 						 ,"KeyPad0"				  },
		{HKeycode::KeyPad1 						 ,"KeyPad1"				  },
		{HKeycode::KeyPad2 						 ,"KeyPad2"				  },
		{HKeycode::KeyPad3 						 ,"KeyPad3"				  },
		{HKeycode::KeyPad4 						 ,"KeyPad4"				  },
		{HKeycode::KeyPad5 						 ,"KeyPad5"				  },
		{HKeycode::KeyPad6 						 ,"KeyPad6"				  },
		{HKeycode::KeyPad7 						 ,"KeyPad7"				  },
		{HKeycode::KeyPad8 						 ,"KeyPad8"				  },
		{HKeycode::KeyPad9 						 ,"KeyPad9"				  },
		{HKeycode::KeyPadPeriod 				 ,"KeyPadPeriod"		  }
	};

	std::string HKeycodeUtils::GetKeyName(HKeycode code)
	{
		if (HKEYCODE_MAP_STRING.find(code) != HKEYCODE_MAP_STRING.end())
			return HKEYCODE_MAP_STRING[code];

		return std::string();
	}

	std::unordered_map<HKeycode, std::string>& HKeycodeUtils::GetKeyNameMap()
	{
		return HKEYCODE_MAP_STRING;
	}
}
