// https://github.com/dfranx/ImGuiColorTextEdit/tree/b66ebd70e232f0710a22730ad030e9c0aa22354b Oct 21, 2020		Fix scrolling
/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
 * Github page: https://github.com/DarlingZeroX/VisionGal
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <SDL3/SDL_scancode.h>

#include "../imgui/imgui.h"
 
namespace ImGuiTextEditor {
	ImGuiKey SDLScancodeToImGuiKey(SDL_Scancode scancode);
	bool IsValidUtf8StartByte(unsigned char c);

	enum class PaletteIndex {
		Default,                     // 默认颜色
		Keyword,                     // 关键字
		Number,                      // 数字
		String,                      // 字符串
		CharLiteral,                 // 字符常量
		Punctuation,                 // 标点符号
		Preprocessor,                // 预处理器指令
		Identifier,                  // 标识符
		KnownIdentifier,             // 已知标识符
		PreprocIdentifier,           // 预处理器标识符
		Comment,                     // 单行注释
		MultiLineComment,            // 多行注释
		Background,                  // 背景
		Cursor,                      // 光标
		Selection,                   // 选择区域
		ErrorMarker,                 // 错误标记
		Breakpoint,                  // 断点
		BreakpointOutline,           // 断点轮廓
		CurrentLineIndicator,        // 当前行指示器
		CurrentLineIndicatorOutline,// 当前行指示器轮廓
		LineNumber,                  // 行号
		CurrentLineFill,             // 当前行填充
		CurrentLineFillInactive,     // 当前行非活动填充
		CurrentLineEdge,             // 当前行边缘
		ErrorMessage,                // 错误消息
		BreakpointDisabled,          // 禁用的断点
		UserFunction,                // 用户定义的函数
		UserType,                    // 用户定义的类型
		UniformVariable,             // 统一变量
		GlobalVariable,              // 全局变量
		LocalVariable,               // 局部变量
		FunctionArgument,            // 函数参数
		Max                          // 最大值
	};

	// 快捷键ID
	enum class ShortcutID {
		Undo,                    // 撤销        CTRL+Z
		Redo,                    // 重做        CTRL+Y
		MoveUp,                  // 向上移动    UP ARROW
		SelectUp,                // 向上选择    SHIFT + UP ARROW
		MoveDown,                // 向下移动    DOWN ARROW
		SelectDown,              // 向下选择    SHIFT + DOWN ARROW
		MoveLeft,                // 向左移动    LEFT ARROW (+ SHIFT/CTRL)
		SelectLeft,              // 向左选择    SHIFT + LEFT ARROW
		MoveWordLeft,            // 向左移动一个单词  CTRL + LEFT ARROW
		SelectWordLeft,          // 向左选择一个单词  CTRL + SHIFT + LEFT ARROW
		MoveRight,               // 向右移动    RIGHT ARROW
		SelectRight,             // 向右选择    SHIFT + RIGHT ARROW
		MoveWordRight,           // 向右移动一个单词  CTRL + RIGHT ARROW
		SelectWordRight,         // 向右选择一个单词  CTRL + SHIFT + RIGHT ARROW
		MoveUpBlock,             // 向上移动一个块  PAGE UP
		SelectUpBlock,           // 向上选择一个块  SHIFT + PAGE UP
		MoveDownBlock,           // 向下移动一个块  PAGE DOWN
		SelectDownBlock,         // 向下选择一个块  SHIFT + PAGE DOWN
		MoveTop,                 // 移动到顶部    CTRL + HOME
		SelectTop,               // 选择顶部      CTRL + SHIFT + HOME
		MoveBottom,              // 移动到底部    CTRL + END
		SelectBottom,            // 选择底部      CTRL + SHIFT + END
		MoveStartLine,           // 移动到行首    HOME
		SelectStartLine,         // 选择行首      SHIFT + HOME
		MoveEndLine,             // 移动到行尾    END
		SelectEndLine,           // 选择行尾      SHIFT + END
		ForwardDelete,           // 删除右侧字符  DELETE
		ForwardDeleteWord,       // 删除右侧单词  CTRL + DELETE
		DeleteRight,             // 删除右侧      SHIFT + BACKSPACE
		BackwardDelete,          // 删除左侧字符  BACKSPACE
		BackwardDeleteWord,      // 删除左侧单词  CTRL + BACKSPACE
		DeleteLeft,              // 删除左侧      SHIFT + BACKSPACE
		OverwriteCursor,         // 覆盖光标      INSERT
		Copy,                    // 复制          CTRL+C
		Paste,                   // 粘贴          CTRL+V
		Cut,                     // 剪切          CTRL+X
		SelectAll,               // 全选          CTRL+A
		AutocompleteOpen,        // 打开自动补全  CTRL+SPACE
		AutocompleteSelect,      // 选择自动补全  TAB
		AutocompleteSelectActive,// 选择激活的自动补全  RETURN
		AutocompleteUp,          // 自动补全向上  UP ARROW
		AutocompleteDown,        // 自动补全向下  DOWN ARROW
		NewLine,                 // 新行          RETURN
		Indent,                  // 缩进          TAB
		Unindent,                // 取消缩进      SHIFT + TAB
		Find,                    // 查找          CTRL+F
		Replace,                 // 替换          CTRL+H
		FindNext,                // 查找下一个    F3
		DebugStep,               // 调试单步执行  F10
		DebugStepInto,           // 调试步入      F11
		DebugStepOut,            // 调试步出      SHIFT+F11
		DebugContinue,           // 调试继续      F5
		DebugStop,               // 停止调试      SHIFT+F5
		DebugBreakpoint,         // 调试断点      F9
		DebugJumpHere,           // 调试跳转到此处  CTRL+ALT+H
		DuplicateLine,           // 复制行        CTRL+D
		Save,                    // 保存          CTRL+S
		Count                    // 总数
	};

	// 快捷键
	struct Shortcut {
		// 0 - not used, 1 - used	[0 - 未使用，1 - 已使用]
		bool Alt;
		bool Ctrl;
		bool Shift;

		// -1 - not used, everything else: Win32 VK_ code	[-1 - 未使用，其他所有情况：Win32 VK_ 代码]
		int Key1;
		int Key2;

		Shortcut(int vk1 = -1, int vk2 = -2, bool alt = 0, bool ctrl = 0, bool shift = 0)
			: Key1(vk1)
			, Key2(vk2)
			, Alt(alt)
			, Ctrl(ctrl)
			, Shift(shift)
		{
		}
	};

	// 选择模式
	enum class SelectionMode {
		Normal,  // 普通选择模式（字符选择）
		Word,    // 单词选择模式（选择整单词）
		Line     // 行选择模式（选择整行）
	};

	// Represents a character coordinate from the user's point of view,
	// i. e. consider an uniform grid (assuming fixed-width font) on the
	// screen as it is rendered, and each cell has its own coordinate, starting from 0.
	// Tabs are counted as [1..mTabSize] count empty spaces, depending on
	// how many space is necessary to reach the next tab stop.
	// For example, coordinate (1, 5) represents the character 'B' in a line "\tABC", when mTabSize = 4,
	// because it is rendered as "    ABC" on the screen.

	// 表示从用户视角出发的字符坐标，
	// 即将屏幕渲染时，假设是固定宽度字体的均匀网格，
	// 每个单元格都有自己的坐标，从 0 开始。
	// 制表符按照 [1..mTabSize] 来计数空格，具体根据需要多少空格才能到达下一个制表位。
	// 例如，坐标 (1, 5) 表示在行 "\tABC" 中的字符 'B'，当 mTabSize = 4 时，
	// 因为它在屏幕上渲染为 "    ABC"。
	struct Coordinates {
		int mLine, mColumn;
		Coordinates()
			: mLine(0)
			, mColumn(0)
		{
		}
		Coordinates(int aLine, int aColumn)
			: mLine(aLine)
			, mColumn(aColumn)
		{
			assert(aLine >= 0);
			assert(aColumn >= 0);
		}
		static Coordinates Invalid()
		{
			static Coordinates invalid(-1, -1);
			return invalid;
		}

		bool operator==(const Coordinates& o) const
		{
			return mLine == o.mLine && mColumn == o.mColumn;
		}

		bool operator!=(const Coordinates& o) const
		{
			return mLine != o.mLine || mColumn != o.mColumn;
		}

		bool operator<(const Coordinates& o) const
		{
			if (mLine != o.mLine)
				return mLine < o.mLine;
			return mColumn < o.mColumn;
		}

		bool operator>(const Coordinates& o) const
		{
			if (mLine != o.mLine)
				return mLine > o.mLine;
			return mColumn > o.mColumn;
		}

		bool operator<=(const Coordinates& o) const
		{
			if (mLine != o.mLine)
				return mLine < o.mLine;
			return mColumn <= o.mColumn;
		}

		bool operator>=(const Coordinates& o) const
		{
			if (mLine != o.mLine)
				return mLine > o.mLine;
			return mColumn >= o.mColumn;
		}
	};

	struct Identifier {
		Identifier() {}
		Identifier(const std::string& declr)
			: mDeclaration(declr)
		{
		}

		Coordinates mLocation;
		std::string mDeclaration;
	};

	// 断点
	struct Breakpoint {
		int mLine;
		bool mEnabled = false;
		std::string mCondition;

		Breakpoint()
			: mLine(-1)
			, mEnabled(false)
		{
		}
	};

	typedef std::string String;
	typedef std::unordered_map<std::string, Identifier> Identifiers;
	typedef std::unordered_set<std::string> Keywords;
	typedef std::map<int, std::string> ErrorMarkers;
	typedef std::array<ImU32, (unsigned)PaletteIndex::Max> Palette;
	typedef uint8_t Char;

	// 字符
	struct Glyph {
		Char mChar;
		PaletteIndex mColorIndex = PaletteIndex::Default;
		bool mComment : 1;
		bool mMultiLineComment : 1;
		bool mPreprocessor : 1;

		Glyph(Char aChar, PaletteIndex aColorIndex)
			: mChar(aChar)
			, mColorIndex(aColorIndex)
			, mComment(false)
			, mMultiLineComment(false)
			, mPreprocessor(false)
		{
		}
	};

	static constexpr int LineNumberSpace = 20;
	static constexpr int DebugDataSpace = 10;

	typedef std::vector<Glyph> Line;
	typedef std::vector<Line> Lines;

	// 语言定义
	struct IMGUI_API LanguageDefinition {
		typedef std::pair<std::string, PaletteIndex> TokenRegexString;
		typedef std::vector<TokenRegexString> TokenRegexStrings;
		typedef bool (*TokenizeCallback)(const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end, PaletteIndex& paletteIndex);

		std::string mName;
		Keywords mKeywords;
		Identifiers mIdentifiers;
		Identifiers mPreprocIdentifiers;
		std::string mCommentStart, mCommentEnd, mSingleLineComment;
		char mPreprocChar;
		bool mAutoIndentation;

		TokenizeCallback mTokenize;

		TokenRegexStrings mTokenRegexStrings;

		bool mCaseSensitive;

		LanguageDefinition()
			: mPreprocChar('#')
			, mAutoIndentation(true)
			, mTokenize(nullptr)
			, mCaseSensitive(true)
		{
		}

		static const LanguageDefinition& CPlusPlus();
		static const LanguageDefinition& HLSL();
		static const LanguageDefinition& GLSL();
		static const LanguageDefinition& SPIRV();
		static const LanguageDefinition& C();
		static const LanguageDefinition& SQL();
		static const LanguageDefinition& AngelScript();
		static const LanguageDefinition& Lua();
		static const LanguageDefinition& GalGameScript();
		static const LanguageDefinition& RmlUI();

	private:
		static void m_HLSLDocumentation(Identifiers& idents);
		static void m_GLSLDocumentation(Identifiers& idents);
	};

	// 函数数据
	struct FunctionData {
		FunctionData()
		{
			LineStart = -1;
			LineEnd = -1;
		}
		FunctionData(int lineStart, int lineEnd, const std::vector<std::string>& args, const std::vector<std::string>& locals)
		{
			LineStart = lineStart;
			LineEnd = lineEnd;
			Arguments = args;
			Locals = locals;
		}
		int LineStart;
		int LineEnd;
		std::vector<std::string> Arguments;
		std::vector<std::string> Locals;
	};

	// 调试动作
	enum class DebugAction
	{
		Step,      // 单步调试，执行当前行并停在下一行
		StepInto,  // 步入，进入当前行中的函数调用
		StepOut,   // 步出，跳出当前函数并停在调用它的地方
		Continue,  // 继续，继续执行直到遇到下一个断点或程序结束
		Stop       // 停止，停止调试并结束调试会话
	};

}