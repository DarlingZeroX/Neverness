// https://github.com/dfranx/ImGuiColorTextEdit	 start on Commits on Oct 21, 2020		Fix scrolling
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

#include "pch.h"
#include <string>
#include <algorithm>
#include "ImGuiColorTextEdit/TextEditorDefines.h"

namespace ImGuiTextEditor {
	 
	// TokenizeCStyleString: 解析 C 风格字符串（由双引号包围的字符串）
	// 支持转义字符（例如 \\"），并返回字符串的开始和结束位置
	//
	// 输入参数：
	//   in_begin - 字符串的开始指针
	//   in_end   - 字符串的结束指针
	//
	// 输出参数：
	//   out_begin - 解析到的 C 风格字符串的开始位置
	//   out_end   - 解析到的 C 风格字符串的结束位置（包括结束的双引号）
	static bool TokenizeCStyleString(const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end)
	{
		const char* p = in_begin;  // 初始化指针 p 指向字符串的开始位置

		if (*p == '"')  // 如果字符串以双引号开头
		{
			p++;  // 跳过开头的双引号

			while (p < in_end)  // 在字符串范围内进行遍历
			{
				// 处理字符串结束的情况，遇到双引号时表示字符串结束
				if (*p == '"')
				{
					out_begin = in_begin;  // 设置字符串的开始位置
					out_end = p + 1;       // 设置字符串的结束位置（包括结束的双引号）
					return true;           // 返回 true，表示成功解析字符串
				}

				// 处理转义字符 \"，跳过后面的双引号字符
				if (*p == '\\' && p + 1 < in_end && p[1] == '"')
					p++;  // 跳过转义的引号字符

				p++;  // 移动指针到下一个字符
			}
		}

		return false;  // 如果没有找到有效的 C 风格字符串，返回 false
	}

	static bool TokenizeCStyleCharacterLiteral(const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end)
	{
		const char* p = in_begin;

		if (*p == '\'')
		{
			p++;

			// handle escape characters
			if (p < in_end && *p == '\\')
				p++;

			if (p < in_end)
				p++;

			// handle end of character literal
			if (p < in_end && *p == '\'')
			{
				out_begin = in_begin;
				out_end = p + 1;
				return true;
			}
		}

		return false;
	}

	static bool TokenizeCStyleIdentifier(const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end)
	{
		const char* p = in_begin;

		if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || *p == '_')
		{
			p++;

			while ((p < in_end) && ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') || *p == '_'))
				p++;

			out_begin = in_begin;
			out_end = p;
			return true;
		}

		return false;
	}

	static bool TokenizeCStyleNumber(const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end)
	{
		const char* p = in_begin;

		const bool startsWithNumber = *p >= '0' && *p <= '9';

		if (*p != '+' && *p != '-' && !startsWithNumber)
			return false;

		p++;

		bool hasNumber = startsWithNumber;

		while (p < in_end && (*p >= '0' && *p <= '9'))
		{
			hasNumber = true;

			p++;
		}

		if (hasNumber == false)
			return false;

		bool isFloat = false;
		bool isHex = false;
		bool isBinary = false;

		if (p < in_end)
		{
			if (*p == '.')
			{
				isFloat = true;

				p++;

				while (p < in_end && (*p >= '0' && *p <= '9'))
					p++;
			}
			else if (*p == 'x' || *p == 'X')
			{
				// hex formatted integer of the type 0xef80

				isHex = true;

				p++;

				while (p < in_end && ((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'f') || (*p >= 'A' && *p <= 'F')))
					p++;
			}
			else if (*p == 'b' || *p == 'B')
			{
				// binary formatted integer of the type 0b01011101

				isBinary = true;

				p++;

				while (p < in_end && (*p >= '0' && *p <= '1'))
					p++;
			}
		}

		if (isHex == false && isBinary == false)
		{
			// floating point exponent
			if (p < in_end && (*p == 'e' || *p == 'E'))
			{
				isFloat = true;

				p++;

				if (p < in_end && (*p == '+' || *p == '-'))
					p++;

				bool hasDigits = false;

				while (p < in_end && (*p >= '0' && *p <= '9'))
				{
					hasDigits = true;

					p++;
				}

				if (hasDigits == false)
					return false;
			}

			// single precision floating point type
			if (p < in_end && *p == 'f')
				p++;
		}

		if (isFloat == false)
		{
			// integer size type
			while (p < in_end && (*p == 'u' || *p == 'U' || *p == 'l' || *p == 'L'))
				p++;
		}

		out_begin = in_begin;
		out_end = p;
		return true;
	}

	static bool TokenizeCStylePunctuation(const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end)
	{
		(void)in_end;

		switch (*in_begin)
		{
		case '[':
		case ']':
		case '{':
		case '}':
		case '!':
		case '%':
		case '^':
		case '&':
		case '*':
		case '(':
		case ')':
		case '-':
		case '+':
		case '=':
		case '~':
		case '|':
		case '<':
		case '>':
		case '?':
		case ':':
		case '/':
		case ';':
		case ',':
		case '.':
			out_begin = in_begin;
			out_end = in_begin + 1;
			return true;
		}

		return false;
	}

	bool IsValidUtf8StartByte(unsigned char c)
	{
		// UTF-8编码规则：
		// 1字节: 0xxxxxxx
		// 2字节: 110xxxxx 10xxxxxx
		// 3字节: 1110xxxx 10xxxxxx 10xxxxxx
		// 4字节: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

		unsigned char first_byte = c;
		size_t char_length = 0;

		// 确定字符长度
		if ((first_byte & 0x80) == 0) {
			// 1字节字符 (ASCII)
			char_length = 1;
		}
		else if ((first_byte & 0xE0) == 0xC0) {
			// 2字节字符
			char_length = 2;
		}
		else if ((first_byte & 0xF0) == 0xE0) {
			// 3字节字符 (常见于中文)
			char_length = 3;
		}
		else if ((first_byte & 0xF8) == 0xF0) {
			// 4字节字符 (表情符号等)
			char_length = 4;
		}

		return char_length != 0;
	}

	ImGuiKey SDLScancodeToImGuiKey(SDL_Scancode scancode)
	{
		switch (scancode)
		{
			// Letters
		case SDL_SCANCODE_A: return ImGuiKey_A;
		case SDL_SCANCODE_B: return ImGuiKey_B;
		case SDL_SCANCODE_C: return ImGuiKey_C;
		case SDL_SCANCODE_D: return ImGuiKey_D;
		case SDL_SCANCODE_E: return ImGuiKey_E;
		case SDL_SCANCODE_F: return ImGuiKey_F;
		case SDL_SCANCODE_G: return ImGuiKey_G;
		case SDL_SCANCODE_H: return ImGuiKey_H;
		case SDL_SCANCODE_I: return ImGuiKey_I;
		case SDL_SCANCODE_J: return ImGuiKey_J;
		case SDL_SCANCODE_K: return ImGuiKey_K;
		case SDL_SCANCODE_L: return ImGuiKey_L;
		case SDL_SCANCODE_M: return ImGuiKey_M;
		case SDL_SCANCODE_N: return ImGuiKey_N;
		case SDL_SCANCODE_O: return ImGuiKey_O;
		case SDL_SCANCODE_P: return ImGuiKey_P;
		case SDL_SCANCODE_Q: return ImGuiKey_Q;
		case SDL_SCANCODE_R: return ImGuiKey_R;
		case SDL_SCANCODE_S: return ImGuiKey_S;
		case SDL_SCANCODE_T: return ImGuiKey_T;
		case SDL_SCANCODE_U: return ImGuiKey_U;
		case SDL_SCANCODE_V: return ImGuiKey_V;
		case SDL_SCANCODE_W: return ImGuiKey_W;
		case SDL_SCANCODE_X: return ImGuiKey_X;
		case SDL_SCANCODE_Y: return ImGuiKey_Y;
		case SDL_SCANCODE_Z: return ImGuiKey_Z;

			// Numbers
		case SDL_SCANCODE_0: return ImGuiKey_0;
		case SDL_SCANCODE_1: return ImGuiKey_1;
		case SDL_SCANCODE_2: return ImGuiKey_2;
		case SDL_SCANCODE_3: return ImGuiKey_3;
		case SDL_SCANCODE_4: return ImGuiKey_4;
		case SDL_SCANCODE_5: return ImGuiKey_5;
		case SDL_SCANCODE_6: return ImGuiKey_6;
		case SDL_SCANCODE_7: return ImGuiKey_7;
		case SDL_SCANCODE_8: return ImGuiKey_8;
		case SDL_SCANCODE_9: return ImGuiKey_9;

			// Function keys
		case SDL_SCANCODE_F1:  return ImGuiKey_F1;
		case SDL_SCANCODE_F2:  return ImGuiKey_F2;
		case SDL_SCANCODE_F3:  return ImGuiKey_F3;
		case SDL_SCANCODE_F4:  return ImGuiKey_F4;
		case SDL_SCANCODE_F5:  return ImGuiKey_F5;
		case SDL_SCANCODE_F6:  return ImGuiKey_F6;
		case SDL_SCANCODE_F7:  return ImGuiKey_F7;
		case SDL_SCANCODE_F8:  return ImGuiKey_F8;
		case SDL_SCANCODE_F9:  return ImGuiKey_F9;
		case SDL_SCANCODE_F10: return ImGuiKey_F10;
		case SDL_SCANCODE_F11: return ImGuiKey_F11;
		case SDL_SCANCODE_F12: return ImGuiKey_F12;

			// Control keys
		case SDL_SCANCODE_RETURN:    return ImGuiKey_Enter;
		case SDL_SCANCODE_ESCAPE:    return ImGuiKey_Escape;
		case SDL_SCANCODE_BACKSPACE: return ImGuiKey_Backspace;
		case SDL_SCANCODE_TAB:       return ImGuiKey_Tab;
		case SDL_SCANCODE_SPACE:     return ImGuiKey_Space;
		case SDL_SCANCODE_INSERT:    return ImGuiKey_Insert;
		case SDL_SCANCODE_DELETE:    return ImGuiKey_Delete;
		case SDL_SCANCODE_HOME:      return ImGuiKey_Home;
		case SDL_SCANCODE_END:       return ImGuiKey_End;
		case SDL_SCANCODE_PAGEUP:    return ImGuiKey_PageUp;
		case SDL_SCANCODE_PAGEDOWN:  return ImGuiKey_PageDown;
		case SDL_SCANCODE_LEFT:      return ImGuiKey_LeftArrow;
		case SDL_SCANCODE_RIGHT:     return ImGuiKey_RightArrow;
		case SDL_SCANCODE_UP:        return ImGuiKey_UpArrow;
		case SDL_SCANCODE_DOWN:      return ImGuiKey_DownArrow;

			// Modifiers
		case SDL_SCANCODE_LCTRL:  return ImGuiKey_LeftCtrl;
		case SDL_SCANCODE_LSHIFT: return ImGuiKey_LeftShift;
		case SDL_SCANCODE_LALT:   return ImGuiKey_LeftAlt;
		case SDL_SCANCODE_LGUI:   return ImGuiKey_LeftSuper;
		case SDL_SCANCODE_RCTRL:  return ImGuiKey_RightCtrl;
		case SDL_SCANCODE_RSHIFT: return ImGuiKey_RightShift;
		case SDL_SCANCODE_RALT:   return ImGuiKey_RightAlt;
		case SDL_SCANCODE_RGUI:   return ImGuiKey_RightSuper;

			// Keypad
		case SDL_SCANCODE_KP_0:         return ImGuiKey_Keypad0;
		case SDL_SCANCODE_KP_1:         return ImGuiKey_Keypad1;
		case SDL_SCANCODE_KP_2:         return ImGuiKey_Keypad2;
		case SDL_SCANCODE_KP_3:         return ImGuiKey_Keypad3;
		case SDL_SCANCODE_KP_4:         return ImGuiKey_Keypad4;
		case SDL_SCANCODE_KP_5:         return ImGuiKey_Keypad5;
		case SDL_SCANCODE_KP_6:         return ImGuiKey_Keypad6;
		case SDL_SCANCODE_KP_7:         return ImGuiKey_Keypad7;
		case SDL_SCANCODE_KP_8:         return ImGuiKey_Keypad8;
		case SDL_SCANCODE_KP_9:         return ImGuiKey_Keypad9;
		case SDL_SCANCODE_KP_PERIOD:    return ImGuiKey_KeypadDecimal;
		case SDL_SCANCODE_KP_DIVIDE:    return ImGuiKey_KeypadDivide;
		case SDL_SCANCODE_KP_MULTIPLY:  return ImGuiKey_KeypadMultiply;
		case SDL_SCANCODE_KP_MINUS:     return ImGuiKey_KeypadSubtract;
		case SDL_SCANCODE_KP_PLUS:      return ImGuiKey_KeypadAdd;
		case SDL_SCANCODE_KP_ENTER:     return ImGuiKey_KeypadEnter;
		case SDL_SCANCODE_KP_EQUALS:    return ImGuiKey_KeypadEqual;
			//case SDL_SCANCODE_KP_COMMA:     return ImGuiKey_KeypadComma;
			//case SDL_SCANCODE_KP_LEFTPAREN: return ImGuiKey_KeypadParenLeft;
			//case SDL_SCANCODE_KP_RIGHTPAREN:return ImGuiKey_KeypadParenRight;

				// Symbols (only a few are mapped in ImGuiKey)
		case SDL_SCANCODE_APOSTROPHE:  return ImGuiKey_Apostrophe;
		case SDL_SCANCODE_COMMA:       return ImGuiKey_Comma;
		case SDL_SCANCODE_MINUS:       return ImGuiKey_Minus;
		case SDL_SCANCODE_PERIOD:      return ImGuiKey_Period;
		case SDL_SCANCODE_SLASH:       return ImGuiKey_Slash;
		case SDL_SCANCODE_SEMICOLON:   return ImGuiKey_Semicolon;
		case SDL_SCANCODE_EQUALS:      return ImGuiKey_Equal;
		case SDL_SCANCODE_LEFTBRACKET: return ImGuiKey_LeftBracket;
		case SDL_SCANCODE_BACKSLASH:   return ImGuiKey_Backslash;
		case SDL_SCANCODE_RIGHTBRACKET:return ImGuiKey_RightBracket;
		case SDL_SCANCODE_GRAVE:       return ImGuiKey_GraveAccent;

			// Non-standard
		case SDL_SCANCODE_CAPSLOCK:    return ImGuiKey_CapsLock;
		case SDL_SCANCODE_SCROLLLOCK:  return ImGuiKey_ScrollLock;
		case SDL_SCANCODE_NUMLOCKCLEAR:return ImGuiKey_NumLock;
		case SDL_SCANCODE_PRINTSCREEN: return ImGuiKey_PrintScreen;
		case SDL_SCANCODE_PAUSE:       return ImGuiKey_Pause;

		default: return ImGuiKey_None;
		}
	}

	const LanguageDefinition& LanguageDefinition::CPlusPlus()
	{
		static bool inited = false;
		static LanguageDefinition langDef;
		if (!inited)
		{
			static const char* const cppKeywords[] = {
				"alignas", "alignof", "and", "and_eq", "asm", "atomic_cancel", "atomic_commit", "atomic_noexcept", "auto", "bitand", "bitor", "bool", "break", "case", "catch", "char", "char16_t", "char32_t", "class",
				"compl", "concept", "const", "constexpr", "const_cast", "continue", "decltype", "default", "delete", "do", "double", "dynamic_cast", "else", "enum", "explicit", "export", "extern", "false", "float",
				"for", "friend", "goto", "if", "import", "inline", "int", "long", "module", "mutable", "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator", "or", "or_eq", "private", "protected", "public",
				"register", "reinterpret_cast", "requires", "return", "short", "signed", "sizeof", "static", "static_assert", "static_cast", "struct", "switch", "synchronized", "template", "this", "thread_local",
				"throw", "true", "try", "typedef", "typeid", "typename", "union", "unsigned", "using", "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq"
			};
			for (auto& k : cppKeywords)
				langDef.mKeywords.insert(k);

			static const char* const identifiers[] = {
				"abort", "abs", "acos", "asin", "atan", "atexit", "atof", "atoi", "atol", "ceil", "clock", "cosh", "ctime", "div", "exit", "fabs", "floor", "fmod", "getchar", "getenv", "isalnum", "isalpha", "isdigit", "isgraph",
				"ispunct", "isspace", "isupper", "kbhit", "log10", "log2", "log", "memcmp", "modf", "pow", "printf", "sprintf", "snprintf", "putchar", "putenv", "puts", "rand", "remove", "rename", "sinh", "sqrt", "srand", "strcat", "strcmp", "strerror", "time", "tolower", "toupper",
				"std", "string", "vector", "map", "unordered_map", "set", "unordered_set", "min", "max"
			};
			for (auto& k : identifiers)
			{
				Identifier id;
				id.mDeclaration = "Built-in function";
				langDef.mIdentifiers.insert(std::make_pair(std::string(k), id));
			}

			langDef.mTokenize = [](const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end, PaletteIndex& paletteIndex) -> bool
				{
					paletteIndex = PaletteIndex::Max;

					while (in_begin < in_end && isascii(*in_begin) && isblank(*in_begin))
						in_begin++;

					if (in_begin == in_end)
					{
						out_begin = in_end;
						out_end = in_end;
						paletteIndex = PaletteIndex::Default;
					}
					else if (TokenizeCStyleString(in_begin, in_end, out_begin, out_end))
						paletteIndex = PaletteIndex::String;
					else if (TokenizeCStyleCharacterLiteral(in_begin, in_end, out_begin, out_end))
						paletteIndex = PaletteIndex::CharLiteral;
					else if (TokenizeCStyleIdentifier(in_begin, in_end, out_begin, out_end))
						paletteIndex = PaletteIndex::Identifier;
					else if (TokenizeCStyleNumber(in_begin, in_end, out_begin, out_end))
						paletteIndex = PaletteIndex::Number;
					else if (TokenizeCStylePunctuation(in_begin, in_end, out_begin, out_end))
						paletteIndex = PaletteIndex::Punctuation;

					return paletteIndex != PaletteIndex::Max;
				};

			langDef.mCommentStart = "/*";
			langDef.mCommentEnd = "*/";
			langDef.mSingleLineComment = "//";

			langDef.mCaseSensitive = true;
			langDef.mAutoIndentation = true;

			langDef.mName = "C++";

			inited = true;
		}
		return langDef;
	}

	const LanguageDefinition& LanguageDefinition::HLSL()
	{
		static bool inited = false;
		static LanguageDefinition langDef;
		if (!inited)
		{
			static const char* const keywords[] = {
				"AppendStructuredBuffer", "asm", "asm_fragment", "BlendState", "bool", "break", "Buffer", "ByteAddressBuffer", "case", "cbuffer", "centroid", "class", "column_major", "compile", "compile_fragment",
				"CompileShader", "const", "continue", "ComputeShader", "ConsumeStructuredBuffer", "default", "DepthStencilState", "DepthStencilView", "discard", "do", "double", "DomainShader", "dword", "else",
				"export", "extern", "false", "float", "for", "fxgroup", "GeometryShader", "groupshared", "half", "Hullshader", "if", "in", "inline", "inout", "InputPatch", "int", "interface", "line", "lineadj",
				"linear", "LineStream", "matrix", "min16float", "min10float", "min16int", "min12int", "min16uint", "namespace", "nointerpolation", "noperspective", "NULL", "out", "OutputPatch", "packoffset",
				"pass", "pixelfragment", "PixelShader", "point", "PointStream", "precise", "RasterizerState", "RenderTargetView", "return", "register", "row_major", "RWBuffer", "RWByteAddressBuffer", "RWStructuredBuffer",
				"RWTexture1D", "RWTexture1DArray", "RWTexture2D", "RWTexture2DArray", "RWTexture3D", "sample", "sampler", "SamplerState", "SamplerComparisonState", "shared", "snorm", "stateblock", "stateblock_state",
				"static", "string", "struct", "switch", "StructuredBuffer", "tbuffer", "technique", "technique10", "technique11", "texture", "Texture1D", "Texture1DArray", "Texture2D", "Texture2DArray", "Texture2DMS",
				"Texture2DMSArray", "Texture3D", "TextureCube", "TextureCubeArray", "true", "typedef", "triangle", "triangleadj", "TriangleStream", "uint", "uniform", "unorm", "unsigned", "vector", "vertexfragment",
				"VertexShader", "void", "volatile", "while",
				"bool1","bool2","bool3","bool4","double1","double2","double3","double4", "float1", "float2", "float3", "float4", "int1", "int2", "int3", "int4", "in", "out", "inout",
				"uint1", "uint2", "uint3", "uint4", "dword1", "dword2", "dword3", "dword4", "half1", "half2", "half3", "half4",
				"float1x1","float2x1","float3x1","float4x1","float1x2","float2x2","float3x2","float4x2",
				"float1x3","float2x3","float3x3","float4x3","float1x4","float2x4","float3x4","float4x4",
				"half1x1","half2x1","half3x1","half4x1","half1x2","half2x2","half3x2","half4x2",
				"half1x3","half2x3","half3x3","half4x3","half1x4","half2x4","half3x4","half4x4",
				"SHADERED_WEB", "SHADERED_DESKTOP", "SHADERED_VERSION"
			};
			for (auto& k : keywords)
				langDef.mKeywords.insert(k);

			m_HLSLDocumentation(langDef.mIdentifiers);

			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[ \\t]*#[ \\t]*[a-zA-Z_]+", PaletteIndex::Preprocessor));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("L?\\\"(\\\\.|[^\\\"])*\\\"", PaletteIndex::String));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("\\'\\\\?[^\\']\\'", PaletteIndex::CharLiteral));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[+-]?[0-9]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("0[0-7]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[a-zA-Z_][a-zA-Z0-9_]*", PaletteIndex::Identifier));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]", PaletteIndex::Punctuation));

			langDef.mCommentStart = "/*";
			langDef.mCommentEnd = "*/";
			langDef.mSingleLineComment = "//";

			langDef.mCaseSensitive = true;
			langDef.mAutoIndentation = true;

			langDef.mName = "HLSL";

			inited = true;
		}
		return langDef;
	}
	void LanguageDefinition::m_HLSLDocumentation(Identifiers& idents)
	{
		/* SOURCE: https://docs.microsoft.com/en-us/windows/desktop/direct3dhlsl/dx-graphics-hlsl-intrinsic-functions */

		idents.insert(std::make_pair("abort", Identifier("Terminates the current draw or dispatch call being executed.")));
		idents.insert(std::make_pair("abs", Identifier("Absolute value (per component).")));
		idents.insert(std::make_pair("acos", Identifier("Returns the arccosine of each component of x.")));
		idents.insert(std::make_pair("all", Identifier("Test if all components of x are nonzero.")));
		idents.insert(std::make_pair("AllMemoryBarrier", Identifier("Blocks execution of all threads in a group until all memory accesses have been completed.")));
		idents.insert(std::make_pair("AllMemoryBarrierWithGroupSync", Identifier("Blocks execution of all threads in a group until all memory accesses have been completed and all threads in the group have reached this call.")));
		idents.insert(std::make_pair("any", Identifier("Test if any component of x is nonzero.")));
		idents.insert(std::make_pair("asdouble", Identifier("Reinterprets a cast value into a double.")));
		idents.insert(std::make_pair("asfloat", Identifier("Convert the input type to a float.")));
		idents.insert(std::make_pair("asin", Identifier("Returns the arcsine of each component of x.")));
		idents.insert(std::make_pair("asint", Identifier("Convert the input type to an integer.")));
		idents.insert(std::make_pair("asuint", Identifier("Convert the input type to an unsigned integer.")));
		idents.insert(std::make_pair("atan", Identifier("Returns the arctangent of x.")));
		idents.insert(std::make_pair("atan2", Identifier("Returns the arctangent of of two values (x,y).")));
		idents.insert(std::make_pair("ceil", Identifier("Returns the smallest integer which is greater than or equal to x.")));
		idents.insert(std::make_pair("CheckAccessFullyMapped", Identifier("Determines whether all values from a Sample or Load operation accessed mapped tiles in a tiled resource.")));
		idents.insert(std::make_pair("clamp", Identifier("Clamps x to the range [min, max].")));
		idents.insert(std::make_pair("clip", Identifier("Discards the current pixel, if any component of x is less than zero.")));
		idents.insert(std::make_pair("cos", Identifier("Returns the cosine of x.")));
		idents.insert(std::make_pair("cosh", Identifier("Returns the hyperbolic cosine of x.")));
		idents.insert(std::make_pair("countbits", Identifier("Counts the number of bits (per component) in the input integer.")));
		idents.insert(std::make_pair("cross", Identifier("Returns the cross product of two 3D vectors.")));
		idents.insert(std::make_pair("D3DCOLORtoUBYTE4", Identifier("Swizzles and scales components of the 4D vector x to compensate for the lack of UBYTE4 support in some hardware.")));
		idents.insert(std::make_pair("ddx", Identifier("Returns the partial derivative of x with respect to the screen-space x-coordinate.")));
		idents.insert(std::make_pair("ddx_coarse", Identifier("Computes a low precision partial derivative with respect to the screen-space x-coordinate.")));
		idents.insert(std::make_pair("ddx_fine", Identifier("Computes a high precision partial derivative with respect to the screen-space x-coordinate.")));
		idents.insert(std::make_pair("ddy", Identifier("Returns the partial derivative of x with respect to the screen-space y-coordinate.")));
		idents.insert(std::make_pair("ddy_coarse", Identifier("Returns the partial derivative of x with respect to the screen-space y-coordinate.")));
		idents.insert(std::make_pair("ddy_fine", Identifier("Computes a high precision partial derivative with respect to the screen-space y-coordinate.")));
		idents.insert(std::make_pair("degrees", Identifier("Converts x from radians to degrees.")));
		idents.insert(std::make_pair("determinant", Identifier("Returns the determinant of the square matrix m.")));
		idents.insert(std::make_pair("DeviceMemoryBarrier", Identifier("Blocks execution of all threads in a group until all device memory accesses have been completed.")));
		idents.insert(std::make_pair("DeviceMemoryBarrierWithGroupSync", Identifier("Blocks execution of all threads in a group until all device memory accesses have been completed and all threads in the group have reached this call.")));
		idents.insert(std::make_pair("distance", Identifier("Returns the distance between two points.")));
		idents.insert(std::make_pair("dot", Identifier("Returns the dot product of two vectors.")));
		idents.insert(std::make_pair("dst", Identifier("Calculates a distance vector.")));
		idents.insert(std::make_pair("errorf", Identifier("Submits an error message to the information queue.")));
		idents.insert(std::make_pair("EvaluateAttributeAtCentroid", Identifier("Evaluates at the pixel centroid.")));
		idents.insert(std::make_pair("EvaluateAttributeAtSample", Identifier("Evaluates at the indexed sample location.")));
		idents.insert(std::make_pair("EvaluateAttributeSnapped", Identifier("Evaluates at the pixel centroid with an offset.")));
		idents.insert(std::make_pair("exp", Identifier("Returns the base-e exponent.")));
		idents.insert(std::make_pair("exp2", Identifier("Base 2 exponent(per component).")));
		idents.insert(std::make_pair("f16tof32", Identifier("Converts the float16 stored in the low-half of the uint to a float.")));
		idents.insert(std::make_pair("f32tof16", Identifier("Converts an input into a float16 type.")));
		idents.insert(std::make_pair("faceforward", Identifier("Returns -n * sign(dot(i, ng)).")));
		idents.insert(std::make_pair("firstbithigh", Identifier("Gets the location of the first set bit starting from the highest order bit and working downward, per component.")));
		idents.insert(std::make_pair("firstbitlow", Identifier("Returns the location of the first set bit starting from the lowest order bit and working upward, per component.")));
		idents.insert(std::make_pair("floor", Identifier("Returns the greatest integer which is less than or equal to x.")));
		idents.insert(std::make_pair("fma", Identifier("Returns the double-precision fused multiply-addition of a * b + c.")));
		idents.insert(std::make_pair("fmod", Identifier("Returns the floating point remainder of x/y.")));
		idents.insert(std::make_pair("frac", Identifier("Returns the fractional part of x.")));
		idents.insert(std::make_pair("frexp", Identifier("Returns the mantissa and exponent of x.")));
		idents.insert(std::make_pair("fwidth", Identifier("Returns abs(ddx(x)) + abs(ddy(x))")));
		idents.insert(std::make_pair("GetRenderTargetSampleCount", Identifier("Returns the number of render-target samples.")));
		idents.insert(std::make_pair("GetRenderTargetSamplePosition", Identifier("Returns a sample position (x,y) for a given sample index.")));
		idents.insert(std::make_pair("GroupMemoryBarrier", Identifier("Blocks execution of all threads in a group until all group shared accesses have been completed.")));
		idents.insert(std::make_pair("GroupMemoryBarrierWithGroupSync", Identifier("Blocks execution of all threads in a group until all group shared accesses have been completed and all threads in the group have reached this call.")));
		idents.insert(std::make_pair("InterlockedAdd", Identifier("Performs a guaranteed atomic add of value to the dest resource variable.")));
		idents.insert(std::make_pair("InterlockedAnd", Identifier("Performs a guaranteed atomic and.")));
		idents.insert(std::make_pair("InterlockedCompareExchange", Identifier("Atomically compares the input to the comparison value and exchanges the result.")));
		idents.insert(std::make_pair("InterlockedCompareStore", Identifier("Atomically compares the input to the comparison value.")));
		idents.insert(std::make_pair("InterlockedExchange", Identifier("Assigns value to dest and returns the original value.")));
		idents.insert(std::make_pair("InterlockedMax", Identifier("Performs a guaranteed atomic max.")));
		idents.insert(std::make_pair("InterlockedMin", Identifier("Performs a guaranteed atomic min.")));
		idents.insert(std::make_pair("InterlockedOr", Identifier("Performs a guaranteed atomic or.")));
		idents.insert(std::make_pair("InterlockedXor", Identifier("Performs a guaranteed atomic xor.")));
		idents.insert(std::make_pair("isfinite", Identifier("Returns true if x is finite, false otherwise.")));
		idents.insert(std::make_pair("isinf", Identifier("Returns true if x is +INF or -INF, false otherwise.")));
		idents.insert(std::make_pair("isnan", Identifier("Returns true if x is NAN or QNAN, false otherwise.")));
		idents.insert(std::make_pair("ldexp", Identifier("Returns x * 2exp")));
		idents.insert(std::make_pair("length", Identifier("Returns the length of the vector v.")));
		idents.insert(std::make_pair("lerp", Identifier("Returns x + s(y - x).")));
		idents.insert(std::make_pair("lit", Identifier("Returns a lighting vector (ambient, diffuse, specular, 1)")));
		idents.insert(std::make_pair("log", Identifier("Returns the base-e logarithm of x.")));
		idents.insert(std::make_pair("log10", Identifier("Returns the base-10 logarithm of x.")));
		idents.insert(std::make_pair("log2", Identifier("Returns the base - 2 logarithm of x.")));
		idents.insert(std::make_pair("mad", Identifier("Performs an arithmetic multiply/add operation on three values.")));
		idents.insert(std::make_pair("max", Identifier("Selects the greater of x and y.")));
		idents.insert(std::make_pair("min", Identifier("Selects the lesser of x and y.")));
		idents.insert(std::make_pair("modf", Identifier("Splits the value x into fractional and integer parts.")));
		idents.insert(std::make_pair("msad4", Identifier("Compares a 4-byte reference value and an 8-byte source value and accumulates a vector of 4 sums.")));
		idents.insert(std::make_pair("mul", Identifier("Performs matrix multiplication using x and y.")));
		idents.insert(std::make_pair("noise", Identifier("Generates a random value using the Perlin-noise algorithm.")));
		idents.insert(std::make_pair("normalize", Identifier("Returns a normalized vector.")));
		idents.insert(std::make_pair("pow", Identifier("Returns x^n.")));
		idents.insert(std::make_pair("printf", Identifier("Submits a custom shader message to the information queue.")));
		idents.insert(std::make_pair("Process2DQuadTessFactorsAvg", Identifier("Generates the corrected tessellation factors for a quad patch.")));
		idents.insert(std::make_pair("Process2DQuadTessFactorsMax", Identifier("Generates the corrected tessellation factors for a quad patch.")));
		idents.insert(std::make_pair("Process2DQuadTessFactorsMin", Identifier("Generates the corrected tessellation factors for a quad patch.")));
		idents.insert(std::make_pair("ProcessIsolineTessFactors", Identifier("Generates the rounded tessellation factors for an isoline.")));
		idents.insert(std::make_pair("ProcessQuadTessFactorsAvg", Identifier("Generates the corrected tessellation factors for a quad patch.")));
		idents.insert(std::make_pair("ProcessQuadTessFactorsMax", Identifier("Generates the corrected tessellation factors for a quad patch.")));
		idents.insert(std::make_pair("ProcessQuadTessFactorsMin", Identifier("Generates the corrected tessellation factors for a quad patch.")));
		idents.insert(std::make_pair("ProcessTriTessFactorsAvg", Identifier("Generates the corrected tessellation factors for a tri patch.")));
		idents.insert(std::make_pair("ProcessTriTessFactorsMax", Identifier("Generates the corrected tessellation factors for a tri patch.")));
		idents.insert(std::make_pair("ProcessTriTessFactorsMin", Identifier("Generates the corrected tessellation factors for a tri patch.")));
		idents.insert(std::make_pair("radians", Identifier("Converts x from degrees to radians.")));
		idents.insert(std::make_pair("rcp", Identifier("Calculates a fast, approximate, per-component reciprocal.")));
		idents.insert(std::make_pair("reflect", Identifier("Returns a reflection vector.")));
		idents.insert(std::make_pair("refract", Identifier("Returns the refraction vector.")));
		idents.insert(std::make_pair("reversebits", Identifier("Reverses the order of the bits, per component.")));
		idents.insert(std::make_pair("round", Identifier("Rounds x to the nearest integer")));
		idents.insert(std::make_pair("rsqrt", Identifier("Returns 1 / sqrt(x)")));
		idents.insert(std::make_pair("saturate", Identifier("Clamps x to the range [0, 1]")));
		idents.insert(std::make_pair("sign", Identifier("Computes the sign of x.")));
		idents.insert(std::make_pair("sin", Identifier("Returns the sine of x")));
		idents.insert(std::make_pair("sincos", Identifier("Returns the sineand cosine of x.")));
		idents.insert(std::make_pair("sinh", Identifier("Returns the hyperbolic sine of x")));
		idents.insert(std::make_pair("smoothstep", Identifier("Returns a smooth Hermite interpolation between 0 and 1.")));
		idents.insert(std::make_pair("sqrt", Identifier("Square root (per component)")));
		idents.insert(std::make_pair("step", Identifier("Returns (x >= a) ? 1 : 0")));
		idents.insert(std::make_pair("tan", Identifier("Returns the tangent of x")));
		idents.insert(std::make_pair("tanh", Identifier("Returns the hyperbolic tangent of x")));
		idents.insert(std::make_pair("tex1D", Identifier("1D texture lookup.")));
		idents.insert(std::make_pair("tex1Dbias", Identifier("1D texture lookup with bias.")));
		idents.insert(std::make_pair("tex1Dgrad", Identifier("1D texture lookup with a gradient.")));
		idents.insert(std::make_pair("tex1Dlod", Identifier("1D texture lookup with LOD.")));
		idents.insert(std::make_pair("tex1Dproj", Identifier("1D texture lookup with projective divide.")));
		idents.insert(std::make_pair("tex2D", Identifier("2D texture lookup.")));
		idents.insert(std::make_pair("tex2Dbias", Identifier("2D texture lookup with bias.")));
		idents.insert(std::make_pair("tex2Dgrad", Identifier("2D texture lookup with a gradient.")));
		idents.insert(std::make_pair("tex2Dlod", Identifier("2D texture lookup with LOD.")));
		idents.insert(std::make_pair("tex2Dproj", Identifier("2D texture lookup with projective divide.")));
		idents.insert(std::make_pair("tex3D", Identifier("3D texture lookup.")));
		idents.insert(std::make_pair("tex3Dbias", Identifier("3D texture lookup with bias.")));
		idents.insert(std::make_pair("tex3Dgrad", Identifier("3D texture lookup with a gradient.")));
		idents.insert(std::make_pair("tex3Dlod", Identifier("3D texture lookup with LOD.")));
		idents.insert(std::make_pair("tex3Dproj", Identifier("3D texture lookup with projective divide.")));
		idents.insert(std::make_pair("texCUBE", Identifier("Cube texture lookup.")));
		idents.insert(std::make_pair("texCUBEbias", Identifier("Cube texture lookup with bias.")));
		idents.insert(std::make_pair("texCUBEgrad", Identifier("Cube texture lookup with a gradient.")));
		idents.insert(std::make_pair("texCUBElod", Identifier("Cube texture lookup with LOD.")));
		idents.insert(std::make_pair("texCUBEproj", Identifier("Cube texture lookup with projective divide.")));
		idents.insert(std::make_pair("transpose", Identifier("Returns the transpose of the matrix m.")));
		idents.insert(std::make_pair("trunc", Identifier("Truncates floating-point value(s) to integer value(s)")));
	}

	const LanguageDefinition& LanguageDefinition::GLSL()
	{
		static bool inited = false;
		static LanguageDefinition langDef;
		if (!inited)
		{
			static const char* const keywords[] = {
				"auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum", "extern", "float", "for", "goto", "if", "inline", "int", "long", "register", "restrict", "return", "short",
				"signed", "sizeof", "static", "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while", "_Alignas", "_Alignof", "_Atomic", "_Bool", "_Complex", "_Generic", "_Imaginary",
				"_Noreturn", "_Static_assert", "_Thread_local", "attribute", "uniform", "varying", "layout", "centroid", "flat", "smooth", "noperspective", "patch", "sample", "subroutine", "in", "out", "inout",
				"bool", "true", "false", "invariant", "mat2", "mat3", "mat4", "dmat2", "dmat3", "dmat4", "mat2x2", "mat2x3", "mat2x4", "dmat2x2", "dmat2x3", "dmat2x4", "mat3x2", "mat3x3", "mat3x4", "dmat3x2", "dmat3x3", "dmat3x4",
				"mat4x2", "mat4x3", "mat4x4", "dmat4x2", "dmat4x3", "dmat4x4", "vec2", "vec3", "vec4", "ivec2", "ivec3", "ivec4", "bvec2", "bvec3", "bvec4", "dvec2", "dvec3", "dvec4", "uint", "uvec2", "uvec3", "uvec4",
				"lowp", "mediump", "highp", "precision", "sampler1D", "sampler2D", "sampler3D", "samplerCube", "sampler1DShadow", "sampler2DShadow", "samplerCubeShadow", "sampler1DArray", "sampler2DArray", "sampler1DArrayShadow",
				"sampler2DArrayShadow", "isampler1D", "isampler2D", "isampler3D", "isamplerCube", "isampler1DArray", "isampler2DArray", "usampler1D", "usampler2D", "usampler3D", "usamplerCube", "usampler1DArray", "usampler2DArray",
				"sampler2DRect", "sampler2DRectShadow", "isampler2DRect", "usampler2DRect", "samplerBuffer", "isamplerBuffer", "usamplerBuffer", "sampler2DMS", "isampler2DMS", "usampler2DMS", "sampler2DMSArray", "isampler2DMSArray",
				"usampler2DMSArray", "samplerCubeArray", "samplerCubeArrayShadow", "isamplerCubeArray", "usamplerCubeArray",
				"SHADERED_WEB", "SHADERED_DESKTOP", "SHADERED_VERSION", "shared", "writeonly", "readonly", "image2D", "image1D", "image3D"
			};
			for (auto& k : keywords)
				langDef.mKeywords.insert(k);

			m_GLSLDocumentation(langDef.mIdentifiers);

			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[ \\t]*#[ \\t]*[a-zA-Z_]+", PaletteIndex::Preprocessor));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("L?\\\"(\\\\.|[^\\\"])*\\\"", PaletteIndex::String));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("\\'\\\\?[^\\']\\'", PaletteIndex::CharLiteral));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[+-]?[0-9]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("0[0-7]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[a-zA-Z_][a-zA-Z0-9_]*", PaletteIndex::Identifier));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]", PaletteIndex::Punctuation));

			langDef.mCommentStart = "/*";
			langDef.mCommentEnd = "*/";
			langDef.mSingleLineComment = "//";

			langDef.mCaseSensitive = true;
			langDef.mAutoIndentation = true;

			langDef.mName = "GLSL";

			inited = true;
		}
		return langDef;
	}
	void LanguageDefinition::m_GLSLDocumentation(Identifiers& idents)
	{
		/* SOURCE: https://docs.microsoft.com/en-us/windows/desktop/direct3dhlsl/dx-graphics-hlsl-intrinsic-functions */

		idents.insert(std::make_pair("radians", Identifier("Converts x from degrees to radians.")));
		idents.insert(std::make_pair("degrees", Identifier("Converts x from radians to degrees.")));
		idents.insert(std::make_pair("sin", Identifier("Returns the sine of x")));
		idents.insert(std::make_pair("cos", Identifier("Returns the cosine of x.")));
		idents.insert(std::make_pair("tan", Identifier("Returns the tangent of x")));
		idents.insert(std::make_pair("asin", Identifier("Returns the arcsine of each component of x.")));
		idents.insert(std::make_pair("acos", Identifier("Returns the arccosine of each component of x.")));
		idents.insert(std::make_pair("atan", Identifier("Returns the arctangent of x.")));
		idents.insert(std::make_pair("sinh", Identifier("Returns the hyperbolic sine of x")));
		idents.insert(std::make_pair("cosh", Identifier("Returns the hyperbolic cosine of x.")));
		idents.insert(std::make_pair("tanh", Identifier("Returns the hyperbolic tangent of x")));
		idents.insert(std::make_pair("asinh", Identifier("Returns the arc hyperbolic sine of x")));
		idents.insert(std::make_pair("acosh", Identifier("Returns the arc hyperbolic cosine of x.")));
		idents.insert(std::make_pair("atanh", Identifier("Returns the arc hyperbolic tangent of x")));
		idents.insert(std::make_pair("pow", Identifier("Returns x^n.")));
		idents.insert(std::make_pair("exp", Identifier("Returns the base-e exponent.")));
		idents.insert(std::make_pair("exp2", Identifier("Base 2 exponent(per component).")));
		idents.insert(std::make_pair("log", Identifier("Returns the base-e logarithm of x.")));
		idents.insert(std::make_pair("log2", Identifier("Returns the base - 2 logarithm of x.")));
		idents.insert(std::make_pair("sqrt", Identifier("Square root (per component).")));
		idents.insert(std::make_pair("inversesqrt", Identifier("Returns rcp(sqrt(x)).")));
		idents.insert(std::make_pair("abs", Identifier("Absolute value (per component).")));
		idents.insert(std::make_pair("sign", Identifier("Computes the sign of x.")));
		idents.insert(std::make_pair("floor", Identifier("Returns the greatest integer which is less than or equal to x.")));
		idents.insert(std::make_pair("trunc", Identifier("Truncates floating-point value(s) to integer value(s)")));
		idents.insert(std::make_pair("round", Identifier("Rounds x to the nearest integer")));
		idents.insert(std::make_pair("roundEven", Identifier("Returns a value equal to the nearest integer to x. A fractional part of 0.5 will round toward the nearest even integer.")));
		idents.insert(std::make_pair("ceil", Identifier("Returns the smallest integer which is greater than or equal to x.")));
		idents.insert(std::make_pair("fract", Identifier("Returns the fractional part of x.")));
		idents.insert(std::make_pair("mod", Identifier("Modulus. Returns x – y ∗ floor (x/y).")));
		idents.insert(std::make_pair("modf", Identifier("Splits the value x into fractional and integer parts.")));
		idents.insert(std::make_pair("max", Identifier("Selects the greater of x and y.")));
		idents.insert(std::make_pair("min", Identifier("Selects the lesser of x and y.")));
		idents.insert(std::make_pair("clamp", Identifier("Clamps x to the range [min, max].")));
		idents.insert(std::make_pair("mix", Identifier("Returns x*(1-a)+y*a.")));
		idents.insert(std::make_pair("isinf", Identifier("Returns true if x is +INF or -INF, false otherwise.")));
		idents.insert(std::make_pair("isnan", Identifier("Returns true if x is NAN or QNAN, false otherwise.")));
		idents.insert(std::make_pair("smoothstep", Identifier("Returns a smooth Hermite interpolation between 0 and 1.")));
		idents.insert(std::make_pair("step", Identifier("Returns (x >= a) ? 1 : 0")));
		idents.insert(std::make_pair("floatBitsToInt", Identifier("Returns a signed or unsigned integer value representing the encoding of a floating-point value. The floatingpoint value's bit-level representation is preserved.")));
		idents.insert(std::make_pair("floatBitsToUint", Identifier("Returns a signed or unsigned integer value representing the encoding of a floating-point value. The floatingpoint value's bit-level representation is preserved.")));
		idents.insert(std::make_pair("intBitsToFloat", Identifier("Returns a floating-point value corresponding to a signed or unsigned integer encoding of a floating-point value.")));
		idents.insert(std::make_pair("uintBitsToFloat", Identifier("Returns a floating-point value corresponding to a signed or unsigned integer encoding of a floating-point value.")));
		idents.insert(std::make_pair("fmod", Identifier("Returns the floating point remainder of x/y.")));
		idents.insert(std::make_pair("fma", Identifier("Returns the double-precision fused multiply-addition of a * b + c.")));
		idents.insert(std::make_pair("ldexp", Identifier("Returns x * 2exp")));
		idents.insert(std::make_pair("packUnorm2x16", Identifier("First, converts each component of the normalized floating - point value v into 8 or 16bit integer values. Then, the results are packed into the returned 32bit unsigned integer.")));
		idents.insert(std::make_pair("packUnorm4x8", Identifier("First, converts each component of the normalized floating - point value v into 8 or 16bit integer values. Then, the results are packed into the returned 32bit unsigned integer.")));
		idents.insert(std::make_pair("packSnorm4x8", Identifier("First, converts each component of the normalized floating - point value v into 8 or 16bit integer values. Then, the results are packed into the returned 32bit unsigned integer.")));
		idents.insert(std::make_pair("unpackUnorm2x16", Identifier("First, unpacks a single 32bit unsigned integer p into a pair of 16bit unsigned integers, four 8bit unsigned integers, or four 8bit signed integers.Then, each component is converted to a normalized floating point value to generate the returned two or four component vector.")));
		idents.insert(std::make_pair("unpackUnorm4x8", Identifier("First, unpacks a single 32bit unsigned integer p into a pair of 16bit unsigned integers, four 8bit unsigned integers, or four 8bit signed integers.Then, each component is converted to a normalized floating point value to generate the returned two or four component vector.")));
		idents.insert(std::make_pair("unpackSnorm4x8", Identifier("First, unpacks a single 32bit unsigned integer p into a pair of 16bit unsigned integers, four 8bit unsigned integers, or four 8bit signed integers.Then, each component is converted to a normalized floating point value to generate the returned two or four component vector.")));
		idents.insert(std::make_pair("packDouble2x32", Identifier("Returns a double-precision value obtained by packing the components of v into a 64-bit value.")));
		idents.insert(std::make_pair("unpackDouble2x32", Identifier("Returns a two-component unsigned integer vector representation of v.")));
		idents.insert(std::make_pair("length", Identifier("Returns the length of the vector v.")));
		idents.insert(std::make_pair("distance", Identifier("Returns the distance between two points.")));
		idents.insert(std::make_pair("dot", Identifier("Returns the dot product of two vectors.")));
		idents.insert(std::make_pair("cross", Identifier("Returns the cross product of two 3D vectors.")));
		idents.insert(std::make_pair("normalize", Identifier("Returns a normalized vector.")));
		idents.insert(std::make_pair("faceforward", Identifier("Returns -n * sign(dot(i, ng)).")));
		idents.insert(std::make_pair("reflect", Identifier("Returns a reflection vector.")));
		idents.insert(std::make_pair("refract", Identifier("Returns the refraction vector.")));
		idents.insert(std::make_pair("matrixCompMult", Identifier("Multiply matrix x by matrix y component-wise.")));
		idents.insert(std::make_pair("outerProduct", Identifier("Linear algebraic matrix multiply c * r.")));
		idents.insert(std::make_pair("transpose", Identifier("Returns the transpose of the matrix m.")));
		idents.insert(std::make_pair("determinant", Identifier("Returns the determinant of the square matrix m.")));
		idents.insert(std::make_pair("inverse", Identifier("Returns a matrix that is the inverse of m.")));
		idents.insert(std::make_pair("lessThan", Identifier("Returns the component-wise compare of x < y")));
		idents.insert(std::make_pair("lessThanEqual", Identifier("Returns the component-wise compare of x <= y")));
		idents.insert(std::make_pair("greaterThan", Identifier("Returns the component-wise compare of x > y")));
		idents.insert(std::make_pair("greaterThanEqual", Identifier("Returns the component-wise compare of x >= y")));
		idents.insert(std::make_pair("equal", Identifier("Returns the component-wise compare of x == y")));
		idents.insert(std::make_pair("notEqual", Identifier("Returns the component-wise compare of x != y")));
		idents.insert(std::make_pair("any", Identifier("Test if any component of x is nonzero.")));
		idents.insert(std::make_pair("all", Identifier("Test if all components of x are nonzero.")));
		idents.insert(std::make_pair("not", Identifier("Returns the component-wise logical complement of x.")));
		idents.insert(std::make_pair("uaddCarry", Identifier("Adds 32bit unsigned integer x and y, returning the sum modulo 2^32.")));
		idents.insert(std::make_pair("usubBorrow", Identifier("Subtracts the 32bit unsigned integer y from x, returning the difference if non-negatice, or 2^32 plus the difference otherwise.")));
		idents.insert(std::make_pair("umulExtended", Identifier("Multiplies 32bit integers x and y, producing a 64bit result.")));
		idents.insert(std::make_pair("imulExtended", Identifier("Multiplies 32bit integers x and y, producing a 64bit result.")));
		idents.insert(std::make_pair("bitfieldExtract", Identifier("Extracts bits [offset, offset + bits - 1] from value, returning them in the least significant bits of the result.")));
		idents.insert(std::make_pair("bitfieldInsert", Identifier("Returns the insertion the bits leas-significant bits of insert into base")));
		idents.insert(std::make_pair("bitfieldReverse", Identifier("Returns the reversal of the bits of value.")));
		idents.insert(std::make_pair("bitCount", Identifier("Returns the number of bits set to 1 in the binary representation of value.")));
		idents.insert(std::make_pair("findLSB", Identifier("Returns the bit number of the least significant bit set to 1 in the binary representation of value.")));
		idents.insert(std::make_pair("findMSB", Identifier("Returns the bit number of the most significant bit in the binary representation of value.")));
		idents.insert(std::make_pair("textureSize", Identifier("Returns the dimensions of level lod  (if present) for the texture bound to sample.")));
		idents.insert(std::make_pair("textureQueryLod", Identifier("Returns the mipmap array(s) that would be accessed in the x component of the return value.")));
		idents.insert(std::make_pair("texture", Identifier("Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.")));
		idents.insert(std::make_pair("textureProj", Identifier("Do a texture lookup with projection.")));
		idents.insert(std::make_pair("textureLod", Identifier("Do a texture lookup as in texture but with explicit LOD.")));
		idents.insert(std::make_pair("textureOffset", Identifier("Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel.")));
		idents.insert(std::make_pair("texelFetch", Identifier("Use integer texture coordinate P to lookup a single texel from sampler.")));
		idents.insert(std::make_pair("texelFetchOffset", Identifier("Fetch a single texel as in texelFetch offset by offset.")));
		idents.insert(std::make_pair("texetureProjOffset", Identifier("Do a projective texture lookup as described in textureProj offset by offset as descrived in textureOffset.")));
		idents.insert(std::make_pair("texetureLodOffset", Identifier("Do an offset texture lookup with explicit LOD.")));
		idents.insert(std::make_pair("textureProjLod", Identifier("Do a projective texture lookup with explicit LOD.")));
		idents.insert(std::make_pair("textureLodOffset", Identifier("Do an offset texture lookup with explicit LOD.")));
		idents.insert(std::make_pair("textureProjLodOffset", Identifier("Do an offset projective texture lookup with explicit LOD.")));
		idents.insert(std::make_pair("textureGrad", Identifier("Do a texture lookup as in texture but with explicit gradients.")));
		idents.insert(std::make_pair("textureGradOffset", Identifier("Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset.")));
		idents.insert(std::make_pair("textureProjGrad", Identifier("Do a texture lookup both projectively and with explicit gradient.")));
		idents.insert(std::make_pair("textureProjGradOffset", Identifier("Do a texture lookup both projectively and with explicit gradient as well as with offset.")));
		idents.insert(std::make_pair("textureGather", Identifier("Built-in function.")));
		idents.insert(std::make_pair("textureGatherOffset", Identifier("Built-in function.")));
		idents.insert(std::make_pair("textureGatherOffsets", Identifier("Built-in function.")));
		idents.insert(std::make_pair("texture1D", Identifier("1D texture lookup.")));
		idents.insert(std::make_pair("texture1DLod", Identifier("1D texture lookup with LOD.")));
		idents.insert(std::make_pair("texture1DProj", Identifier("1D texture lookup with projective divide.")));
		idents.insert(std::make_pair("texture1DProjLod", Identifier("1D texture lookup with projective divide and with LOD.")));
		idents.insert(std::make_pair("texture2D", Identifier("2D texture lookup.")));
		idents.insert(std::make_pair("texture2DLod", Identifier("2D texture lookup with LOD.")));
		idents.insert(std::make_pair("texture2DProj", Identifier("2D texture lookup with projective divide.")));
		idents.insert(std::make_pair("texture2DProjLod", Identifier("2D texture lookup with projective divide and with LOD.")));
		idents.insert(std::make_pair("texture3D", Identifier("3D texture lookup.")));
		idents.insert(std::make_pair("texture3DLod", Identifier("3D texture lookup with LOD.")));
		idents.insert(std::make_pair("texture3DProj", Identifier("3D texture lookup with projective divide.")));
		idents.insert(std::make_pair("texture3DProjLod", Identifier("3D texture lookup with projective divide and with LOD.")));
		idents.insert(std::make_pair("textureCube", Identifier("Cube texture lookup.")));
		idents.insert(std::make_pair("textureCubeLod", Identifier("Cube texture lookup with LOD.")));
		idents.insert(std::make_pair("shadow1D", Identifier("1D texture lookup.")));
		idents.insert(std::make_pair("shadow1DLod", Identifier("1D texture lookup with LOD.")));
		idents.insert(std::make_pair("shadow1DProj", Identifier("1D texture lookup with projective divide.")));
		idents.insert(std::make_pair("shadow1DProjLod", Identifier("1D texture lookup with projective divide and with LOD.")));
		idents.insert(std::make_pair("shadow2D", Identifier("2D texture lookup.")));
		idents.insert(std::make_pair("shadow2DLod", Identifier("2D texture lookup with LOD.")));
		idents.insert(std::make_pair("shadow2DProj", Identifier("2D texture lookup with projective divide.")));
		idents.insert(std::make_pair("shadow2DProjLod", Identifier("2D texture lookup with projective divide and with LOD.")));
		idents.insert(std::make_pair("dFdx", Identifier("Returns the partial derivative of x with respect to the screen-space x-coordinate.")));
		idents.insert(std::make_pair("dFdy", Identifier("Returns the partial derivative of x with respect to the screen-space y-coordinate.")));
		idents.insert(std::make_pair("fwidth", Identifier("Returns abs(ddx(x)) + abs(ddy(x))")));
		idents.insert(std::make_pair("interpolateAtCentroid", Identifier("Return the value of the input varying interpolant sampled at a location inside the both the pixel and the primitive being processed.")));
		idents.insert(std::make_pair("interpolateAtSample", Identifier("Return the value of the input varying interpolant at the location of sample number sample.")));
		idents.insert(std::make_pair("interpolateAtOffset", Identifier("Return the value of the input varying interpolant sampled at an offset from the center of the pixel specified by offset.")));
		idents.insert(std::make_pair("noise1", Identifier("Generates a random value")));
		idents.insert(std::make_pair("noise2", Identifier("Generates a random value")));
		idents.insert(std::make_pair("noise3", Identifier("Generates a random value")));
		idents.insert(std::make_pair("noise4", Identifier("Generates a random value")));
		idents.insert(std::make_pair("EmitStreamVertex", Identifier("Emit the current values of output variables to the current output primitive on stream stream.")));
		idents.insert(std::make_pair("EndStreamPrimitive", Identifier("Completes the current output primitive on stream stream and starts a new one.")));
		idents.insert(std::make_pair("EmitVertex", Identifier("Emit the current values to the current output primitive.")));
		idents.insert(std::make_pair("EndPrimitive", Identifier("Completes the current output primitive and starts a new one.")));
		idents.insert(std::make_pair("barrier", Identifier("Synchronize execution of multiple shader invocations")));
		idents.insert(std::make_pair("groupMemoryBarrier", Identifier("Controls the ordering of memory transaction issued shader invocation relative to a work group")));
		idents.insert(std::make_pair("memoryBarrier", Identifier("Controls the ordering of memory transactions issued by a single shader invocation")));
		idents.insert(std::make_pair("memoryBarrierAtomicCounter", Identifier("Controls the ordering of operations on atomic counters issued by a single shader invocation")));
		idents.insert(std::make_pair("memoryBarrierBuffer", Identifier("Controls the ordering of operations on buffer variables issued by a single shader invocation")));
		idents.insert(std::make_pair("memoryBarrierImage", Identifier("Controls the ordering of operations on image variables issued by a single shader invocation")));
		idents.insert(std::make_pair("memoryBarrierShared", Identifier("Controls the ordering of operations on shared variables issued by a single shader invocation")));

		idents.insert(std::make_pair("atomicAdd", Identifier("Perform an atomic addition to a variable")));
		idents.insert(std::make_pair("atomicAnd", Identifier("Perform an atomic logical AND operation to a variable")));
		idents.insert(std::make_pair("atomicCompSwap", Identifier("Perform an atomic compare-exchange operation to a variable")));
		idents.insert(std::make_pair("atomicCounter", Identifier("Return the current value of an atomic counter")));
		idents.insert(std::make_pair("atomicCounterDecrement", Identifier("Atomically decrement a counter and return its new value")));
		idents.insert(std::make_pair("atomicCounterIncrement", Identifier("Atomically increment a counter and return the prior value")));
		idents.insert(std::make_pair("atomicExchange", Identifier("Perform an atomic exchange operation to a variable")));
		idents.insert(std::make_pair("atomicMax", Identifier("Perform an atomic max operation to a variable")));
		idents.insert(std::make_pair("atomicMin", Identifier("Perform an atomic min operation to a variable")));
		idents.insert(std::make_pair("atomicOr", Identifier("Perform an atomic logical OR operation to a variable")));
		idents.insert(std::make_pair("atomicXor", Identifier("Perform an atomic logical exclusive OR operation to a variable")));
	}

	const LanguageDefinition& LanguageDefinition::SPIRV()
	{
		static bool inited = false;
		static LanguageDefinition langDef;
		if (!inited) {
			/*
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[ \\t]*#[ \\t]*[a-zA-Z_]+", PaletteIndex::Preprocessor));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("\\'\\\\?[^\\']\\'", PaletteIndex::CharLiteral));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[a-zA-Z_][a-zA-Z0-9_]*", PaletteIndex::Identifier));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]", PaletteIndex::Punctuation));
			*/

			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("L?\\\"(\\\\.|[^\\\"])*\\\"", PaletteIndex::String));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[ =\\t]Op[a-zA-Z]*", PaletteIndex::Keyword));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("%[_a-zA-Z0-9]*", PaletteIndex::Identifier));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[+-]?[0-9]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("0[0-7]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?", PaletteIndex::Number));

			langDef.mCommentStart = "/*";
			langDef.mCommentEnd = "*/";
			langDef.mSingleLineComment = ";";

			langDef.mCaseSensitive = true;
			langDef.mAutoIndentation = false;

			langDef.mName = "SPIR-V";

			inited = true;
		}
		return langDef;
	}

	const LanguageDefinition& LanguageDefinition::C()
	{
		static bool inited = false;
		static LanguageDefinition langDef;
		if (!inited)
		{
			static const char* const keywords[] = {
				"auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum", "extern", "float", "for", "goto", "if", "inline", "int", "long", "register", "restrict", "return", "short",
				"signed", "sizeof", "static", "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while", "_Alignas", "_Alignof", "_Atomic", "_Bool", "_Complex", "_Generic", "_Imaginary",
				"_Noreturn", "_Static_assert", "_Thread_local"
			};
			for (auto& k : keywords)
				langDef.mKeywords.insert(k);

			static const char* const identifiers[] = {
				"abort", "abs", "acos", "asin", "atan", "atexit", "atof", "atoi", "atol", "ceil", "clock", "cosh", "ctime", "div", "exit", "fabs", "floor", "fmod", "getchar", "getenv", "isalnum", "isalpha", "isdigit", "isgraph",
				"ispunct", "isspace", "isupper", "kbhit", "log10", "log2", "log", "memcmp", "modf", "pow", "putchar", "putenv", "puts", "rand", "remove", "rename", "sinh", "sqrt", "srand", "strcat", "strcmp", "strerror", "time", "tolower", "toupper"
			};
			for (auto& k : identifiers)
			{
				Identifier id;
				id.mDeclaration = "Built-in function";
				langDef.mIdentifiers.insert(std::make_pair(std::string(k), id));
			}

			langDef.mTokenize = [](const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end, PaletteIndex& paletteIndex) -> bool
				{
					paletteIndex = PaletteIndex::Max;

					while (in_begin < in_end && isascii(*in_begin) && isblank(*in_begin))
						in_begin++;

					if (in_begin == in_end)
					{
						out_begin = in_end;
						out_end = in_end;
						paletteIndex = PaletteIndex::Default;
					}
					else if (TokenizeCStyleString(in_begin, in_end, out_begin, out_end))
						paletteIndex = PaletteIndex::String;
					else if (TokenizeCStyleCharacterLiteral(in_begin, in_end, out_begin, out_end))
						paletteIndex = PaletteIndex::CharLiteral;
					else if (TokenizeCStyleIdentifier(in_begin, in_end, out_begin, out_end))
						paletteIndex = PaletteIndex::Identifier;
					else if (TokenizeCStyleNumber(in_begin, in_end, out_begin, out_end))
						paletteIndex = PaletteIndex::Number;
					else if (TokenizeCStylePunctuation(in_begin, in_end, out_begin, out_end))
						paletteIndex = PaletteIndex::Punctuation;

					return paletteIndex != PaletteIndex::Max;
				};

			langDef.mCommentStart = "/*";
			langDef.mCommentEnd = "*/";
			langDef.mSingleLineComment = "//";

			langDef.mCaseSensitive = true;
			langDef.mAutoIndentation = true;

			langDef.mName = "C";

			inited = true;
		}
		return langDef;
	}

	const LanguageDefinition& LanguageDefinition::SQL()
	{
		static bool inited = false;
		static LanguageDefinition langDef;
		if (!inited)
		{
			static const char* const keywords[] = {
				"ADD", "EXCEPT", "PERCENT", "ALL", "EXEC", "PLAN", "ALTER", "EXECUTE", "PRECISION", "AND", "EXISTS", "PRIMARY", "ANY", "EXIT", "PRINT", "AS", "FETCH", "PROC", "ASC", "FILE", "PROCEDURE",
				"AUTHORIZATION", "FILLFACTOR", "PUBLIC", "BACKUP", "FOR", "RAISERROR", "BEGIN", "FOREIGN", "READ", "BETWEEN", "FREETEXT", "READTEXT", "BREAK", "FREETEXTTABLE", "RECONFIGURE",
				"BROWSE", "FROM", "REFERENCES", "BULK", "FULL", "REPLICATION", "BY", "FUNCTION", "RESTORE", "CASCADE", "GOTO", "RESTRICT", "CASE", "GRANT", "RETURN", "CHECK", "GROUP", "REVOKE",
				"CHECKPOINT", "HAVING", "RIGHT", "CLOSE", "HOLDLOCK", "ROLLBACK", "CLUSTERED", "IDENTITY", "ROWCOUNT", "COALESCE", "IDENTITY_INSERT", "ROWGUIDCOL", "COLLATE", "IDENTITYCOL", "RULE",
				"COLUMN", "IF", "SAVE", "COMMIT", "IN", "SCHEMA", "COMPUTE", "INDEX", "SELECT", "CONSTRAINT", "INNER", "SESSION_USER", "CONTAINS", "INSERT", "SET", "CONTAINSTABLE", "INTERSECT", "SETUSER",
				"CONTINUE", "INTO", "SHUTDOWN", "CONVERT", "IS", "SOME", "CREATE", "JOIN", "STATISTICS", "CROSS", "KEY", "SYSTEM_USER", "CURRENT", "KILL", "TABLE", "CURRENT_DATE", "LEFT", "TEXTSIZE",
				"CURRENT_TIME", "LIKE", "THEN", "CURRENT_TIMESTAMP", "LINENO", "TO", "CURRENT_USER", "LOAD", "TOP", "CURSOR", "NATIONAL", "TRAN", "DATABASE", "NOCHECK", "TRANSACTION",
				"DBCC", "NONCLUSTERED", "TRIGGER", "DEALLOCATE", "NOT", "TRUNCATE", "DECLARE", "NULL", "TSEQUAL", "DEFAULT", "NULLIF", "UNION", "DELETE", "OF", "UNIQUE", "DENY", "OFF", "UPDATE",
				"DESC", "OFFSETS", "UPDATETEXT", "DISK", "ON", "USE", "DISTINCT", "OPEN", "USER", "DISTRIBUTED", "OPENDATASOURCE", "VALUES", "DOUBLE", "OPENQUERY", "VARYING","DROP", "OPENROWSET", "VIEW",
				"DUMMY", "OPENXML", "WAITFOR", "DUMP", "OPTION", "WHEN", "ELSE", "OR", "WHERE", "END", "ORDER", "WHILE", "ERRLVL", "OUTER", "WITH", "ESCAPE", "OVER", "WRITETEXT"
			};

			for (auto& k : keywords)
				langDef.mKeywords.insert(k);

			static const char* const identifiers[] = {
				"ABS",  "ACOS",  "ADD_MONTHS",  "ASCII",  "ASCIISTR",  "ASIN",  "ATAN",  "ATAN2",  "AVG",  "BFILENAME",  "BIN_TO_NUM",  "BITAND",  "CARDINALITY",  "CASE",  "CAST",  "CEIL",
				"CHARTOROWID",  "CHR",  "COALESCE",  "COMPOSE",  "CONCAT",  "CONVERT",  "CORR",  "COS",  "COSH",  "COUNT",  "COVAR_POP",  "COVAR_SAMP",  "CUME_DIST",  "CURRENT_DATE",
				"CURRENT_TIMESTAMP",  "DBTIMEZONE",  "DECODE",  "DECOMPOSE",  "DENSE_RANK",  "DUMP",  "EMPTY_BLOB",  "EMPTY_CLOB",  "EXP",  "EXTRACT",  "FIRST_VALUE",  "FLOOR",  "FROM_TZ",  "GREATEST",
				"GROUP_ID",  "HEXTORAW",  "INITCAP",  "INSTR",  "INSTR2",  "INSTR4",  "INSTRB",  "INSTRC",  "LAG",  "LAST_DAY",  "LAST_VALUE",  "LEAD",  "LEAST",  "LENGTH",  "LENGTH2",  "LENGTH4",
				"LENGTHB",  "LENGTHC",  "LISTAGG",  "LN",  "LNNVL",  "LOCALTIMESTAMP",  "LOG",  "LOWER",  "LPAD",  "LTRIM",  "MAX",  "MEDIAN",  "MIN",  "MOD",  "MONTHS_BETWEEN",  "NANVL",  "NCHR",
				"NEW_TIME",  "NEXT_DAY",  "NTH_VALUE",  "NULLIF",  "NUMTODSINTERVAL",  "NUMTOYMINTERVAL",  "NVL",  "NVL2",  "POWER",  "RANK",  "RAWTOHEX",  "REGEXP_COUNT",  "REGEXP_INSTR",
				"REGEXP_REPLACE",  "REGEXP_SUBSTR",  "REMAINDER",  "REPLACE",  "ROUND",  "ROWNUM",  "RPAD",  "RTRIM",  "SESSIONTIMEZONE",  "SIGN",  "SIN",  "SINH",
				"SOUNDEX",  "SQRT",  "STDDEV",  "SUBSTR",  "SUM",  "SYS_CONTEXT",  "SYSDATE",  "SYSTIMESTAMP",  "TAN",  "TANH",  "TO_CHAR",  "TO_CLOB",  "TO_DATE",  "TO_DSINTERVAL",  "TO_LOB",
				"TO_MULTI_BYTE",  "TO_NCLOB",  "TO_NUMBER",  "TO_SINGLE_BYTE",  "TO_TIMESTAMP",  "TO_TIMESTAMP_TZ",  "TO_YMINTERVAL",  "TRANSLATE",  "TRIM",  "TRUNC", "TZ_OFFSET",  "UID",  "UPPER",
				"USER",  "USERENV",  "VAR_POP",  "VAR_SAMP",  "VARIANCE",  "VSIZE "
			};
			for (auto& k : identifiers)
			{
				Identifier id;
				id.mDeclaration = "Built-in function";
				langDef.mIdentifiers.insert(std::make_pair(std::string(k), id));
			}

			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("L?\\\"(\\\\.|[^\\\"])*\\\"", PaletteIndex::String));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("\\\'[^\\\']*\\\'", PaletteIndex::String));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[+-]?[0-9]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("0[0-7]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[a-zA-Z_][a-zA-Z0-9_]*", PaletteIndex::Identifier));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]", PaletteIndex::Punctuation));

			langDef.mCommentStart = "/*";
			langDef.mCommentEnd = "*/";
			langDef.mSingleLineComment = "//";

			langDef.mCaseSensitive = false;
			langDef.mAutoIndentation = false;

			langDef.mName = "SQL";

			inited = true;
		}
		return langDef;
	}

	const LanguageDefinition& LanguageDefinition::AngelScript()
	{
		static bool inited = false;
		static LanguageDefinition langDef;
		if (!inited)
		{
			static const char* const keywords[] = {
				"and", "abstract", "auto", "bool", "break", "case", "cast", "class", "const", "continue", "default", "do", "double", "else", "enum", "false", "final", "float", "for",
				"from", "funcdef", "function", "get", "if", "import", "in", "inout", "int", "interface", "int8", "int16", "int32", "int64", "is", "mixin", "namespace", "not",
				"null", "or", "out", "override", "private", "protected", "return", "set", "shared", "super", "switch", "this ", "true", "typedef", "uint", "uint8", "uint16", "uint32",
				"uint64", "void", "while", "xor"
			};

			for (auto& k : keywords)
				langDef.mKeywords.insert(k);

			static const char* const identifiers[] = {
				"cos", "sin", "tab", "acos", "asin", "atan", "atan2", "cosh", "sinh", "tanh", "log", "log10", "pow", "sqrt", "abs", "ceil", "floor", "fraction", "closeTo", "fpFromIEEE", "fpToIEEE",
				"complex", "opEquals", "opAddAssign", "opSubAssign", "opMulAssign", "opDivAssign", "opAdd", "opSub", "opMul", "opDiv"
			};
			for (auto& k : identifiers)
			{
				Identifier id;
				id.mDeclaration = "Built-in function";
				langDef.mIdentifiers.insert(std::make_pair(std::string(k), id));
			}

			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("L?\\\"(\\\\.|[^\\\"])*\\\"", PaletteIndex::String));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("\\'\\\\?[^\\']\\'", PaletteIndex::String));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[+-]?[0-9]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("0[0-7]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[a-zA-Z_][a-zA-Z0-9_]*", PaletteIndex::Identifier));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]", PaletteIndex::Punctuation));

			langDef.mCommentStart = "/*";
			langDef.mCommentEnd = "*/";
			langDef.mSingleLineComment = "//";

			langDef.mCaseSensitive = true;
			langDef.mAutoIndentation = true;

			langDef.mName = "AngelScript";

			inited = true;
		}
		return langDef;
	}

	const LanguageDefinition& LanguageDefinition::Lua()
	{
		static bool inited = false;
		static LanguageDefinition langDef;
		if (!inited)
		{
			static const char* const keywords[] = {
				"and", "break", "do", "", "else", "elseif", "end", "false", "for", "function", "if", "in", "", "local", "nil", "not", "or", "repeat", "return", "then", "true", "until", "while"
			};

			for (auto& k : keywords)
				langDef.mKeywords.insert(k);

			static const char* const identifiers[] = {
				"assert", "collectgarbage", "dofile", "error", "getmetatable", "ipairs", "loadfile", "load", "loadstring",  "next",  "pairs",  "pcall",  "print",  "rawequal",  "rawlen",  "rawget",  "rawset",
				"select",  "setmetatable",  "tonumber",  "tostring",  "type",  "xpcall",  "_G",  "_VERSION","arshift", "band", "bnot", "bor", "bxor", "btest", "extract", "lrotate", "lshift", "replace",
				"rrotate", "rshift", "create", "resume", "running", "status", "wrap", "yield", "isyieldable", "debug","getuservalue", "gethook", "getinfo", "getlocal", "getregistry", "getmetatable",
				"getupvalue", "upvaluejoin", "upvalueid", "setuservalue", "sethook", "setlocal", "setmetatable", "setupvalue", "traceback", "close", "flush", "input", "lines", "open", "output", "popen",
				"read", "tmpfile", "type", "write", "close", "flush", "lines", "read", "seek", "setvbuf", "write", "__gc", "__tostring", "abs", "acos", "asin", "atan", "ceil", "cos", "deg", "exp", "tointeger",
				"floor", "fmod", "ult", "log", "max", "min", "modf", "rad", "random", "randomseed", "sin", "sqrt", "string", "tan", "type", "atan2", "cosh", "sinh", "tanh",
				 "pow", "frexp", "ldexp", "log10", "pi", "huge", "maxinteger", "mininteger", "loadlib", "searchpath", "seeall", "preload", "cpath", "path", "searchers", "loaded", "module", "require", "clock",
				 "date", "difftime", "execute", "exit", "getenv", "remove", "rename", "setlocale", "time", "tmpname", "byte", "char", "dump", "find", "format", "gmatch", "gsub", "len", "lower", "match", "rep",
				 "reverse", "sub", "upper", "pack", "packsize", "unpack", "concat", "maxn", "insert", "pack", "unpack", "remove", "move", "sort", "offset", "codepoint", "char", "len", "codes", "charpattern",
				 "coroutine", "table", "io", "os", "string", "utf8", "bit32", "math", "debug", "package"
			};
			for (auto& k : identifiers)
			{
				Identifier id;
				id.mDeclaration = "Built-in function";
				langDef.mIdentifiers.insert(std::make_pair(std::string(k), id));
			}

			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("L?\\\"(\\\\.|[^\\\"])*\\\"", PaletteIndex::String));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("\\\'[^\\\']*\\\'", PaletteIndex::String));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[+-]?[0-9]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[a-zA-Z_][a-zA-Z0-9_]*", PaletteIndex::Identifier));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]", PaletteIndex::Punctuation));

			langDef.mCommentStart = "--[[";
			langDef.mCommentEnd = "]]";
			langDef.mSingleLineComment = "--";

			langDef.mCaseSensitive = true;
			langDef.mAutoIndentation = false;

			langDef.mName = "Lua";

			inited = true;
		}
		return langDef;
	}

	const LanguageDefinition& LanguageDefinition::GalGameScript()
	{
		static bool inited = false;
		static LanguageDefinition langDef;
		if (!inited)
		{
			static const char* const keywords[] = {
				"and", "break", "do", "", "else", "elseif", "end", "false", "for", "function", "if", "in", "", "local", "nil", "not", "or", "repeat", "return", "then", "true", "until", "while"
			};

			for (auto& k : keywords)
				langDef.mKeywords.insert(k);

			static const char* const identifiers[] = {
				"assert", "collectgarbage", "dofile", "error", "getmetatable", "ipairs", "loadfile", "load", "loadstring",  "next",  "pairs",  "pcall",  "print",  "rawequal",  "rawlen",  "rawget",  "rawset",
				"select",  "setmetatable",  "tonumber",  "tostring",  "type",  "xpcall",  "_G",  "_VERSION","arshift", "band", "bnot", "bor", "bxor", "btest", "extract", "lrotate", "lshift", "replace",
				"rrotate", "rshift", "create", "resume", "running", "status", "wrap", "yield", "isyieldable", "debug","getuservalue", "gethook", "getinfo", "getlocal", "getregistry", "getmetatable",
				"getupvalue", "upvaluejoin", "upvalueid", "setuservalue", "sethook", "setlocal", "setmetatable", "setupvalue", "traceback", "close", "flush", "input", "lines", "open", "output", "popen",
				"read", "tmpfile", "type", "write", "close", "flush", "lines", "read", "seek", "setvbuf", "write", "__gc", "__tostring", "abs", "acos", "asin", "atan", "ceil", "cos", "deg", "exp", "tointeger",
				"floor", "fmod", "ult", "log", "max", "min", "modf", "rad", "random", "randomseed", "sin", "sqrt", "string", "tan", "type", "atan2", "cosh", "sinh", "tanh",
				 "pow", "frexp", "ldexp", "log10", "pi", "huge", "maxinteger", "mininteger", "loadlib", "searchpath", "seeall", "preload", "cpath", "path", "searchers", "loaded", "module", "require", "clock",
				 "date", "difftime", "execute", "exit", "getenv", "remove", "rename", "setlocale", "time", "tmpname", "byte", "char", "dump", "find", "format", "gmatch", "gsub", "len", "lower", "match", "rep",
				 "reverse", "sub", "upper", "pack", "packsize", "unpack", "concat", "maxn", "insert", "pack", "unpack", "remove", "move", "sort", "offset", "codepoint", "char", "len", "codes", "charpattern",
				 "coroutine", "table", "io", "os", "string", "utf8", "bit32", "math", "debug", "package"//,

				 //"创建人物","引擎","显示背景","播放效果音乐","播放音乐"
			};
			for (auto& k : identifiers)
			{
				Identifier id;
				id.mDeclaration = "Built-in function";
				langDef.mIdentifiers.insert(std::make_pair(std::string(k), id));
			}

			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("L?\\\"(\\\\.|[^\\\"])*\\\"", PaletteIndex::String));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("\\\'[^\\\']*\\\'", PaletteIndex::String));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[+-]?[0-9]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[a-zA-Z_][a-zA-Z0-9_]*", PaletteIndex::Identifier));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]", PaletteIndex::Punctuation));

			langDef.mCommentStart = "--[[";
			langDef.mCommentEnd = "]]";
			langDef.mSingleLineComment = "--";

			langDef.mCaseSensitive = true;
			langDef.mAutoIndentation = false;

			langDef.mName = "Lua";

			inited = true;

			langDef.mTokenize = [](const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end, PaletteIndex& paletteIndex)
				{
					//const char* keyword = u8"创建人物";
					//
					//out_begin = std::search(in_begin, in_end, keyword, keyword + std::strlen(keyword));
					//if (out_begin != in_end) 
					//{
					//	out_end = out_begin + std::strlen(keyword);
					//	paletteIndex = PaletteIndex::KnownIdentifier;
					//	return true;
					//}

					auto tokenize = [](const char* keyword, const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end, PaletteIndex& paletteIndex)
						{
							out_begin = std::search(in_begin, in_end, keyword, keyword + std::strlen(keyword));
							if (out_begin != in_end)
							{
								out_end = out_begin + std::strlen(keyword);
								paletteIndex = PaletteIndex::KnownIdentifier;
								return true;
							}

							return false;
						};

					auto userfunction = [](const char* keyword, const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end, PaletteIndex& paletteIndex)
						{
							out_begin = std::search(in_begin, in_end, keyword, keyword + std::strlen(keyword));
							if (out_begin != in_end)
							{
								out_end = out_begin + std::strlen(keyword);
								paletteIndex = PaletteIndex::UserFunction;
								return true;
							}

							out_begin++;
							out_end--;

							return false;
						};

					if (tokenize(u8"创建人物", in_begin, in_end, out_begin, out_end, paletteIndex))
						return true;
					if (tokenize(u8"引擎", in_begin, in_end, out_begin, out_end, paletteIndex))
						return true;
					if (tokenize(u8"显示背景", in_begin, in_end, out_begin, out_end, paletteIndex))
						return true;
					if (tokenize(u8"播放效果音乐", in_begin, in_end, out_begin, out_end, paletteIndex))
						return true;
					if (tokenize(u8"播放音乐", in_begin, in_end, out_begin, out_end, paletteIndex))
						return true;

					if (userfunction(u8":说着(", in_begin, in_end, out_begin, out_end, paletteIndex))
						return true;

					if (userfunction(u8":添加立绘(", in_begin, in_end, out_begin, out_end, paletteIndex))
						return true;

					if (userfunction(u8":显示立绘(", in_begin, in_end, out_begin, out_end, paletteIndex))
						return true;

					if (userfunction(u8":添加立绘显示回调(", in_begin, in_end, out_begin, out_end, paletteIndex))
						return true;

					if (userfunction(u8":添加立绘隐藏回调(", in_begin, in_end, out_begin, out_end, paletteIndex))
						return true;

					return false;
				};
		}
		return langDef;
	}

    const LanguageDefinition& LanguageDefinition::RmlUI()
    {
		static bool inited = false;
		static LanguageDefinition langDef;
		if (!inited)
		{
			static const char* const keywords[] = {
				// HTML
				"html","head","title","base","link","meta","style","script","noscript","body","section","nav","article","aside","h1","h2","h3","h4","h5","h6",
				"header","footer","address","main","p","hr","pre","blockquote","ol","ul","li","dl","dt","dd","figure","figcaption","div","span","a","em","strong",
				"small","s","cite","q","dfn","abbr","data","time","code","var","samp","kbd","sub","sup","i","b","u","mark","ruby","rt","rp","bdi","bdo","br","wbr",
				"ins","del","img","iframe","embed","object","param","video","audio","source","track","canvas","map","area","svg","math","table","caption","colgroup",
				"col","tbody","thead","tfoot","tr","td","th","form","fieldset","legend","label","input","button","select","datalist","optgroup","option","textarea",
				"keygen","output","progress","meter","details","summary","menuitem","menu",

				// Lua
				"and", "break", "do", "", "else", "elseif", "end", "false", "for", "function", "if", "in", "", "local", "nil", "not", "or", "repeat", "return", "then", "true", "until", "while"
			};

			for (auto& k : keywords)
				langDef.mKeywords.insert(k);

			static const char* const identifiers[] = {
				// Lua
				"assert", "collectgarbage", "dofile", "error", "getmetatable", "ipairs", "loadfile", "load", "loadstring",  "next",  "pairs",  "pcall",  "print",  "rawequal",  "rawlen",  "rawget",  "rawset",
				"select",  "setmetatable",  "tonumber",  "tostring",  "type",  "xpcall",  "_G",  "_VERSION","arshift", "band", "bnot", "bor", "bxor", "btest", "extract", "lrotate", "lshift", "replace",
				"rrotate", "rshift", "create", "resume", "running", "status", "wrap", "yield", "isyieldable", "debug","getuservalue", "gethook", "getinfo", "getlocal", "getregistry", "getmetatable",
				"getupvalue", "upvaluejoin", "upvalueid", "setuservalue", "sethook", "setlocal", "setmetatable", "setupvalue", "traceback", "close", "flush", "input", "lines", "open", "output", "popen",
				"read", "tmpfile", "type", "write", "close", "flush", "lines", "read", "seek", "setvbuf", "write", "__gc", "__tostring", "abs", "acos", "asin", "atan", "ceil", "cos", "deg", "exp", "tointeger",
				"floor", "fmod", "ult", "log", "max", "min", "modf", "rad", "random", "randomseed", "sin", "sqrt", "string", "tan", "type", "atan2", "cosh", "sinh", "tanh",
				 "pow", "frexp", "ldexp", "log10", "pi", "huge", "maxinteger", "mininteger", "loadlib", "searchpath", "seeall", "preload", "cpath", "path", "searchers", "loaded", "module", "require", "clock",
				 "date", "difftime", "execute", "exit", "getenv", "remove", "rename", "setlocale", "time", "tmpname", "byte", "char", "dump", "find", "format", "gmatch", "gsub", "len", "lower", "match", "rep",
				 "reverse", "sub", "upper", "pack", "packsize", "unpack", "concat", "maxn", "insert", "pack", "unpack", "remove", "move", "sort", "offset", "codepoint", "char", "len", "codes", "charpattern",
				 "coroutine", "table", "io", "os", "string", "utf8", "bit32", "math", "debug", "package",

				 // HTML
				"id","class","style","title","name","value","type","src","href","alt","width","height","rel","media","charset","content","http-equiv","lang","dir",
				"role","aria-label","aria-hidden","placeholder","disabled","checked","readonly","required","multiple","selected","size","maxlength","min","max","step",
				"onclick","onload","onchange","data-*","contenteditable"
			};
			for (auto& k : identifiers)
			{
				Identifier id;
				id.mDeclaration = "Built-in function";
				langDef.mIdentifiers.insert(std::make_pair(std::string(k), id));
			}

			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("L?\\\"(\\\\.|[^\\\"])*\\\"", PaletteIndex::String));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("\\\'[^\\\']*\\\'", PaletteIndex::String));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[+-]?[0-9]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[a-zA-Z_][a-zA-Z0-9_]*", PaletteIndex::Identifier));
			langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]", PaletteIndex::Punctuation));

			langDef.mCommentStart = "<!--";
			langDef.mCommentEnd = "-->";
			langDef.mSingleLineComment = "--";

			langDef.mCaseSensitive = true;
			langDef.mAutoIndentation = false;

			langDef.mName = "RmlUI";

			inited = true;
		}
		return langDef;
    }
}
