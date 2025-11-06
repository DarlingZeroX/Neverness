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
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>
#include <algorithm>
#include <functional>
#include <chrono>
#include <string>
#include <regex>
#include <cmath>

#include "ImGuiColorTextEdit/TextEditor.h"

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui.h> // for imGui::GetCurrentWindow()
#include <imgui/imgui_internal.h>
  
namespace ImGuiTextEditor {
	// TODO
	// - multiline comments vs single-line: latter is blocking start of a ML

	template<class InputIt1, class InputIt2, class BinaryPredicate>
	bool equals(InputIt1 first1, InputIt1 last1,
		InputIt2 first2, InputIt2 last2, BinaryPredicate p)
	{
		for (; first1 != last1 && first2 != last2; ++first1, ++first2)
		{
			if (!p(*first1, *first2))
				return false;
		}
		return first1 == last1 && first2 == last2;
	}

	int isBracket(char ch) {
		if (ch == '(' || ch == '[' || ch == '{')
			return 1;
		if (ch == ')' || ch == ']' || ch == '}')
			return 2;
		return 0;
	}
	bool isClosingBracket(char open, char actual)
	{
		return (open == '{' && actual == '}') || (open == '[' && actual == ']') || (open == '(' && actual == ')');
	}
	bool isOpeningBracket(char close, char actual)
	{
		return (close == '}' && actual == '{') || (close == ']' && actual == '[') || (close == ')' && actual == '(');
	}

	TextEditor::TextEditor()
		: mLineSpacing(1.0f)
		, mUndoIndex(0)
		, mInsertSpaces(false)
		, mTabSize(4)
		, mHighlightBrackets(false)
		, mAutocomplete(true)
		, mACOpened(false)
		, mHighlightLine(true)
		, mHorizontalScroll(true)
		, mCompleteBraces(true)
		, mShowLineNumbers(true)
		, mSmartIndent(true)
		, mOverwrite(false)
		, mReadOnly(false)
		, mWithinRender(false)
		, mScrollToCursor(false)
		, mScrollToTop(false)
		, mTextChanged(false)
		, mColorizerEnabled(true)
		, mTextStart(20.0f)
		, mLeftMargin(DebugDataSpace + LineNumberSpace)
		, mCursorPositionChanged(false)
		, mColorRangeMin(0)
		, mColorRangeMax(0)
		, mSelectionMode(SelectionMode::Normal)
		, mCheckComments(true)
		, mLastClick(-1.0f)
		, mHandleKeyboardInputs(true)
		, mHandleMouseInputs(true)
		, mIgnoreImGuiChild(false)
		, mShowWhitespaces(false)
		, mDebugCurrentLineUpdated(false)
		, mDebugCurrentLine(-1)
		, mPath("")
		, OnContentUpdate(nullptr)
		, mFuncTooltips(true)
		, mUIScale(1.0f)
		, mUIFontSize(18.0f)
		, mEditorFontSize(18.0f)
		, mActiveAutocomplete(false)
		, m_readyForAutocomplete(false)
		, m_requestAutocomplete(false)
		, mScrollbarMarkers(false)
		, mAutoindentOnPaste(false)
		, mIsSnippet(false)
		, mSnippetTagSelected(0)
		, mSidebar(true)
		, mHasSearch(true)
		, mReplaceIndex(0)
		, mStartTime(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count())
	{
		memset(mFindWord, 0, 256 * sizeof(char));
		memset(mReplaceWord, 0, 256 * sizeof(char));
		mFindOpened = false;
		mReplaceOpened = false;
		mFindJustOpened = false;
		mFindFocused = false;
		mReplaceFocused = false;

		OnSave = nullptr;
		OnDebuggerJump = nullptr;
		OnDebuggerAction = nullptr;
		OnBreakpointRemove = nullptr;
		OnBreakpointUpdate = nullptr;
		OnIdentifierHover = nullptr;
		HasIdentifierHover = nullptr;
		OnExpressionHover = nullptr;
		HasExpressionHover = nullptr;

		mDebugBarWidth = 0.0f;
		mDebugBarHeight = 0.0f;

		SetPalette(GetDarkPalette());
		SetLanguageDefinition(LanguageDefinition::HLSL());
		mLines.push_back(Line());

		m_shortcuts = GetDefaultShortcuts();
	}

	const std::vector<Shortcut> TextEditor::GetDefaultShortcuts()
	{
		std::vector<Shortcut> ret;
		ret.resize((int)ShortcutID::Count);

		ret[(int)ShortcutID::Undo] = Shortcut(SDLK_Z, -1, 0, 1, 0); // CTRL+Z
		ret[(int)ShortcutID::Redo] = Shortcut(SDLK_Y, -1, 0, 1, 0); // CTRL+Y
		ret[(int)ShortcutID::MoveUp] = Shortcut(SDLK_UP, -1, 0, 0, 0); // UP ARROW
		ret[(int)ShortcutID::SelectUp] = Shortcut(SDLK_UP, -1, 0, 0, 1); // SHIFT + UP ARROW
		ret[(int)ShortcutID::MoveDown] = Shortcut(SDLK_DOWN, -1, 0, 0, 0); // DOWN ARROW
		ret[(int)ShortcutID::SelectDown] = Shortcut(SDLK_DOWN, -1, 0, 0, 1); // SHIFT + DOWN ARROW
		ret[(int)ShortcutID::MoveLeft] = Shortcut(SDLK_LEFT, -1, 0, 0, 0); // LEFT ARROW (+ SHIFT/CTRL)
		ret[(int)ShortcutID::SelectLeft] = Shortcut(SDLK_LEFT, -1, 0, 0, 1); // SHIFT + LEFT ARROW
		ret[(int)ShortcutID::MoveWordLeft] = Shortcut(SDLK_LEFT, -1, 0, 1, 0); // CTRL + LEFT ARROW
		ret[(int)ShortcutID::SelectWordLeft] = Shortcut(SDLK_LEFT, -1, 0, 1, 1); // CTRL + SHIFT + LEFT ARROW
		ret[(int)ShortcutID::MoveRight] = Shortcut(SDLK_RIGHT, -1, 0, 0, 0); // RIGHT ARROW
		ret[(int)ShortcutID::SelectRight] = Shortcut(SDLK_RIGHT, -1, 0, 0, 1); // SHIFT + RIGHT ARROW
		ret[(int)ShortcutID::MoveWordRight] = Shortcut(SDLK_RIGHT, -1, 0, 1, 0); // CTRL + RIGHT ARROW
		ret[(int)ShortcutID::SelectWordRight] = Shortcut(SDLK_RIGHT, -1, 0, 1, 1); // CTRL + SHIFT + RIGHT ARROW
		ret[(int)ShortcutID::MoveUpBlock] = Shortcut(SDLK_PAGEUP, -1, 0, 0, 0); // PAGE UP
		ret[(int)ShortcutID::SelectUpBlock] = Shortcut(SDLK_PAGEUP, -1, 0, 0, 1); // SHIFT + PAGE UP
		ret[(int)ShortcutID::MoveDownBlock] = Shortcut(SDLK_PAGEDOWN, -1, 0, 0, 0); // PAGE DOWN
		ret[(int)ShortcutID::SelectDownBlock] = Shortcut(SDLK_PAGEDOWN, -1, 0, 0, 1); // SHIFT + PAGE DOWN
		ret[(int)ShortcutID::MoveTop] = Shortcut(SDLK_HOME, -1, 0, 1, 0); // CTRL + HOME
		ret[(int)ShortcutID::SelectTop] = Shortcut(SDLK_HOME, -1, 0, 1, 1); // CTRL + SHIFT + HOME
		ret[(int)ShortcutID::MoveBottom] = Shortcut(SDLK_END, -1, 0, 1, 0); // CTRL + END
		ret[(int)ShortcutID::SelectBottom] = Shortcut(SDLK_END, -1, 0, 1, 1); // CTRL + SHIFT + END
		ret[(int)ShortcutID::MoveStartLine] = Shortcut(SDLK_HOME, -1, 0, 0, 0); // HOME
		ret[(int)ShortcutID::SelectStartLine] = Shortcut(SDLK_HOME, -1, 0, 0, 1); // SHIFT + HOME
		ret[(int)ShortcutID::MoveEndLine] = Shortcut(SDLK_END, -1, 0, 0, 0); // END
		ret[(int)ShortcutID::SelectEndLine] = Shortcut(SDLK_END, -1, 0, 0, 1); // SHIFT + END
		ret[(int)ShortcutID::ForwardDelete] = Shortcut(SDLK_DELETE, -1, 0, 0, 0); // DELETE
		ret[(int)ShortcutID::ForwardDeleteWord] = Shortcut(SDLK_DELETE, -1, 0, 1, 0); // CTRL + DELETE
		ret[(int)ShortcutID::DeleteRight] = Shortcut(SDLK_DELETE, -1, 0, 0, 1); // SHIFT+BACKSPACE
		ret[(int)ShortcutID::BackwardDelete] = Shortcut(SDLK_BACKSPACE, -1, 0, 0, 0); // BACKSPACE
		ret[(int)ShortcutID::BackwardDeleteWord] = Shortcut(SDLK_BACKSPACE, -1, 0, 1, 0); // CTRL + BACKSPACE
		ret[(int)ShortcutID::DeleteLeft] = Shortcut(SDLK_BACKSPACE, -1, 0, 0, 1); // SHIFT+BACKSPACE
		ret[(int)ShortcutID::OverwriteCursor] = Shortcut(SDLK_INSERT, -1, 0, 0, 0); // INSERT
		ret[(int)ShortcutID::Copy] = Shortcut(SDLK_C, -1, 0, 1, 0); // CTRL+C
		ret[(int)ShortcutID::Paste] = Shortcut(SDLK_V, -1, 0, 1, 0); // CTRL+V
		ret[(int)ShortcutID::Cut] = Shortcut(SDLK_X, -1, 0, 1, 0); // CTRL+X
		ret[(int)ShortcutID::SelectAll] = Shortcut(SDLK_A, -1, 0, 1, 0); // CTRL+A
		ret[(int)ShortcutID::AutocompleteOpen] = Shortcut(SDLK_SPACE, -1, 0, 1, 0); // CTRL+SPACE
		ret[(int)ShortcutID::AutocompleteSelect] = Shortcut(SDLK_TAB, -1, 0, 0, 0); // TAB
		ret[(int)ShortcutID::AutocompleteSelectActive] = Shortcut(SDLK_RETURN, -1, 0, 0, 0); // RETURN
		ret[(int)ShortcutID::AutocompleteUp] = Shortcut(SDLK_UP, -1, 0, 0, 0); // UP ARROW
		ret[(int)ShortcutID::AutocompleteDown] = Shortcut(SDLK_DOWN, -1, 0, 0, 0); // DOWN ARROW
		ret[(int)ShortcutID::NewLine] = Shortcut(SDLK_RETURN, -1, 0, 0, 0); // RETURN
		ret[(int)ShortcutID::Indent] = Shortcut(SDLK_TAB, -1, 0, 0, 0); // TAB
		ret[(int)ShortcutID::Unindent] = Shortcut(SDLK_TAB, -1, 0, 0, 1); // SHIFT + TAB
		ret[(int)ShortcutID::Find] = Shortcut(SDLK_F, -1, 0, 1, 0); // CTRL+F
		ret[(int)ShortcutID::Replace] = Shortcut(SDLK_H, -1, 0, 1, 0); // CTRL+H
		ret[(int)ShortcutID::FindNext] = Shortcut(SDLK_F3, -1, 0, 0, 0); // F3
		ret[(int)ShortcutID::DebugStep] = Shortcut(SDLK_F10, -1, 0, 0, 0); // F10
		ret[(int)ShortcutID::DebugStepInto] = Shortcut(SDLK_F11, -1, 0, 0, 0); // F11
		ret[(int)ShortcutID::DebugStepOut] = Shortcut(SDLK_F11, -1, 0, 0, 1); // SHIFT+F11
		ret[(int)ShortcutID::DebugContinue] = Shortcut(SDLK_F5, -1, 0, 0, 0); // F5
		ret[(int)ShortcutID::DebugStop] = Shortcut(SDLK_F5, -1, 0, 0, 1); // SHIFT+F5
		ret[(int)ShortcutID::DebugBreakpoint] = Shortcut(SDLK_F9, -1, 0, 0, 0); // F9
		ret[(int)ShortcutID::DebugJumpHere] = Shortcut(SDLK_H, -1, 1, 1, 0); // CTRL+ALT+H
		ret[(int)ShortcutID::DuplicateLine] = Shortcut(SDLK_D, -1, 0, 1, 0); // CTRL+D
		ret[(int)ShortcutID::Save] = Shortcut(SDLK_S, -1, 0, 1, 0); // CTRL+S
		 
		return ret;
	}

	TextEditor::~TextEditor()
	{
	}

	void TextEditor::SetLanguageDefinition(const LanguageDefinition& aLanguageDef)
	{
		mLanguageDefinition = aLanguageDef;
		mRegexList.clear();

		for (auto& r : mLanguageDefinition.mTokenRegexStrings)
			mRegexList.push_back(std::make_pair(std::regex(r.first, std::regex_constants::optimize), r.second));

		Colorize();
	}

	void TextEditor::SetPalette(const Palette& aValue)
	{
		mPaletteBase = aValue;
	}

	std::string TextEditor::GetText(const Coordinates& aStart, const Coordinates& aEnd) const
	{
		std::string result;

		auto lstart = aStart.mLine;
		auto lend = aEnd.mLine;
		auto istart = GetCharacterIndex(aStart);
		auto iend = GetCharacterIndex(aEnd);
		size_t s = 0;

		for (size_t i = lstart; i < lend; i++)
			s += mLines[i].size();

		result.reserve(s + s / 8);

		while (istart < iend || lstart < lend)
		{
			if (lstart >= (int)mLines.size())
				break;

			auto& line = mLines[lstart];
			if (istart < (int)line.size())
			{
				result += line[istart].mChar;
				istart++;
			}
			else
			{
				istart = 0;
				if (!(lstart == lend - 1 && iend == -1))
					result += '\n';
				++lstart;
			}
		}

		return result;
	}

	Coordinates TextEditor::GetActualCursorCoordinates() const
	{
		return SanitizeCoordinates(mState.mCursorPosition);
	}

	Coordinates TextEditor::SanitizeCoordinates(const Coordinates& aValue) const
	{
		auto line = aValue.mLine;
		auto column = aValue.mColumn;
		if (line >= (int)mLines.size())
		{
			if (mLines.empty())
			{
				line = 0;
				column = 0;
			}
			else
			{
				line = (int)mLines.size() - 1;
				column = GetLineMaxColumn(line);
			}
			return Coordinates(line, column);
		}
		else
		{
			column = mLines.empty() ? 0 : std::min(column, GetLineMaxColumn(line));
			return Coordinates(line, column);
		}
	}

	// https://en.wikipedia.org/wiki/UTF-8
	// We assume that the char is a standalone character (<128) or a leading byte of an UTF-8 code sequence (non-10xxxxxx code)
	static int UTF8CharLength(Char c)
	{
		if ((c & 0xFE) == 0xFC)
			return 6;
		if ((c & 0xFC) == 0xF8)
			return 5;
		if ((c & 0xF8) == 0xF0)
			return 4;
		else if ((c & 0xF0) == 0xE0)
			return 3;
		else if ((c & 0xE0) == 0xC0)
			return 2;
		return 1;
	}

	static bool IsUTFSequence(char c)
	{
		return (c & 0xC0) == 0x80;
	}

	// "Borrowed" from ImGui source
	static inline int ImTextCharToUtf8(char* buf, int buf_size, unsigned int c)
	{
		if (c < 0x80)
		{
			buf[0] = (char)c;
			return 1;
		}
		if (c < 0x800)
		{
			if (buf_size < 2) return 0;
			buf[0] = (char)(0xc0 + (c >> 6));
			buf[1] = (char)(0x80 + (c & 0x3f));
			return 2;
		}
		if (c >= 0xdc00 && c < 0xe000)
		{
			return 0;
		}
		if (c >= 0xd800 && c < 0xdc00)
		{
			if (buf_size < 4) return 0;
			buf[0] = (char)(0xf0 + (c >> 18));
			buf[1] = (char)(0x80 + ((c >> 12) & 0x3f));
			buf[2] = (char)(0x80 + ((c >> 6) & 0x3f));
			buf[3] = (char)(0x80 + ((c) & 0x3f));
			return 4;
		}
		//else if (c < 0x10000)
		{
			if (buf_size < 3) return 0;
			buf[0] = (char)(0xe0 + (c >> 12));
			buf[1] = (char)(0x80 + ((c >> 6) & 0x3f));
			buf[2] = (char)(0x80 + ((c) & 0x3f));
			return 3;
		}
	}

	void TextEditor::Advance(Coordinates& aCoordinates) const
	{
		if (aCoordinates.mLine < (int)mLines.size())
		{
			auto& line = mLines[aCoordinates.mLine];
			auto cindex = GetCharacterIndex(aCoordinates);

			if (cindex + 1 < (int)line.size())
			{
				auto delta = UTF8CharLength(line[cindex].mChar);
				cindex = std::min(cindex + delta, (int)line.size() - 1);
			}
			else
			{
				++aCoordinates.mLine;
				cindex = 0;
			}
			aCoordinates.mColumn = GetCharacterColumn(aCoordinates.mLine, cindex);
		}
	}

	void TextEditor::DeleteRange(const Coordinates& aStart, const Coordinates& aEnd)
	{
		assert(aEnd >= aStart);
		assert(!mReadOnly);

		//printf("D(%d.%d)-(%d.%d)\n", aStart.mLine, aStart.mColumn, aEnd.mLine, aEnd.mColumn);

		if (aEnd == aStart)
			return;

		auto start = GetCharacterIndex(aStart);
		auto end = GetCharacterIndex(aEnd);

		if (aStart.mLine == aEnd.mLine)
		{
			auto& line = mLines[aStart.mLine];
			auto n = GetLineMaxColumn(aStart.mLine);
			if (aEnd.mColumn >= n)
				line.erase(line.begin() + start, line.end());
			else
				line.erase(line.begin() + start, line.begin() + end);
		}
		else
		{
			auto& firstLine = mLines[aStart.mLine];
			auto& lastLine = mLines[aEnd.mLine];

			firstLine.erase(firstLine.begin() + start, firstLine.end());
			lastLine.erase(lastLine.begin(), lastLine.begin() + end);

			if (aStart.mLine < aEnd.mLine)
				firstLine.insert(firstLine.end(), lastLine.begin(), lastLine.end());

			if (aStart.mLine < aEnd.mLine)
				RemoveLine(aStart.mLine + 1, aEnd.mLine + 1);
		}

		mTextChanged = true;
		if (OnContentUpdate != nullptr)
			OnContentUpdate(this);
	}

	int TextEditor::InsertTextAt(Coordinates& /* inout */ aWhere, const char* aValue, bool indent)
	{
		assert(!mReadOnly);

		int autoIndentStart = 0;
		for (int i = 0; i < mLines[aWhere.mLine].size() && indent; i++) {
			Char ch = mLines[aWhere.mLine][i].mChar;
			if (ch == ' ')
				autoIndentStart++;
			else if (ch == '\t')
				autoIndentStart += mTabSize;
			else break;
		}

		int cindex = GetCharacterIndex(aWhere);
		int totalLines = 0;
		int autoIndent = autoIndentStart;
		while (*aValue != '\0')
		{
			assert(!mLines.empty());

			if (*aValue == '\r')
			{
				// skip
				++aValue;
			}
			else if (*aValue == '\n')
			{
				if (cindex < (int)mLines[aWhere.mLine].size())
				{
					auto& newLine = InsertLine(aWhere.mLine + 1);
					auto& line = mLines[aWhere.mLine];
					newLine.insert(newLine.begin(), line.begin() + cindex, line.end());
					line.erase(line.begin() + cindex, line.end());
				}
				else
				{
					InsertLine(aWhere.mLine + 1);
				}
				++aWhere.mLine;
				cindex = 0;
				aWhere.mColumn = 0;
				++totalLines;
				++aValue;

				if (indent) {
					bool lineIsAlreadyIndent = (isspace(*aValue) && *aValue != '\n' && *aValue != '\r');

					// first check if we need to "unindent"
					const char* bracketSearch = aValue;
					while (*bracketSearch != '\0' && isspace(*bracketSearch) && *bracketSearch != '\n')
						bracketSearch++;
					if (*bracketSearch == '}')
						autoIndent = std::max(0, autoIndent - mTabSize);

					int actualAutoIndent = autoIndent;
					if (lineIsAlreadyIndent)
					{
						actualAutoIndent = autoIndentStart;

						const char* aValueCopy = aValue;
						while (isspace(*aValueCopy) && *aValueCopy != '\n' && *aValueCopy != '\r' && *aValueCopy != 0) {
							actualAutoIndent = std::max(0, actualAutoIndent - mTabSize);
							aValueCopy++;
						}
					}

					// add tabs
					int tabCount = actualAutoIndent / mTabSize;
					int spaceCount = actualAutoIndent - tabCount * mTabSize;
					if (mInsertSpaces) {
						tabCount = 0;
						spaceCount = actualAutoIndent;
					}

					cindex = tabCount + spaceCount;
					aWhere.mColumn = actualAutoIndent;

					while (spaceCount-- > 0) {
						mLines[aWhere.mLine].insert(mLines[aWhere.mLine].begin(), Glyph(' ', PaletteIndex::Default));
						for (int i = 0; i < mSnippetTagStart.size(); i++) {
							if (mSnippetTagStart[i].mLine == aWhere.mLine) {
								mSnippetTagStart[i].mColumn++;
								mSnippetTagEnd[i].mColumn++;
							}
						}
					}
					while (tabCount-- > 0) {
						mLines[aWhere.mLine].insert(mLines[aWhere.mLine].begin(), Glyph('\t', PaletteIndex::Default));
						for (int i = 0; i < mSnippetTagStart.size(); i++) {
							if (mSnippetTagStart[i].mLine == aWhere.mLine) {
								mSnippetTagStart[i].mColumn += mTabSize;
								mSnippetTagEnd[i].mColumn += mTabSize;
							}
						}
					}
				}
			}
			else
			{
				if (*aValue == '{')
					autoIndent += mTabSize;
				else if (*aValue == '}')
					autoIndent -= mTabSize;

				bool isTab = *aValue == '\t';
				auto& line = mLines[aWhere.mLine];
				auto d = UTF8CharLength(*aValue);
				while (d-- > 0 && *aValue != '\0')
					line.insert(line.begin() + cindex++, Glyph(*aValue++, PaletteIndex::Default));
				aWhere.mColumn += (isTab ? mTabSize : 1);
			}
		}

		mTextChanged = true;
		if (OnContentUpdate != nullptr)
			OnContentUpdate(this);

		return totalLines;
	}

	void TextEditor::AddUndo(UndoRecord& aValue)
	{
		assert(!mReadOnly);
		//printf("AddUndo: (@%d.%d) +\'%s' [%d.%d .. %d.%d], -\'%s', [%d.%d .. %d.%d] (@%d.%d)\n",
		//	aValue.mBefore.mCursorPosition.mLine, aValue.mBefore.mCursorPosition.mColumn,
		//	aValue.mAdded.c_str(), aValue.mAddedStart.mLine, aValue.mAddedStart.mColumn, aValue.mAddedEnd.mLine, aValue.mAddedEnd.mColumn,
		//	aValue.mRemoved.c_str(), aValue.mRemovedStart.mLine, aValue.mRemovedStart.mColumn, aValue.mRemovedEnd.mLine, aValue.mRemovedEnd.mColumn,
		//	aValue.mAfter.mCursorPosition.mLine, aValue.mAfter.mCursorPosition.mColumn
		//	);

		mUndoBuffer.resize((size_t)(mUndoIndex + 1));
		mUndoBuffer.back() = aValue;
		++mUndoIndex;
	}

	Coordinates TextEditor::ScreenPosToCoordinates(const ImVec2& aPosition) const
	{
		ImVec2 origin = mUICursorPos;
		ImVec2 local(aPosition.x - origin.x, aPosition.y - origin.y);

		int lineNo = std::max(0, (int)floor(local.y / mCharAdvance.y));

		int columnCoord = 0;

		if (lineNo >= 0 && lineNo < (int)mLines.size())
		{
			auto& line = mLines.at(lineNo);

			int columnIndex = 0;
			float columnX = 0.0f;

			while ((size_t)columnIndex < line.size())
			{
				float columnWidth = 0.0f;

				if (line[columnIndex].mChar == '\t')
				{
					float spaceSize = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, " ").x;
					float oldX = columnX;
					float newColumnX = (1.0f + std::floor((1.0f + columnX) / (float(mTabSize) * spaceSize))) * (float(mTabSize) * spaceSize);
					columnWidth = newColumnX - oldX;
					if (mTextStart + columnX + columnWidth * 0.5f > local.x)
						break;
					columnX = newColumnX;
					columnCoord = (columnCoord / mTabSize) * mTabSize + mTabSize;
					columnIndex++;
				}
				else
				{
					char buf[7];
					auto d = UTF8CharLength(line[columnIndex].mChar);
					int i = 0;
					while (i < 6 && d-- > 0)
						buf[i++] = line[columnIndex++].mChar;
					buf[i] = '\0';
					columnWidth = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, buf).x;
					if (mTextStart + columnX + columnWidth * 0.5f > local.x)
						break;
					columnX += columnWidth;
					columnCoord++;
				}
			}
		}

		return SanitizeCoordinates(Coordinates(lineNo, columnCoord));
	}
	Coordinates TextEditor::MousePosToCoordinates(const ImVec2& aPosition) const
	{
		ImVec2 origin = mUICursorPos;
		ImVec2 local(aPosition.x - origin.x, aPosition.y - origin.y);

		int lineNo = std::max(0, (int)floor(local.y / mCharAdvance.y));

		int columnCoord = 0;
		int modifier = 0;

		if (lineNo >= 0 && lineNo < (int)mLines.size()) {
			auto& line = mLines.at(lineNo);

			int columnIndex = 0;
			float columnX = 0.0f;

			while ((size_t)columnIndex < line.size()) {
				float columnWidth = 0.0f;

				if (line[columnIndex].mChar == '\t') {
					float spaceSize = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, " ").x;
					float oldX = columnX;
					float newColumnX = (1.0f + std::floor((1.0f + columnX) / (float(mTabSize) * spaceSize))) * (float(mTabSize) * spaceSize);
					columnWidth = newColumnX - oldX;
					if (mTextStart + columnX + columnWidth * 0.5f > local.x)
						break;
					columnX = newColumnX;
					columnCoord = (columnCoord / mTabSize) * mTabSize + mTabSize;
					columnIndex++;
					modifier += 3;
				}
				else {
					char buf[7];
					auto d = UTF8CharLength(line[columnIndex].mChar);
					int i = 0;
					while (i < 6 && d-- > 0)
						buf[i++] = line[columnIndex++].mChar;
					buf[i] = '\0';
					columnWidth = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, buf).x;
					if (mTextStart + columnX + columnWidth * 0.5f > local.x)
						break;
					columnX += columnWidth;
					columnCoord++;
				}
			}
		}

		return SanitizeCoordinates(Coordinates(lineNo, columnCoord - modifier));
	}

	Coordinates TextEditor::FindWordStart(const Coordinates& aFrom) const
	{
		Coordinates at = aFrom;
		if (at.mLine >= (int)mLines.size())
			return at;

		auto& line = mLines[at.mLine];
		auto cindex = GetCharacterIndex(at);

		if (cindex >= (int)line.size())
			return at;

		while (cindex > 0 && isspace(line[cindex].mChar))
			--cindex;

		auto cstart = (PaletteIndex)line[cindex].mColorIndex;
		while (cindex > 0)
		{
			auto c = line[cindex].mChar;
			if ((c & 0xC0) != 0x80)	// not UTF code sequence 10xxxxxx
			{
				if (c <= 32 && isspace(c))
				{
					cindex++;
					break;
				}
				if (cstart != (PaletteIndex)line[size_t(cindex - 1)].mColorIndex)
					break;
			}
			--cindex;
		}
		return Coordinates(at.mLine, GetCharacterColumn(at.mLine, cindex));
	}

	Coordinates TextEditor::FindWordEnd(const Coordinates& aFrom) const
	{
		Coordinates at = aFrom;
		if (at.mLine >= (int)mLines.size())
			return at;

		auto& line = mLines[at.mLine];
		auto cindex = GetCharacterIndex(at);

		if (cindex >= (int)line.size())
			return at;

		bool prevspace = (bool)isspace(line[cindex].mChar);
		auto cstart = (PaletteIndex)line[cindex].mColorIndex;
		while (cindex < (int)line.size())
		{
			auto c = line[cindex].mChar;
			auto d = UTF8CharLength(c);
			if (cstart != (PaletteIndex)line[cindex].mColorIndex)
				break;

			if (prevspace != !!isspace(c))
			{
				if (isspace(c))
					while (cindex < (int)line.size() && isspace(line[cindex].mChar))
						++cindex;
				break;
			}
			cindex += d;
		}
		return Coordinates(aFrom.mLine, GetCharacterColumn(aFrom.mLine, cindex));
	}

	Coordinates TextEditor::FindNextWord(const Coordinates& aFrom) const
	{
		Coordinates at = aFrom;
		if (at.mLine >= (int)mLines.size())
			return at;

		// skip to the next non-word character
		auto cindex = GetCharacterIndex(aFrom);
		bool isword = false;
		bool skip = false;
		if (cindex < (int)mLines[at.mLine].size())
		{
			auto& line = mLines[at.mLine];
			isword = isalnum(line[cindex].mChar);
			skip = isword;
		}

		while (!isword || skip)
		{
			if (at.mLine >= mLines.size())
			{
				auto l = std::max(0, (int)mLines.size() - 1);
				return Coordinates(l, GetLineMaxColumn(l));
			}

			auto& line = mLines[at.mLine];
			if (cindex < (int)line.size())
			{
				isword = isalnum(line[cindex].mChar);

				if (isword && !skip)
					return Coordinates(at.mLine, GetCharacterColumn(at.mLine, cindex));

				if (!isword)
					skip = false;

				cindex++;
			}
			else
			{
				cindex = 0;
				++at.mLine;
				skip = false;
				isword = false;
			}
		}

		return at;
	}

	int TextEditor::GetCharacterIndex(const Coordinates& aCoordinates) const
	{
		if (aCoordinates.mLine >= mLines.size())
			return -1;
		auto& line = mLines[aCoordinates.mLine];
		int c = 0;
		int i = 0;
		for (; i < line.size() && c < aCoordinates.mColumn;)
		{
			if (line[i].mChar == '\t')
				c = (c / mTabSize) * mTabSize + mTabSize;
			else
				++c;
			i += UTF8CharLength(line[i].mChar);
		}
		return i;
	}

	int TextEditor::GetCharacterColumn(int aLine, int aIndex) const
	{
		if (aLine >= mLines.size())
			return 0;
		auto& line = mLines[aLine];
		int col = 0;
		int i = 0;
		while (i < aIndex && i < (int)line.size())
		{
			auto c = line[i].mChar;
			i += UTF8CharLength(c);
			if (c == '\t')
				col = (col / mTabSize) * mTabSize + mTabSize;
			else
				col++;
		}
		return col;
	}

	int TextEditor::GetLineCharacterCount(int aLine) const
	{
		if (aLine >= mLines.size())
			return 0;
		auto& line = mLines[aLine];
		int c = 0;
		for (unsigned i = 0; i < line.size(); c++)
			i += UTF8CharLength(line[i].mChar);
		return c;
	}

	int TextEditor::GetLineMaxColumn(int aLine) const
	{
		if (aLine >= mLines.size())
			return 0;
		auto& line = mLines[aLine];
		int col = 0;
		for (unsigned i = 0; i < line.size(); )
		{
			auto c = line[i].mChar;
			if (c == '\t')
				col = (col / mTabSize) * mTabSize + mTabSize;
			else
				col++;
			i += UTF8CharLength(c);
		}
		return col;
	}

	bool TextEditor::IsOnWordBoundary(const Coordinates& aAt) const
	{
		if (aAt.mLine >= (int)mLines.size() || aAt.mColumn == 0)
			return true;

		auto& line = mLines[aAt.mLine];
		auto cindex = GetCharacterIndex(aAt);
		if (cindex >= (int)line.size())
			return true;

		if (mColorizerEnabled)
			return line[cindex].mColorIndex != line[size_t(cindex - 1)].mColorIndex;

		return isspace(line[cindex].mChar) != isspace(line[cindex - 1].mChar);
	}

	void TextEditor::RemoveLine(int aStart, int aEnd)
	{
		assert(!mReadOnly);
		assert(aEnd >= aStart);
		assert(mLines.size() > (size_t)(aEnd - aStart));

		ErrorMarkers etmp;
		for (auto& i : mErrorMarkers)
		{
			ErrorMarkers::value_type e(i.first >= aStart ? i.first - 1 : i.first, i.second);
			if (e.first >= aStart && e.first <= aEnd)
				continue;
			etmp.insert(e);
		}
		mErrorMarkers = std::move(etmp);

		auto btmp = mBreakpoints;
		mBreakpoints.clear();
		for (auto i : btmp)
		{
			if (i.mLine >= aStart && i.mLine <= aEnd) {
				RemoveBreakpoint(i.mLine);
				continue;
			}
			AddBreakpoint(i.mLine >= aStart ? i.mLine - 1 : i.mLine, i.mCondition, i.mEnabled);
		}

		mLines.erase(mLines.begin() + aStart, mLines.begin() + aEnd);
		assert(!mLines.empty());

		mTextChanged = true;
		if (OnContentUpdate != nullptr)
			OnContentUpdate(this);
	}

	void TextEditor::RemoveLine(int aIndex)
	{
		assert(!mReadOnly);
		assert(mLines.size() > 1);

		ErrorMarkers etmp;
		for (auto& i : mErrorMarkers)
		{
			ErrorMarkers::value_type e(i.first > aIndex ? i.first - 1 : i.first, i.second);
			if (e.first - 1 == aIndex)
				continue;
			etmp.insert(e);
		}
		mErrorMarkers = std::move(etmp);

		auto btmp = mBreakpoints;
		mBreakpoints.clear();
		for (auto i : btmp)
		{
			if (i.mLine == aIndex) {
				RemoveBreakpoint(i.mLine);
				continue;
			}
			AddBreakpoint(i.mLine >= aIndex ? i.mLine - 1 : i.mLine, i.mCondition, i.mEnabled);
		}

		mLines.erase(mLines.begin() + aIndex);
		assert(!mLines.empty());

		mTextChanged = true;
		if (OnContentUpdate != nullptr)
			OnContentUpdate(this);
	}

	Line& TextEditor::InsertLine(int aIndex)
	{
		assert(!mReadOnly);

		auto& result = *mLines.insert(mLines.begin() + aIndex, Line());

		ErrorMarkers etmp;
		for (auto& i : mErrorMarkers)
			etmp.insert(ErrorMarkers::value_type(i.first >= aIndex ? i.first + 1 : i.first, i.second));
		mErrorMarkers = std::move(etmp);

		auto btmp = mBreakpoints;
		mBreakpoints.clear();
		for (auto i : btmp)
			RemoveBreakpoint(i.mLine);
		for (auto i : btmp)
			AddBreakpoint(i.mLine >= aIndex ? i.mLine + 1 : i.mLine, i.mCondition, i.mEnabled);

		return result;
	}

	std::string TextEditor::GetWordUnderCursor() const
	{
		auto c = GetCursorPosition();
		c.mColumn = std::max(c.mColumn - 1, 0);
		return GetWordAt(c);
	}

	std::string TextEditor::GetWordAt(const Coordinates& aCoords) const
	{
		auto start = FindWordStart(aCoords);
		auto end = FindWordEnd(aCoords);

		std::string r;

		auto istart = GetCharacterIndex(start);
		auto iend = GetCharacterIndex(end);

		for (auto it = istart; it < iend; ++it)
			r.push_back(mLines[aCoords.mLine][it].mChar);

		return r;
	}

	ImU32 TextEditor::GetGlyphColor(const Glyph& aGlyph) const
	{
		if (!mColorizerEnabled)
			return mPalette[(int)PaletteIndex::Default];
		if (aGlyph.mComment)
			return mPalette[(int)PaletteIndex::Comment];
		if (aGlyph.mMultiLineComment)
			return mPalette[(int)PaletteIndex::MultiLineComment];
		auto const color = mPalette[(int)aGlyph.mColorIndex];
		if (aGlyph.mPreprocessor)
		{
			const auto ppcolor = mPalette[(int)PaletteIndex::Preprocessor];
			const int c0 = ((ppcolor & 0xff) + (color & 0xff)) / 2;
			const int c1 = (((ppcolor >> 8) & 0xff) + ((color >> 8) & 0xff)) / 2;
			const int c2 = (((ppcolor >> 16) & 0xff) + ((color >> 16) & 0xff)) / 2;
			const int c3 = (((ppcolor >> 24) & 0xff) + ((color >> 24) & 0xff)) / 2;
			return ImU32(c0 | (c1 << 8) | (c2 << 16) | (c3 << 24));
		}

		return color;
	}

	void TextEditor::HandleKeyboardInputs()
	{
		ImGuiIO& io = ImGui::GetIO();
		auto shift = io.KeyShift;
		auto ctrl = io.ConfigMacOSXBehaviors ? io.KeySuper : io.KeyCtrl;
		auto alt = io.ConfigMacOSXBehaviors ? io.KeyCtrl : io.KeyAlt;

		if (ImGui::IsWindowFocused())
		{
			// 设置鼠标指针
			if (ImGui::IsWindowHovered())
				ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);
			//ImGui::CaptureKeyboardFromApp(true);

			io.WantCaptureKeyboard = true;
			io.WantTextInput = true;

			// 遍历全部快捷键进行检查是否按下
			ShortcutID actionID = ShortcutID::Count;
			for (int i = 0; i < m_shortcuts.size(); i++) {
				auto sct = m_shortcuts[i];

				if (sct.Key1 == -1)
					continue;

				ShortcutID curActionID = ShortcutID::Count;
				bool additionalChecks = true;

				SDL_Keymod mod;
				SDL_Scancode sc1 = SDL_GetScancodeFromKey(sct.Key1, &mod);

				// 先对键盘快捷键进行额外检查，比如是否当前状态屏蔽了快捷键
				if ((ImGui::IsKeyPressed(SDLScancodeToImGuiKey(sc1)) || (sc1 == SDL_SCANCODE_RETURN && ImGui::IsKeyPressed(SDLScancodeToImGuiKey(SDL_SCANCODE_KP_ENTER))) && ((sct.Key2 != -1 && ImGui::IsKeyPressed(SDLScancodeToImGuiKey(SDL_GetScancodeFromKey(sct.Key2, &mod))) || sct.Key2 == -1)))) {
					if ((sct.Ctrl == ctrl) && (sct.Alt == alt) && (sct.Shift == shift)) {
						// PRESSED:
						curActionID = (ShortcutID)i;
						switch (curActionID) {
							// 以下快捷键在只读模式在已读模式无效
						case ShortcutID::Paste:
						case ShortcutID::Cut:
						case ShortcutID::Redo:
						case ShortcutID::Undo:
						case ShortcutID::ForwardDelete:
						case ShortcutID::BackwardDelete:
						case ShortcutID::DeleteLeft:
						case ShortcutID::DeleteRight:
						case ShortcutID::ForwardDeleteWord:
						case ShortcutID::BackwardDeleteWord:
							additionalChecks = !IsReadOnly();
							break;
							// 以下快捷键在自动补全模式无效
						case ShortcutID::MoveUp:
						case ShortcutID::MoveDown:
						case ShortcutID::SelectUp:
						case ShortcutID::SelectDown:
							additionalChecks = !mACOpened;
							break;
							// 以下快捷键在自动补全模式有效
						case ShortcutID::AutocompleteUp:
						case ShortcutID::AutocompleteDown:
						case ShortcutID::AutocompleteSelect:
							additionalChecks = mACOpened;
							break;
						case ShortcutID::AutocompleteSelectActive:
							additionalChecks = mACOpened && mACSwitched;
							break;
						case ShortcutID::NewLine:
						case ShortcutID::Indent:
						case ShortcutID::Unindent:
							additionalChecks = !IsReadOnly() && !mACOpened;
							break;
						default: break;
						}
					}
				}

				if (additionalChecks && curActionID != ShortcutID::Count)
					actionID = curActionID;
			}

			int keyCount = 0;
			bool keepACOpened = false;
			// 检查当前键盘动作是否是快捷键按下
			if (actionID != ShortcutID::Count)
			{
				if (actionID != ShortcutID::Indent)
					mIsSnippet = false;
				// 根据键盘快捷键做出不同操作
				switch (actionID) {
				case ShortcutID::Undo: Undo(); break;
				case ShortcutID::Redo: Redo(); break;
				case ShortcutID::MoveUp: MoveUp(1, false); break;
				case ShortcutID::SelectUp: MoveUp(1, true); break;
				case ShortcutID::MoveDown: MoveDown(1, false); break;
				case ShortcutID::SelectDown: MoveDown(1, true); break;
				case ShortcutID::MoveLeft: MoveLeft(1, false, false); break;
				case ShortcutID::SelectLeft: MoveLeft(1, true, false); break;
				case ShortcutID::MoveWordLeft: MoveLeft(1, false, true); break;
				case ShortcutID::SelectWordLeft: MoveLeft(1, true, true); break;
				case ShortcutID::MoveRight: MoveRight(1, false, false); break;
				case ShortcutID::SelectRight: MoveRight(1, true, false); break;
				case ShortcutID::MoveWordRight: MoveRight(1, false, true); break;
				case ShortcutID::SelectWordRight: MoveRight(1, true, true); break;
				case ShortcutID::MoveTop: MoveTop(false); break;
				case ShortcutID::SelectTop: MoveTop(true); break;
				case ShortcutID::MoveBottom: MoveBottom(false); break;
				case ShortcutID::SelectBottom: MoveBottom(true); break;
				case ShortcutID::MoveUpBlock: MoveUp(GetPageSize() - 4, false); break;
				case ShortcutID::MoveDownBlock: MoveDown(GetPageSize() - 4, false); break;
				case ShortcutID::SelectUpBlock: MoveUp(GetPageSize() - 4, true); break;
				case ShortcutID::SelectDownBlock: MoveDown(GetPageSize() - 4, true); break;
				case ShortcutID::MoveEndLine: MoveEnd(false); break;
				case ShortcutID::SelectEndLine: MoveEnd(true); break;
				case ShortcutID::MoveStartLine: MoveHome(false); break;
				case ShortcutID::SelectStartLine: MoveHome(true); break;
				case ShortcutID::DeleteRight:
				case ShortcutID::ForwardDelete: Delete(); break;
				case ShortcutID::ForwardDeleteWord:
					//if (ctrl)
					if (ctrl && mState.mSelectionStart == mState.mSelectionEnd) // Fix double click select, CTRL+delete & CTRL+backspace shortcuts
						MoveRight(1, true, true);
					Delete();
					break;
				case ShortcutID::DeleteLeft:
				case ShortcutID::BackwardDelete: Backspace(); break;
				case ShortcutID::BackwardDeleteWord:
					//if (ctrl)
					if (ctrl && (mState.mSelectionStart == mState.mSelectionEnd || mState.mSelectionStart == mState.mCursorPosition))	//Fix double click select, CTRL+delete & CTRL+backspace shortcuts
						MoveLeft(1, true, true);
					Backspace();
					break;
				case ShortcutID::OverwriteCursor: mOverwrite ^= true; break;
				case ShortcutID::Copy: Copy(); break;
				case ShortcutID::Paste: Paste(); break;
				case ShortcutID::Cut: Cut(); break;
				case ShortcutID::SelectAll: SelectAll(); break;
				case ShortcutID::AutocompleteOpen:
				{
					if (mAutocomplete && !mIsSnippet)
						m_buildSuggestions(&keepACOpened);
				} break;
				case ShortcutID::AutocompleteSelect:
				case ShortcutID::AutocompleteSelectActive:
				{
					mAutocompleteSelect();
				}
				break;
				case ShortcutID::AutocompleteUp:
					mACIndex = std::max<int>(mACIndex - 1, 0), mACSwitched = true;
					keepACOpened = true;
					break;
				case ShortcutID::AutocompleteDown:
					mACIndex = std::min<int>(mACIndex + 1, (int)mACSuggestions.size() - 1), mACSwitched = true;
					keepACOpened = true;
					break;
				case ShortcutID::NewLine:
					EnterCharacter('\n', false);
					break;
				case ShortcutID::Indent:
					if (mIsSnippet) {
						do {
							mSnippetTagSelected++;
							if (mSnippetTagSelected >= mSnippetTagStart.size())
								mSnippetTagSelected = 0;
						} while (!mSnippetTagHighlight[mSnippetTagSelected]);

						mSnippetTagLength = 0;
						mSnippetTagPreviousLength = mSnippetTagEnd[mSnippetTagSelected].mColumn - mSnippetTagStart[mSnippetTagSelected].mColumn;

						SetSelection(mSnippetTagStart[mSnippetTagSelected], mSnippetTagEnd[mSnippetTagSelected]);
						SetCursorPosition(mSnippetTagEnd[mSnippetTagSelected]);
					}
					else
						EnterCharacter('\t', false);
					break;
				case ShortcutID::Unindent:
					EnterCharacter('\t', true);
					break;
				case ShortcutID::Find: mFindOpened = mHasSearch; mFindJustOpened = mHasSearch; mReplaceOpened = false; break;
				case ShortcutID::Replace: mFindOpened = mHasSearch; mFindJustOpened = mHasSearch; mReplaceOpened = mHasSearch; break;
				case ShortcutID::DebugStep:
					if (OnDebuggerAction)
						OnDebuggerAction(this, DebugAction::Step);
					break;
				case ShortcutID::DebugStepInto:
					if (OnDebuggerAction)
						OnDebuggerAction(this, DebugAction::StepInto);
					break;
				case ShortcutID::DebugStepOut:
					if (OnDebuggerAction)
						OnDebuggerAction(this, DebugAction::StepOut);
					break;
				case ShortcutID::DebugContinue:
					if (OnDebuggerAction)
						OnDebuggerAction(this, DebugAction::Continue);
					break;
				case ShortcutID::DebugStop:
					if (OnDebuggerAction)
						OnDebuggerAction(this, DebugAction::Stop);
					break;
				case ShortcutID::DebugJumpHere:
					if (OnDebuggerJump)
						OnDebuggerJump(this, GetCursorPosition().mLine);
					break;
				case ShortcutID::DebugBreakpoint:
					if (OnBreakpointUpdate) {
						int line = GetCursorPosition().mLine + 1;
						if (HasBreakpoint(line))
							RemoveBreakpoint(line);
						else AddBreakpoint(line);
					}
					break;
				case ShortcutID::DuplicateLine: {
					TextEditor::UndoRecord undo;
					undo.mBefore = mState;

					auto oldLine = mLines[mState.mCursorPosition.mLine];
					auto& line = InsertLine(mState.mCursorPosition.mLine);

					undo.mAdded += '\n';
					for (auto& glyph : oldLine) {
						line.push_back(glyph);
						undo.mAdded += glyph.mChar;
					}
					mState.mCursorPosition.mLine++;

					undo.mAddedStart = Coordinates(mState.mCursorPosition.mLine - 1, mState.mCursorPosition.mColumn);
					undo.mAddedEnd = mState.mCursorPosition;

					undo.mAfter = mState;

					AddUndo(undo);
				} break;
				case ShortcutID::Save:
					if (OnSave)
						OnSave(this, GetPath());
					break;
				}
			}
			// 是键盘字符输入的情况
			else if (!IsReadOnly())
			{
				auto& characters = ImGui::GetIO().InputQueueCharacters;
				for (int i = 0; i < characters.size(); i++)
				{
					// 修复中文显示问题
					auto c = characters[i];
					if (c != 0 && (c == '\n' || c >= 32)) {
						// 输入字符
						EnterCharacter(c, shift);

						// 
						if (mIsSnippet) {
							mSnippetTagLength++;
							mSnippetTagEnd[mSnippetTagSelected].mColumn = mSnippetTagStart[mSnippetTagSelected].mColumn + mSnippetTagLength;

							Coordinates curCursor = GetCursorPosition();
							SetSelection(mSnippetTagStart[mSnippetTagSelected], mSnippetTagEnd[mSnippetTagSelected]);
							std::string curWord = GetSelectedText();
							std::unordered_map<int, int> modif;
							modif[curCursor.mLine] = 0;
							for (int j = 0; j < mSnippetTagStart.size(); j++) {
								if (j != mSnippetTagSelected) {
									mSnippetTagStart[j].mColumn += modif[mSnippetTagStart[j].mLine];
									mSnippetTagEnd[j].mColumn += modif[mSnippetTagStart[j].mLine];
								}
								if (mSnippetTagID[j] == mSnippetTagID[mSnippetTagSelected]) {
									modif[mSnippetTagStart[j].mLine] += mSnippetTagLength - mSnippetTagPreviousLength;

									if (j != mSnippetTagSelected) {
										SetSelection(mSnippetTagStart[j], mSnippetTagEnd[j]);
										Backspace();
										InsertText(curWord);
										mSnippetTagEnd[j].mColumn = mSnippetTagStart[j].mColumn + mSnippetTagLength;
									}
								}
							}
							SetSelection(curCursor, curCursor);
							SetCursorPosition(curCursor);
							EnsureCursorVisible();
							mSnippetTagPreviousLength = mSnippetTagLength;
						}
						keyCount++;
					}
				}
				characters.resize(0);
			}

			// 激活自动补全功能 active autocomplete
			if (m_requestAutocomplete && m_readyForAutocomplete && !mIsSnippet) {
				m_buildSuggestions(&keepACOpened);
				m_requestAutocomplete = false;
				m_readyForAutocomplete = false;
			}

			// 当自动补全打开并且存在任何键按键，而且没有匹配到后续自动补全字符就关闭自动补全
			// keepACOpened=自动补全有没有识别到后续匹配字符
			if (mACOpened && !keepACOpened) {
				// 如果存在任何键按下
				for (size_t i = ImGuiKey_NamedKey_BEGIN; i < ImGuiKey_NamedKey_END; i++)
					keyCount += ImGui::IsKeyPressed(static_cast<ImGuiKey>(i));

				// 就关闭自动补全
				if (keyCount != 0)
					mACOpened = false;
			}
		}
	}

	void TextEditor::HandleMouseInputs()
	{
		ImGuiIO& io = ImGui::GetIO();
		auto shift = io.KeyShift;
		auto ctrl = io.ConfigMacOSXBehaviors ? io.KeySuper : io.KeyCtrl;
		auto alt = io.ConfigMacOSXBehaviors ? io.KeyCtrl : io.KeyAlt;

		if (ImGui::IsWindowHovered()) {
			auto click = ImGui::IsMouseClicked(0);
			if ((!shift || (shift && click)) && !alt)
			{
				auto doubleClick = ImGui::IsMouseDoubleClicked(0);
				auto t = ImGui::GetTime();
				auto tripleClick = click && !doubleClick && (mLastClick != -1.0f && (t - mLastClick) < io.MouseDoubleClickTime);

				if (click || doubleClick || tripleClick)
					mIsSnippet = false;

				/*
					Left mouse button triple click
				*/

				if (tripleClick)
				{
					if (!ctrl)
					{
						mState.mCursorPosition = mInteractiveStart = mInteractiveEnd = ScreenPosToCoordinates(ImGui::GetMousePos());
						mSelectionMode = SelectionMode::Line;
						SetSelection(mInteractiveStart, mInteractiveEnd, mSelectionMode);
					}

					mLastClick = -1.0f;
				}

				/*
					Left mouse button double click
				*/

				else if (doubleClick)
				{
					if (!ctrl)
					{
						mState.mCursorPosition = mInteractiveStart = mInteractiveEnd = ScreenPosToCoordinates(ImGui::GetMousePos());
						if (mSelectionMode == SelectionMode::Line)
							mSelectionMode = SelectionMode::Normal;
						else
							mSelectionMode = SelectionMode::Word;
						SetSelection(mInteractiveStart, mInteractiveEnd, mSelectionMode);
						//Fix double click select, CTRL+delete & CTRL+backspace shortcuts
						mState.mCursorPosition = mState.mSelectionEnd;
					}

					mLastClick = (float)ImGui::GetTime();
				}

				/*
					mouse button click
				*/
				else if (click)
				{
					ImVec2 pos = ImGui::GetMousePos();

					if (pos.x - mUICursorPos.x < ImGui::GetStyle().WindowPadding.x + mEditorCalculateSize(DebugDataSpace)) {
						Coordinates lineInfo = ScreenPosToCoordinates(ImGui::GetMousePos());
						lineInfo.mLine += 1;

						if (HasBreakpoint(lineInfo.mLine))
							RemoveBreakpoint(lineInfo.mLine);
						else AddBreakpoint(lineInfo.mLine);
					}
					else {
						mACOpened = false;

						auto tcoords = ScreenPosToCoordinates(ImGui::GetMousePos());

						if (!shift)
							mInteractiveStart = tcoords;

						mState.mCursorPosition = mInteractiveEnd = tcoords;

						if (ctrl && !shift)
							mSelectionMode = SelectionMode::Word;
						else
							mSelectionMode = SelectionMode::Normal;
						SetSelection(mInteractiveStart, mInteractiveEnd, mSelectionMode);

						mLastClick = (float)ImGui::GetTime();
					}
				}
				// Mouse left button dragging (=> update selection)
				else if (ImGui::IsMouseDragging(0) && ImGui::IsMouseDown(0))
				{
					io.WantCaptureMouse = true;
					mState.mCursorPosition = mInteractiveEnd = ScreenPosToCoordinates(ImGui::GetMousePos());
					SetSelection(mInteractiveStart, mInteractiveEnd, mSelectionMode);

					float mx = ImGui::GetMousePos().x;
					if (mx > mFindOrigin.x + mWindowWidth - 50 && mx < mFindOrigin.x + mWindowWidth)
						ImGui::SetScrollX(ImGui::GetScrollX() + 1.0f);
					else if (mx > mFindOrigin.x && mx < mFindOrigin.x + mTextStart + 50)
						ImGui::SetScrollX(ImGui::GetScrollX() - 1.0f);
				}
			}
		}
	}

	bool TextEditor::HasBreakpoint(int line)
	{
		for (const auto& bkpt : mBreakpoints)
			if (bkpt.mLine == line)
				return true;
		return false;
	}
	void TextEditor::AddBreakpoint(int line, std::string condition, bool enabled)
	{
		RemoveBreakpoint(line);

		Breakpoint bkpt;
		bkpt.mLine = line;
		bkpt.mCondition = condition;
		bkpt.mEnabled = enabled;

		if (OnBreakpointUpdate)
			OnBreakpointUpdate(this, line, condition, enabled);

		mBreakpoints.push_back(bkpt);
	}
	void TextEditor::RemoveBreakpoint(int line)
	{
		for (int i = 0; i < mBreakpoints.size(); i++)
			if (mBreakpoints[i].mLine == line) {
				mBreakpoints.erase(mBreakpoints.begin() + i);
				break;
			}
		if (OnBreakpointRemove)
			OnBreakpointRemove(this, line);
	}
	void TextEditor::SetBreakpointEnabled(int line, bool enable)
	{
		for (int i = 0; i < mBreakpoints.size(); i++)
			if (mBreakpoints[i].mLine == line) {
				mBreakpoints[i].mEnabled = enable;
				if (OnBreakpointUpdate)
					OnBreakpointUpdate(this, line, mBreakpoints[i].mCondition, enable);
				break;
			}
	}
	static Breakpoint bp;

	Breakpoint& TextEditor::GetBreakpoint(int line)
	{
		for (int i = 0; i < mBreakpoints.size(); i++)
			if (mBreakpoints[i].mLine == line)
				return mBreakpoints[i];

		return bp;
	}

	void TextEditor::RenderInternal(const char* aTitle)
	{
		/* Compute mCharAdvance regarding to scaled font size (Ctrl + mouse wheel)*/
		const float fontSize = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, "#", nullptr, nullptr).x;
		mCharAdvance = ImVec2(fontSize, ImGui::GetTextLineHeightWithSpacing() * mLineSpacing);

		/* Update palette with the current alpha from style */
		for (int i = 0; i < (int)PaletteIndex::Max; ++i)
		{
			auto color = ImGui::ColorConvertU32ToFloat4(mPaletteBase[i]);
			color.w *= ImGui::GetStyle().Alpha;
			mPalette[i] = ImGui::ColorConvertFloat4ToU32(color);
		}

		assert(mLineBuffer.empty());
		mFocused = ImGui::IsWindowFocused() || mFindFocused || mReplaceFocused;

		auto contentSize = ImGui::GetWindowContentRegionMax();
		auto drawList = ImGui::GetWindowDrawList();
		float longest(mTextStart);

		if (mScrollToTop)
		{
			mScrollToTop = false;
			ImGui::SetScrollY(0.f);
		}

		ImVec2 cursorScreenPos = mUICursorPos = ImGui::GetCursorScreenPos();
		auto scrollX = ImGui::GetScrollX();
		auto scrollY = ImGui::GetScrollY();

		auto lineNo = (int)floor(scrollY / mCharAdvance.y);
		auto globalLineMax = (int)mLines.size();
		auto lineMax = std::max<int>(0, std::min<int>((int)mLines.size() - 1, lineNo + (int)floor((scrollY + contentSize.y) / mCharAdvance.y)));

		// Deduce mTextStart by evaluating mLines size (global lineMax) plus two spaces as text width
		char buf[16];
		snprintf(buf, 16, " %3d ", globalLineMax);
		mTextStart = (ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, buf, nullptr, nullptr).x + mLeftMargin) * mSidebar;

		if (!mLines.empty())
		{
			float spaceSize = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, " ", nullptr, nullptr).x;

			// find bracket pairs to highlight
			bool highlightBrackets = false;
			Coordinates highlightBracketCoord = mState.mCursorPosition, highlightBracketCursor = mState.mCursorPosition;
			if (mHighlightBrackets && mState.mSelectionStart == mState.mSelectionEnd) {
				if (mState.mCursorPosition.mLine <= mLines.size()) {
					int lineSize = mLines[mState.mCursorPosition.mLine].size();
					Coordinates start = GetCursorPosition();
					start.mColumn = std::min<int>(start.mColumn, mLines[start.mLine].size() - 1);
					highlightBracketCursor = start;

					if (lineSize != 0 && start.mLine >= 0 && start.mLine < mLines.size() && start.mColumn >= 0 && start.mColumn < mLines[start.mLine].size()) {
						char c1 = mLines[start.mLine][start.mColumn].mChar;
						char c2 = mLines[start.mLine][std::max<int>(0, start.mColumn - 1)].mChar;
						int b1 = isBracket(c1);
						int b2 = isBracket(c2);

						if (b1 || b2) {
							char search = c1;
							int dir = b1;
							if (b2) {
								search = c2;
								dir = b2;
								start.mColumn = std::max<int>(0, start.mColumn - 1);
								highlightBracketCursor = start;
							}

							int weight = 0;

							// go forward
							if (dir == 1) {
								while (start.mLine < mLines.size()) {
									for (; start.mColumn < mLines[start.mLine].size(); start.mColumn++) {
										char curChar = mLines[start.mLine][start.mColumn].mChar;
										if (curChar == search)
											weight++;
										else if (isClosingBracket(search, curChar)) {
											weight--;
											if (weight <= 0) {
												highlightBrackets = true;
												highlightBracketCoord = start;
												break;
											}
										}
									}

									if (highlightBrackets)
										break;

									start.mLine++;
									start.mColumn = 0;
								}
							}
							// go backwards
							else {
								while (start.mLine >= 0) {
									for (; start.mColumn >= 0; start.mColumn--) {
										char curChar = mLines[start.mLine][start.mColumn].mChar;
										if (curChar == search)
											weight++;
										else if (isOpeningBracket(search, curChar)) {
											weight--;
											if (weight <= 0) {
												highlightBrackets = true;
												highlightBracketCoord = start;
												break;
											}
										}
									}

									if (highlightBrackets)
										break;

									start.mLine--;

									if (start.mLine >= 0)
										start.mColumn = mLines[start.mLine].size() - 1;
								}
							}
						}
					}
				}
			}

			// render

			while (lineNo <= lineMax)
			{
				ImVec2 lineStartScreenPos = ImVec2(cursorScreenPos.x, cursorScreenPos.y + lineNo * mCharAdvance.y);
				ImVec2 textScreenPos = ImVec2(lineStartScreenPos.x + mTextStart, lineStartScreenPos.y);

				auto& line = mLines[lineNo];
				longest = std::max(mTextStart + TextDistanceToLineStart(Coordinates(lineNo, GetLineMaxColumn(lineNo))), longest);
				auto columnNo = 0;
				Coordinates lineStartCoord(lineNo, 0);
				Coordinates lineEndCoord(lineNo, GetLineMaxColumn(lineNo));

				// Draw selection for the current line
				float sstart = -1.0f;
				float ssend = -1.0f;

				assert(mState.mSelectionStart <= mState.mSelectionEnd);
				if (mState.mSelectionStart <= lineEndCoord)
					sstart = mState.mSelectionStart > lineStartCoord ? TextDistanceToLineStart(mState.mSelectionStart) : 0.0f;
				if (mState.mSelectionEnd > lineStartCoord)
					ssend = TextDistanceToLineStart(mState.mSelectionEnd < lineEndCoord ? mState.mSelectionEnd : lineEndCoord);

				if (mState.mSelectionEnd.mLine > lineNo)
					ssend += mCharAdvance.x;

				if (sstart != -1 && ssend != -1 && sstart < ssend)
				{
					ImVec2 vstart(lineStartScreenPos.x + mTextStart + sstart, lineStartScreenPos.y);
					ImVec2 vend(lineStartScreenPos.x + mTextStart + ssend, lineStartScreenPos.y + mCharAdvance.y);
					drawList->AddRectFilled(vstart, vend, mPalette[(int)PaletteIndex::Selection]);
				}

				if (mIsSnippet) {
					unsigned int oldColor = mPalette[(int)PaletteIndex::Selection];
					unsigned int alpha = (oldColor & 0xFF000000) >> 25;
					unsigned int newColor = (oldColor & 0x00FFFFFF) | (alpha << 24);

					for (int i = 0; i < mSnippetTagStart.size(); i++) {
						if (mSnippetTagStart[i].mLine == lineNo && mSnippetTagHighlight[i]) {
							float tstart = TextDistanceToLineStart(mSnippetTagStart[i]);
							float tend = TextDistanceToLineStart(mSnippetTagEnd[i]);

							ImVec2 vstart(lineStartScreenPos.x + mTextStart + tstart, lineStartScreenPos.y);
							ImVec2 vend(lineStartScreenPos.x + mTextStart + tend, lineStartScreenPos.y + mCharAdvance.y);
							drawList->AddRectFilled(vstart, vend, newColor);
						}
					}
				}

				auto start = ImVec2(lineStartScreenPos.x + scrollX, lineStartScreenPos.y);

				// Draw error markers
				auto errorIt = mErrorMarkers.find(lineNo + 1);
				if (errorIt != mErrorMarkers.end())
				{
					auto end = ImVec2(lineStartScreenPos.x + contentSize.x + 2.0f * scrollX, lineStartScreenPos.y + mCharAdvance.y);
					drawList->AddRectFilled(start, end, mPalette[(int)PaletteIndex::ErrorMarker]);

					if (ImGui::IsMouseHoveringRect(lineStartScreenPos, end))
					{
						ImGui::BeginTooltip();
						ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(mPalette[(int)PaletteIndex::ErrorMessage]));
						ImGui::Text("Error at line %d:", errorIt->first);
						ImGui::PopStyleColor();
						ImGui::Separator();
						ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(mPalette[(int)PaletteIndex::ErrorMessage]));
						ImGui::Text("%s", errorIt->second.c_str());
						ImGui::PopStyleColor();
						ImGui::EndTooltip();
					}
				}

				// Highlight the current line (where the cursor is)
				if (mState.mCursorPosition.mLine == lineNo)
				{
					auto focused = ImGui::IsWindowFocused();

					// Highlight the current line (where the cursor is)
					if (mHighlightLine && !HasSelection()) {
						auto end = ImVec2(start.x + contentSize.x + scrollX, start.y + mCharAdvance.y + 2.0f);
						drawList->AddRectFilled(start, end, mPalette[(int)(focused ? PaletteIndex::CurrentLineFill : PaletteIndex::CurrentLineFillInactive)]);
						drawList->AddRect(start, end, mPalette[(int)PaletteIndex::CurrentLineEdge], 1.0f);
					}

					// Render the cursor
					if (focused)
					{
						auto timeEnd = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
						auto elapsed = timeEnd - mStartTime;
						if (elapsed > 400)
						{
							float width = 1.0f;
							auto cindex = GetCharacterIndex(mState.mCursorPosition);
							float cx = TextDistanceToLineStart(mState.mCursorPosition);

							if (mOverwrite && cindex < (int)line.size())
							{
								auto c = line[cindex].mChar;
								if (c == '\t')
								{
									auto x = (1.0f + std::floor((1.0f + cx) / (float(mTabSize) * spaceSize))) * (float(mTabSize) * spaceSize);
									width = x - cx;
								}
								else
								{
									char buf2[2];
									buf2[0] = line[cindex].mChar;
									buf2[1] = '\0';
									width = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, buf2).x;
								}
							}
							ImVec2 cstart(textScreenPos.x + cx, lineStartScreenPos.y);
							ImVec2 cend(textScreenPos.x + cx + width, lineStartScreenPos.y + mCharAdvance.y);
							drawList->AddRectFilled(cstart, cend, mPalette[(int)PaletteIndex::Cursor]);
							if (elapsed > 800)
								mStartTime = timeEnd;
						}
					}
				}

				// Render colorized text
				auto prevColor = line.empty() ? mPalette[(int)PaletteIndex::Default] : GetGlyphColor(line[0]);
				ImVec2 bufferOffset;

				for (int i = 0; i < line.size();)
				{
					auto& glyph = line[i];
					auto color = GetGlyphColor(glyph);

					if ((color != prevColor || glyph.mChar == '\t' || glyph.mChar == ' ') && !mLineBuffer.empty())
					{
						const ImVec2 newOffset(textScreenPos.x + bufferOffset.x, textScreenPos.y + bufferOffset.y);
						drawList->AddText(newOffset, prevColor, mLineBuffer.c_str());
						auto textSize = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, mLineBuffer.c_str(), nullptr, nullptr);
						bufferOffset.x += textSize.x;
						mLineBuffer.clear();
					}
					prevColor = color;

					// highlight brackets
					if (highlightBrackets) {
						if ((lineNo == highlightBracketCoord.mLine && i == highlightBracketCoord.mColumn) ||
							(lineNo == highlightBracketCursor.mLine && i == highlightBracketCursor.mColumn)) {
							const ImVec2 p1(textScreenPos.x + bufferOffset.x, textScreenPos.y + bufferOffset.y);
							//const ImVec2 p2(textScreenPos.x + bufferOffset.x + ImGui::GetFont()->GetCharAdvance(mLines[highlightBracketCoord.mLine][highlightBracketCoord.mColumn].mChar), textScreenPos.y + bufferOffset.y + ImGui::GetFontSize());
							const ImVec2 p2(textScreenPos.x + bufferOffset.x + ImGui::GetFont()->GetFontBaked(ImGui::GetFontSize())->GetCharAdvance(mLines[highlightBracketCoord.mLine][highlightBracketCoord.mColumn].mChar), textScreenPos.y + bufferOffset.y + ImGui::GetFontSize());
							drawList->AddRectFilled(p1, p2, mPalette[(int)PaletteIndex::Selection]);
						}
					}

					// tab, space, etc...
					if (glyph.mChar == '\t')
					{
						auto oldX = bufferOffset.x;
						bufferOffset.x = (1.0f + std::floor((1.0f + bufferOffset.x) / (float(mTabSize) * spaceSize))) * (float(mTabSize) * spaceSize);
						++i;

						if (mShowTabArrow)
						{
							const auto s = ImGui::GetFontSize();
							const auto x1 = textScreenPos.x + oldX + 1.0f;
							const auto x2 = textScreenPos.x + bufferOffset.x - 1.0f;
							const auto y = textScreenPos.y + bufferOffset.y + s * 0.5f;
							const ImVec2 p1(x1, y);
							const ImVec2 p2(x2, y);
							const ImVec2 p3(x2 - s * 0.2f, y - s * 0.2f);
							const ImVec2 p4(x2 - s * 0.2f, y + s * 0.2f);
							drawList->AddLine(p1, p2, 0x90909090);
							drawList->AddLine(p2, p3, 0x90909090);
							drawList->AddLine(p2, p4, 0x90909090);
						}
					}
					else if (glyph.mChar == ' ')
					{
						if (mShowWhitespaces)
						{
							const auto s = ImGui::GetFontSize();
							const auto x = textScreenPos.x + bufferOffset.x + spaceSize * 0.5f;
							const auto y = textScreenPos.y + bufferOffset.y + s * 0.5f;
							drawList->AddCircleFilled(ImVec2(x, y), 1.5f, 0x80808080, 4);
						}
						bufferOffset.x += spaceSize;
						// todo 让空格显示更加宽
						i++;
					}
					else
					{
						auto l = UTF8CharLength(glyph.mChar);
						while (l-- > 0)
							mLineBuffer.push_back(line[i++].mChar);
					}
					++columnNo;
				}

				if (!mLineBuffer.empty())
				{
					const ImVec2 newOffset(textScreenPos.x + bufferOffset.x, textScreenPos.y + bufferOffset.y);
					drawList->AddText(newOffset, prevColor, mLineBuffer.c_str());
					mLineBuffer.clear();
				}

				// side bar bg
				if (mSidebar) {
					drawList->AddRectFilled(ImVec2(lineStartScreenPos.x + scrollX, lineStartScreenPos.y), ImVec2(lineStartScreenPos.x + scrollX + mTextStart - 5.0f, lineStartScreenPos.y + mCharAdvance.y), ImGui::GetColorU32(ImGuiCol_WindowBg));

					// Draw breakpoints
					if (HasBreakpoint(lineNo + 1) != 0) {
						float radius = ImGui::GetFontSize() * 1.0f / 3.0f;
						float startX = lineStartScreenPos.x + scrollX + radius + 2.0f;
						float startY = lineStartScreenPos.y + radius + 4.0f;

						drawList->AddCircle(ImVec2(startX, startY), radius + 1, mPalette[(int)PaletteIndex::BreakpointOutline]);
						drawList->AddCircleFilled(ImVec2(startX, startY), radius, mPalette[(int)PaletteIndex::Breakpoint]);

						Breakpoint bkpt = GetBreakpoint(lineNo + 1);
						if (!bkpt.mEnabled)
							drawList->AddCircleFilled(ImVec2(startX, startY), radius - 1, mPalette[(int)PaletteIndex::BreakpointDisabled]);
						else {
							if (!bkpt.mCondition.empty())
								drawList->AddRectFilled(ImVec2(startX - radius + 3, startY - radius / 4), ImVec2(startX + radius - 3, startY + radius / 4), mPalette[(int)PaletteIndex::BreakpointOutline]);
						}
					}

					// Draw current line indicator
					if (lineNo + 1 == mDebugCurrentLine) {
						float radius = ImGui::GetFontSize() * 1.0f / 3.0f;
						float startX = lineStartScreenPos.x + scrollX + radius + 2.0f;
						float startY = lineStartScreenPos.y + 4.0f;

						// outline
						drawList->AddRect(
							ImVec2(startX - radius, startY + radius / 2), ImVec2(startX, startY + radius * 3.0f / 2.0f),
							mPalette[(int)PaletteIndex::CurrentLineIndicatorOutline]);
						drawList->AddTriangle(
							ImVec2(startX - 1, startY - 2), ImVec2(startX - 1, startY + radius * 2 + 1), ImVec2(startX + radius, startY + radius),
							mPalette[(int)PaletteIndex::CurrentLineIndicatorOutline]);

						// fill
						drawList->AddRectFilled(
							ImVec2(startX - radius + 1, startY + 1 + radius / 2), ImVec2(startX + 1, startY - 1 + radius * 3.0f / 2.0f),
							mPalette[(int)PaletteIndex::CurrentLineIndicator]);
						drawList->AddTriangleFilled(
							ImVec2(startX, startY + 1), ImVec2(startX, startY - 1 + radius * 2), ImVec2(startX - 1 + radius, startY + radius),
							mPalette[(int)PaletteIndex::CurrentLineIndicator]);
					}

					// Draw line number (right aligned)
					if (mShowLineNumbers) {
						snprintf(buf, 16, "%3d  ", lineNo + 1);

						auto lineNoWidth = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, buf, nullptr, nullptr).x;
						drawList->AddText(ImVec2(lineStartScreenPos.x + scrollX + mTextStart - lineNoWidth, lineStartScreenPos.y), mPalette[(int)PaletteIndex::LineNumber], buf);
					}
				}

				++lineNo;
			}

			// Draw a tooltip on known identifiers/preprocessor symbols
			if (ImGui::IsMousePosValid() && (IsDebugging() || mFuncTooltips))
			{
				Coordinates hoverPosition = MousePosToCoordinates(ImGui::GetMousePos());
				if (hoverPosition != mLastHoverPosition) {
					mLastHoverPosition = hoverPosition;
					mLastHoverTime = std::chrono::steady_clock::now();
				}

				Char hoverChar = 0;
				if (hoverPosition.mLine < mLines.size() && hoverPosition.mColumn < mLines[hoverPosition.mLine].size())
					hoverChar = mLines[hoverPosition.mLine][hoverPosition.mColumn].mChar;

				double hoverTime = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - mLastHoverTime).count();

				if (hoverTime > 0.5 && (hoverChar == '(' || hoverChar == ')') && IsDebugging()) {
					std::string expr = "";

					int colStart = 0, rowStart = hoverPosition.mLine;
					int bracketMatch = 0;
					if (hoverChar == ')') {
						int colIndex = hoverPosition.mColumn;
						for (; rowStart >= 0; rowStart--) {
							for (int i = colIndex; i >= 0; i--) {
								char curChar = mLines[rowStart][i].mChar;
								if (curChar == '(')
									bracketMatch++;
								else if (curChar == ')')
									bracketMatch--;

								if (!isspace(curChar) || curChar == ' ')
									expr += curChar;

								if (bracketMatch == 0) {
									colStart = i - 1;
									break;
								}
							}
							if (bracketMatch == 0)
								break;
							if (rowStart != 0)
								colIndex = mLines[rowStart - 1].size() - 1;
						}
						std::reverse(expr.begin(), expr.end());

						if (rowStart <= 0)
							colStart = -1;
					}
					else if (hoverChar == '(') {
						int colIndex = hoverPosition.mColumn;
						colStart = hoverPosition.mColumn - 1;
						for (int j = rowStart; j < mLines.size(); j++) {
							for (int i = colIndex; i < mLines[j].size(); i++) {
								char curChar = mLines[j][i].mChar;
								if (curChar == '(')
									bracketMatch++;
								else if (curChar == ')')
									bracketMatch--;

								if (!isspace(curChar) || curChar == ' ')
									expr += curChar;

								if (bracketMatch == 0)
									break;
							}

							if (bracketMatch == 0)
								break;
							if (j != 0)
								colIndex = 0;
						}

						if (rowStart >= mLines.size())
							colStart = -1;
					}

					while (colStart >= 0 && isalnum(mLines[rowStart][colStart].mChar)) {
						expr.insert(expr.begin(), mLines[rowStart][colStart].mChar);
						colStart--;
					}

					if (OnExpressionHover && HasExpressionHover)
					{
						if (HasExpressionHover(this, expr)) {
							ImGui::BeginTooltip();
							OnExpressionHover(this, expr);
							ImGui::EndTooltip();
						}
					}
				}
				else if (hoverTime > 0.2) {
					auto id = GetWordAt(ScreenPosToCoordinates(ImGui::GetMousePos()));
					if (!id.empty()) {
						auto it = mLanguageDefinition.mIdentifiers.find(id);
						if (it != mLanguageDefinition.mIdentifiers.end()) {
							ImGui::BeginTooltip();
							ImGui::TextUnformatted(it->second.mDeclaration.c_str());
							ImGui::EndTooltip();
						}
						else {
							auto pi = mLanguageDefinition.mPreprocIdentifiers.find(id);
							if (pi != mLanguageDefinition.mPreprocIdentifiers.end()) {
								ImGui::BeginTooltip();
								ImGui::TextUnformatted(pi->second.mDeclaration.c_str());
								ImGui::EndTooltip();
							}
							else if (IsDebugging() && OnIdentifierHover && HasIdentifierHover) {
								if (HasIdentifierHover(this, id)) {
									ImGui::BeginTooltip();
									OnIdentifierHover(this, id);
									ImGui::EndTooltip();
								}
							}
						}
					}
				}
			}
		}

		// suggestions window
		if (mACOpened) {
			auto acCoord = FindWordStart(mACPosition);
			ImVec2 acPos = CoordinatesToScreenPos(acCoord);
			acPos.y += mCharAdvance.y;
			acPos.x += ImGui::GetScrollX();

			drawList->AddRectFilled(acPos, ImVec2(acPos.x + mUICalculateSize(150), acPos.y + mUICalculateSize(100)), ImGui::GetColorU32(ImGuiCol_FrameBg));

			ImFont* font = ImGui::GetFont();
			//ImGui::PopFont();

			ImGui::SetNextWindowPos(acPos, ImGuiCond_Always);
			ImGui::BeginChild("##texteditor_autocompl", ImVec2(mUICalculateSize(150), mUICalculateSize(100)), true);

			for (int i = 0; i < mACSuggestions.size(); i++) {
				ImGui::Selectable(mACSuggestions[i].first.c_str(), i == mACIndex);
				if (i == mACIndex)
					ImGui::SetScrollHereY();
			}

			ImGui::EndChild();

			//ImGui::PushFont(font);

			ImGui::SetWindowFocus();
			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
				mACOpened = false;
		}

		ImGui::Dummy(ImVec2(longest + mEditorCalculateSize(100), mLines.size() * mCharAdvance.y));

		if (mDebugCurrentLineUpdated) {
			float scrollX = ImGui::GetScrollX();
			float scrollY = ImGui::GetScrollY();

			auto height = ImGui::GetWindowHeight();
			auto width = ImGui::GetWindowWidth();

			auto top = 1 + (int)ceil(scrollY / mCharAdvance.y);
			auto bottom = (int)ceil((scrollY + height) / mCharAdvance.y);

			auto left = (int)ceil(scrollX / mCharAdvance.x);
			auto right = (int)ceil((scrollX + width) / mCharAdvance.x);

			Coordinates pos(mDebugCurrentLine, 0);

			if (pos.mLine < top)
				ImGui::SetScrollY(std::max(0.0f, (pos.mLine - 1) * mCharAdvance.y));
			if (pos.mLine > bottom - 4)
				ImGui::SetScrollY(std::max(0.0f, (pos.mLine + 4) * mCharAdvance.y - height));

			mDebugCurrentLineUpdated = false;
		}

		if (mScrollToCursor)
		{
			EnsureCursorVisible();
			ImGui::SetWindowFocus();
			mScrollToCursor = false;
		}

		// hacky way to get the bg working
		if (mFindOpened)
		{
			ImVec2 findPos = ImVec2(mUICursorPos.x + scrollX + mWindowWidth - mUICalculateSize(250), mUICursorPos.y + ImGui::GetScrollY() + mUICalculateSize(50) * IsDebugging());
			drawList->AddRectFilled(findPos, ImVec2(findPos.x + mUICalculateSize(220), findPos.y + mUICalculateSize(mReplaceOpened ? 90 : 40)), ImGui::GetColorU32(ImGuiCol_WindowBg));
		}
		if (IsDebugging())
		{
			ImVec2 dbgPos = ImVec2(mUICursorPos.x + scrollX + mWindowWidth / 2 - mDebugBarWidth / 2, mUICursorPos.y + ImGui::GetScrollY());
			drawList->AddRectFilled(dbgPos, ImVec2(dbgPos.x + mDebugBarWidth, dbgPos.y + mDebugBarHeight), ImGui::GetColorU32(ImGuiCol_FrameBg));
		}
	}

	std::string TextEditor::mAutcompleteParse(const std::string& str, const Coordinates& start)
	{
		const char* buffer = str.c_str();
		const char* tagPlaceholderStart = buffer;
		const char* tagStart = buffer;

		bool parsingTag = false;
		bool parsingTagPlaceholder = false;

		std::vector<int> tagIds, tagLocations, tagLengths;
		std::unordered_map<int, std::string> tagPlaceholders;

		mSnippetTagStart.clear();
		mSnippetTagEnd.clear();
		mSnippetTagID.clear();
		mSnippetTagHighlight.clear();

		Coordinates cursor = start, tagStartCoord, tagEndCoord;

		int tagId = -1;

		int modif = 0;
		while (*buffer != '\0') {
			if (*buffer == '{' && *(buffer + 1) == '$') {
				parsingTagPlaceholder = false;
				parsingTag = true;
				tagId = -1;
				tagStart = buffer;

				tagStartCoord = cursor;

				const char* skipBuffer = buffer;
				char** endLoc = const_cast<char**>(&buffer); // oops
				tagId = strtol(buffer + 2, endLoc, 10);

				cursor.mColumn += *endLoc - skipBuffer;

				if (*buffer == ':') {
					tagPlaceholderStart = buffer + 1;
					parsingTagPlaceholder = true;
				}
			}

			if (*buffer == '}' && parsingTag) {
				std::string tagPlaceholder = "";
				if (parsingTagPlaceholder)
					tagPlaceholder = std::string(tagPlaceholderStart, buffer - tagPlaceholderStart);

				tagIds.push_back(tagId);
				tagLocations.push_back(tagStart - str.c_str());
				tagLengths.push_back(buffer - tagStart + 1);
				if (!tagPlaceholder.empty() || tagPlaceholders.count(tagId) == 0) {
					if (tagPlaceholder.empty())
						tagPlaceholder = " ";

					tagStartCoord.mColumn = std::max<int>(0, tagStartCoord.mColumn - modif);
					tagEndCoord = tagStartCoord;
					tagEndCoord.mColumn += tagPlaceholder.size();

					mSnippetTagStart.push_back(tagStartCoord);
					mSnippetTagEnd.push_back(tagEndCoord);
					mSnippetTagID.push_back(tagId);
					mSnippetTagHighlight.push_back(true);

					tagPlaceholders[tagId] = tagPlaceholder;
				}
				else {
					tagStartCoord.mColumn = std::max<int>(0, tagStartCoord.mColumn - modif);
					tagEndCoord = tagStartCoord;
					tagEndCoord.mColumn += tagPlaceholders[tagId].size();

					mSnippetTagStart.push_back(tagStartCoord);
					mSnippetTagEnd.push_back(tagEndCoord);
					mSnippetTagID.push_back(tagId);
					mSnippetTagHighlight.push_back(false);
				}
				modif += (tagLengths.back() - tagPlaceholders[tagId].size());

				parsingTagPlaceholder = false;
				parsingTag = false;
				tagId = -1;
			}

			if (*buffer == '\n') {
				cursor.mLine++;
				cursor.mColumn = 0;
				modif = 0;
			}
			else
				cursor.mColumn++;

			buffer++;
		}

		mIsSnippet = !tagIds.empty();

		std::string ret = str;
		for (int i = tagLocations.size() - 1; i >= 0; i--) {
			ret.erase(tagLocations[i], tagLengths[i]);
			ret.insert(tagLocations[i], tagPlaceholders[tagIds[i]]);
		}

		return ret;
	}
	void TextEditor::mAutocompleteSelect()
	{
		UndoRecord undo;
		undo.mBefore = mState;

		auto curCoord = GetCursorPosition();
		curCoord.mColumn = std::max<int>(curCoord.mColumn - 1, 0);

		auto acStart = FindWordStart(curCoord);
		auto acEnd = FindWordEnd(curCoord);

		undo.mAddedStart = acStart;
		int undoPopCount = std::max(0, acEnd.mColumn - acStart.mColumn) + 1;

		const auto& acEntry = mACSuggestions[mACIndex];

		std::string entryText = mAutcompleteParse(acEntry.second, acStart);

		SetSelection(acStart, acEnd);
		Backspace();
		InsertText(entryText, true);

		undo.mAdded = entryText;
		undo.mAddedEnd = GetActualCursorCoordinates();

		if (mIsSnippet && mSnippetTagStart.size() > 0) {
			SetSelection(mSnippetTagStart[0], mSnippetTagEnd[0]);
			SetCursorPosition(mSnippetTagEnd[0]);
			mSnippetTagSelected = 0;
			mSnippetTagLength = 0;
			mSnippetTagPreviousLength = mSnippetTagEnd[mSnippetTagSelected].mColumn - mSnippetTagStart[mSnippetTagSelected].mColumn;
		}

		m_requestAutocomplete = false;
		mACOpened = false;

		undo.mAfter = mState;

		while (undoPopCount-- != 0) {
			mUndoIndex--;
			mUndoBuffer.pop_back();
		}
		AddUndo(undo);
	}

	void TextEditor::m_buildSuggestions(bool* keepACOpened)
	{
		mACWord = GetWordUnderCursor();

		bool isValid = false;
		for (int i = 0; i < mACWord.size(); i++)
		{
			if ((mACWord[i] >= 'a' && mACWord[i] <= 'z') || (mACWord[i] >= 'A' && mACWord[i] <= 'Z')) {
				isValid = true;
				break;
			}
		}

		// 修复中文补全
		if (!mACWord.empty() && IsValidUtf8StartByte(mACWord[0]))
		{
			isValid = true;
		}

		if (isValid) {
			mACSuggestions.clear();
			mACIndex = 0;
			mACSwitched = false;

			struct ACEntry {
				ACEntry(const std::string& str, const std::string& val, int loc)
				{
					DisplayString = str;
					Value = val;
					Location = loc;
				}

				std::string DisplayString;
				std::string Value;
				int Location;
			};
			std::vector<ACEntry> weights;

			std::string acWord = mACWord;
			std::transform(acWord.begin(), acWord.end(), acWord.begin(), tolower);

			// get the words
			for (int i = 0; i < mACEntrySearch.size(); i++) {
				std::string lwrStr = mACEntrySearch[i];
				std::transform(lwrStr.begin(), lwrStr.end(), lwrStr.begin(), tolower);

				size_t loc = lwrStr.find(acWord);
				if (loc != std::string::npos)
					weights.push_back(ACEntry(mACEntries[i].first, mACEntries[i].second, loc));
			}
			for (auto& func : mACFunctions) {
				std::string lwrStr = func.first;
				std::transform(lwrStr.begin(), lwrStr.end(), lwrStr.begin(), tolower);

				// suggest arguments and locals
				if (mState.mCursorPosition.mLine >= func.second.LineStart - 2 && mState.mCursorPosition.mLine <= func.second.LineEnd + 1) {
					// locals
					for (auto& str : func.second.Locals) {
						std::string lwrLoc = str;
						std::transform(lwrLoc.begin(), lwrLoc.end(), lwrLoc.begin(), tolower);

						size_t loc = lwrLoc.find(acWord);
						if (loc != std::string::npos)
							weights.push_back(ACEntry(str, str, loc));
					}

					// arguments
					for (auto& str : func.second.Arguments) {
						std::string lwrLoc = str;
						std::transform(lwrLoc.begin(), lwrLoc.end(), lwrLoc.begin(), tolower);

						size_t loc = lwrLoc.find(acWord);
						if (loc != std::string::npos)
							weights.push_back(ACEntry(str, str, loc));
					}
				}

				size_t loc = lwrStr.find(acWord);
				if (loc != std::string::npos) {
					std::string val = func.first;
					if (mCompleteBraces) val += "()";
					weights.push_back(ACEntry(func.first, val, loc));
				}
			}
			for (auto& str : mACUniforms) {
				std::string lwrStr = str;
				std::transform(lwrStr.begin(), lwrStr.end(), lwrStr.begin(), tolower);

				size_t loc = lwrStr.find(acWord);
				if (loc != std::string::npos)
					weights.push_back(ACEntry(str, str, loc));
			}
			for (auto& str : mACGlobals) {
				std::string lwrStr = str;
				std::transform(lwrStr.begin(), lwrStr.end(), lwrStr.begin(), tolower);

				size_t loc = lwrStr.find(acWord);
				if (loc != std::string::npos)
					weights.push_back(ACEntry(str, str, loc));
			}
			for (auto& str : mACUserTypes) {
				std::string lwrStr = str;
				std::transform(lwrStr.begin(), lwrStr.end(), lwrStr.begin(), tolower);

				size_t loc = lwrStr.find(acWord);
				if (loc != std::string::npos)
					weights.push_back(ACEntry(str, str, loc));
			}
			for (auto& str : mLanguageDefinition.mKeywords) {
				std::string lwrStr = str;
				std::transform(lwrStr.begin(), lwrStr.end(), lwrStr.begin(), tolower);

				size_t loc = lwrStr.find(acWord);
				if (loc != std::string::npos)
					weights.push_back(ACEntry(str, str, loc));
			}
			for (auto& str : mLanguageDefinition.mIdentifiers) {
				std::string lwrStr = str.first;
				std::transform(lwrStr.begin(), lwrStr.end(), lwrStr.begin(), tolower);

				size_t loc = lwrStr.find(acWord);
				if (loc != std::string::npos) {
					std::string val = str.first;
					if (mCompleteBraces) val += "()";
					weights.push_back(ACEntry(str.first, val, loc));
				}
			}

			// build the actual list
			for (const auto& entry : weights)
				if (entry.Location == 0)
					mACSuggestions.push_back(std::make_pair(entry.DisplayString, entry.Value));
			for (const auto& entry : weights)
				if (entry.Location != 0)
					mACSuggestions.push_back(std::make_pair(entry.DisplayString, entry.Value));

			if (mACSuggestions.size() > 0) {
				mACOpened = true;

				if (keepACOpened != nullptr)
					*keepACOpened = true;

				Coordinates curCursor = GetCursorPosition();
				curCursor.mColumn--;

				mACPosition = FindWordStart(curCursor);
			}
		}
	}

	ImVec2 TextEditor::CoordinatesToScreenPos(const Coordinates& aPosition) const
	{
		ImVec2 origin = mUICursorPos;
		int dist = aPosition.mColumn;

		int retY = origin.y + aPosition.mLine * mCharAdvance.y;
		int retX = origin.x + GetTextStart() * mCharAdvance.x + dist * mCharAdvance.x - ImGui::GetScrollX();

		return ImVec2(retX, retY);
	}

	void TextEditor::Render(const char* aTitle, const ImVec2& aSize, bool aBorder)
	{
		mWithinRender = true;
		mCursorPositionChanged = false;

		mFindOrigin = ImGui::GetCursorScreenPos();
		float windowWidth = mWindowWidth = ImGui::GetWindowWidth();

		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertU32ToFloat4(mPalette[(int)PaletteIndex::Background]));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
		if (!mIgnoreImGuiChild)
			ImGui::BeginChild(aTitle, aSize, aBorder, (ImGuiWindowFlags_AlwaysHorizontalScrollbar * mHorizontalScroll) | ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoNav);

		ImGui::SetWindowFontScale(mUIScale);

		if (mHandleKeyboardInputs)
		{
			HandleKeyboardInputs();
			//ImGui::PushAllowKeyboardFocus(true);
			//ImGui::SetKeyboardFocusHere();
		}

		if (mHandleMouseInputs)
			HandleMouseInputs();

		ColorizeInternal();
		m_readyForAutocomplete = true;
		RenderInternal(aTitle);

		// markers
		if (mScrollbarMarkers) {
			ImGuiWindow* window = ImGui::GetCurrentWindowRead();
			if (window->ScrollbarY) {
				ImDrawList* drawList = ImGui::GetWindowDrawList();
				ImRect scrollBarRect = ImGui::GetWindowScrollbarRect(window, ImGuiAxis_Y);
				ImGui::PushClipRect(scrollBarRect.Min, scrollBarRect.Max, false);
				int mSelectedLine = mState.mCursorPosition.mLine;
				if (mSelectedLine != 0) {
					float lineStartY = std::round(scrollBarRect.Min.y + (mSelectedLine - 0.5f) / mLines.size() * scrollBarRect.GetHeight());
					drawList->AddLine(ImVec2(scrollBarRect.Min.x, lineStartY), ImVec2(scrollBarRect.Max.x, lineStartY), (mPalette[(int)PaletteIndex::Default] & 0x00FFFFFFu) | 0x83000000u, 3);
				}

				for (auto& error : mErrorMarkers) {
					float lineStartY = std::round(scrollBarRect.Min.y + (float(error.first) - 0.5f) / mLines.size() * scrollBarRect.GetHeight());
					drawList->AddRectFilled(ImVec2(scrollBarRect.Min.x, lineStartY), ImVec2(scrollBarRect.Min.x + scrollBarRect.GetWidth() * 0.4f, lineStartY + 6.0f), mPalette[(int)PaletteIndex::ErrorMarker]);
				}
				ImGui::PopClipRect();
			}
		}

		if (ImGui::IsMouseClicked(1)) {
			mRightClickPos = ImGui::GetMousePos();

			if (ImGui::IsWindowHovered())
				SetCursorPosition(ScreenPosToCoordinates(mRightClickPos));
		}

		bool openBkptConditionWindow = false;
		if (ImGui::BeginPopupContextItem(("##edcontext" + std::string(aTitle)).c_str())) {
			if (mRightClickPos.x - mUICursorPos.x > ImGui::GetStyle().WindowPadding.x + DebugDataSpace) {
				if (ImGui::Selectable("Cut")) { Cut(); }
				if (ImGui::Selectable("Copy")) { Copy(); }
				if (ImGui::Selectable("Paste")) { Paste(); }
			}
			else {
				int line = ScreenPosToCoordinates(mRightClickPos).mLine + 1;

				if (IsDebugging() && ImGui::Selectable("Jump") && OnDebuggerJump) OnDebuggerJump(this, line);
				if (ImGui::Selectable("Breakpoint")) AddBreakpoint(line);
				if (HasBreakpoint(line)) {
					const auto& bkpt = GetBreakpoint(line);
					bool isEnabled = bkpt.mEnabled;
					if (ImGui::Selectable("Condition")) {
						mPopupCondition_Line = line;
						mPopupCondition_Use = !bkpt.mCondition.empty();
						memcpy(mPopupCondition_Condition, bkpt.mCondition.c_str(), bkpt.mCondition.size());
						mPopupCondition_Condition[std::min<size_t>(511, bkpt.mCondition.size())] = 0;
						openBkptConditionWindow = true;
					}
					if (ImGui::Selectable(isEnabled ? "Disable" : "Enable")) { SetBreakpointEnabled(line, !isEnabled); }
					if (ImGui::Selectable("Delete")) { RemoveBreakpoint(line); }
				}
			}
			ImGui::EndPopup();
		}

		/* FIND TEXT WINDOW */
		if (mFindOpened)
		{
			ImFont* font = ImGui::GetFont();
			//ImGui::PopFont();

			ImGui::SetNextWindowPos(ImVec2(mFindOrigin.x + windowWidth - mUICalculateSize(250), mFindOrigin.y + mUICalculateSize(50) * IsDebugging()), ImGuiCond_Always);
			ImGui::BeginChild(("##ted_findwnd" + std::string(aTitle)).c_str(), ImVec2(mUICalculateSize(220), mUICalculateSize(mReplaceOpened ? 90 : 40)), true, ImGuiWindowFlags_NoScrollbar);

			// check for findnext shortcut here...
			ShortcutID curActionID = ShortcutID::Count;
			ImGuiIO& io = ImGui::GetIO();
			auto shift = io.KeyShift;
			auto ctrl = io.ConfigMacOSXBehaviors ? io.KeySuper : io.KeyCtrl;
			auto alt = io.ConfigMacOSXBehaviors ? io.KeyCtrl : io.KeyAlt;
			for (int i = 0; i < m_shortcuts.size(); i++) {
				auto sct = m_shortcuts[i];

				if (sct.Key1 == -1)
					continue;
				SDL_Keymod mod;
				SDL_Scancode sc1 = SDL_GetScancodeFromKey(sct.Key1, &mod);

				if (ImGui::IsKeyPressed(SDLScancodeToImGuiKey(sc1)) && ((sct.Key2 != -1 && ImGui::IsKeyPressed(SDLScancodeToImGuiKey(SDL_GetScancodeFromKey(sct.Key2, &mod))) || sct.Key2 == -1))) {
					if (((sct.Ctrl == 0 && !ctrl) || (sct.Ctrl == 1 && ctrl) || (sct.Ctrl == 2)) &&		// ctrl check
						((sct.Alt == 0 && !alt) || (sct.Alt == 1 && alt) || (sct.Alt == 2)) &&			// alt check
						((sct.Shift == 0 && !shift) || (sct.Shift == 1 && shift) || (sct.Shift == 2))) {// shift check
						curActionID = (ShortcutID)i;
					}
				}
			}
			mFindNext = curActionID == ShortcutID::FindNext;

			if (mFindJustOpened) {
				std::string txt = GetSelectedText();
				if (txt.size() > 0)
					strcpy_s(mFindWord, txt.c_str());
			}

			ImGui::PushItemWidth(mUICalculateSize(-45));
			if (ImGui::InputText(("##ted_findtextbox" + std::string(aTitle)).c_str(), mFindWord, 256, ImGuiInputTextFlags_EnterReturnsTrue) || mFindNext) {
				auto curPos = mState.mCursorPosition;
				size_t cindex = 0;
				for (size_t ln = 0; ln < curPos.mLine; ln++)
					cindex += GetLineCharacterCount(ln) + 1;
				cindex += curPos.mColumn;

				std::string wordLower = mFindWord;
				std::transform(wordLower.begin(), wordLower.end(), wordLower.begin(), ::tolower);

				std::string textSrc = GetText();
				std::transform(textSrc.begin(), textSrc.end(), textSrc.begin(), ::tolower);

				size_t textLoc = textSrc.find(wordLower, cindex);
				if (textLoc == std::string::npos)
					textLoc = textSrc.find(wordLower, 0);

				if (textLoc != std::string::npos) {
					curPos.mLine = curPos.mColumn = 0;
					cindex = 0;
					for (size_t ln = 0; ln < mLines.size(); ln++) {
						int charCount = GetLineCharacterCount(ln) + 1;
						if (cindex + charCount > textLoc) {
							curPos.mLine = ln;
							curPos.mColumn = textLoc - cindex;

							auto& line = mLines[curPos.mLine];
							for (int i = 0; i < line.size(); i++)
								if (line[i].mChar == '\t')
									curPos.mColumn += (mTabSize - 1);

							break;
						}
						else {// just keep adding
							cindex += charCount;
						}
					}

					auto selStart = curPos, selEnd = curPos;
					selEnd.mColumn += strlen(mFindWord);
					SetSelection(curPos, selEnd);
					SetCursorPosition(selEnd);
					mScrollToCursor = true;

					if (!mFindNext)
						ImGui::SetKeyboardFocusHere(0);
				}

				mFindNext = false;
			}
			if (ImGui::IsItemActive())
				mFindFocused = true;
			else
				mFindFocused = false;
			if (mFindJustOpened) {
				ImGui::SetKeyboardFocusHere(0);
				mFindJustOpened = false;
			}
			ImGui::PopItemWidth();

			if (!mReadOnly) {
				ImGui::SameLine();
				if (ImGui::ArrowButton(("##expandFind" + std::string(aTitle)).c_str(), mReplaceOpened ? ImGuiDir_Up : ImGuiDir_Down))
					mReplaceOpened = !mReplaceOpened;
			}

			ImGui::SameLine();
			if (ImGui::Button(("X##" + std::string(aTitle)).c_str()))
				mFindOpened = false;

			if (mReplaceOpened && !mReadOnly) {
				ImGui::PushItemWidth(mUICalculateSize(-45));
				ImGui::NewLine();
				bool shouldReplace = false;
				if (ImGui::InputText(("##ted_replacetb" + std::string(aTitle)).c_str(), mReplaceWord, 256, ImGuiInputTextFlags_EnterReturnsTrue))
					shouldReplace = true;
				if (ImGui::IsItemActive())
					mReplaceFocused = true;
				else
					mReplaceFocused = false;
				ImGui::PopItemWidth();

				ImGui::SameLine();
				if (ImGui::Button((">##replaceOne" + std::string(aTitle)).c_str()) || shouldReplace) {
					if (strlen(mFindWord) > 0) {
						auto curPos = mState.mCursorPosition;

						std::string textSrc = GetText();
						if (mReplaceIndex >= textSrc.size())
							mReplaceIndex = 0;
						size_t textLoc = textSrc.find(mFindWord, mReplaceIndex);
						if (textLoc == std::string::npos) {
							mReplaceIndex = 0;
							textLoc = textSrc.find(mFindWord, 0);
						}

						if (textLoc != std::string::npos) {
							curPos.mLine = curPos.mColumn = 0;
							int totalCount = 0;
							for (size_t ln = 0; ln < mLines.size(); ln++) {
								int lineCharCount = GetLineCharacterCount(ln) + 1;
								if (textLoc >= totalCount && textLoc < totalCount + lineCharCount) {
									curPos.mLine = ln;
									curPos.mColumn = textLoc - totalCount;

									auto& line = mLines[curPos.mLine];
									for (int i = 0; i < line.size(); i++)
										if (line[i].mChar == '\t')
											curPos.mColumn += (mTabSize - 1);

									break;
								}
								totalCount += lineCharCount;
							}

							auto selStart = curPos, selEnd = curPos;
							selEnd.mColumn += strlen(mFindWord);
							SetSelection(curPos, selEnd);
							DeleteSelection();
							InsertText(mReplaceWord);
							SetCursorPosition(selEnd);
							mScrollToCursor = true;

							ImGui::SetKeyboardFocusHere(0);

							mReplaceIndex = textLoc + strlen(mReplaceWord);
						}
					}
				}

				ImGui::SameLine();
				if (ImGui::Button((">>##replaceAll" + std::string(aTitle)).c_str())) {
					if (strlen(mFindWord) > 0) {
						auto curPos = mState.mCursorPosition;

						std::string textSrc = GetText();
						size_t textLoc = textSrc.find(mFindWord, 0);

						do {
							if (textLoc != std::string::npos) {
								curPos.mLine = curPos.mColumn = 0;
								int totalCount = 0;
								for (size_t ln = 0; ln < mLines.size(); ln++) {
									int lineCharCount = GetLineCharacterCount(ln) + 1;
									if (textLoc >= totalCount && textLoc < totalCount + lineCharCount) {
										curPos.mLine = ln;
										curPos.mColumn = textLoc - totalCount;

										auto& line = mLines[curPos.mLine];
										for (int i = 0; i < line.size(); i++)
											if (line[i].mChar == '\t')
												curPos.mColumn += (mTabSize - 1);

										break;
									}
									totalCount += lineCharCount;
								}

								auto selStart = curPos, selEnd = curPos;
								selEnd.mColumn += strlen(mFindWord);
								SetSelection(curPos, selEnd);
								DeleteSelection();
								InsertText(mReplaceWord);
								SetCursorPosition(selEnd);
								mScrollToCursor = true;

								ImGui::SetKeyboardFocusHere(0);

								// find next occurance
								textSrc = GetText();
								textLoc += strlen(mReplaceWord);
								textLoc = textSrc.find(mFindWord, textLoc);
							}
						} while (textLoc != std::string::npos);
					}
				}
			}

			ImGui::EndChild();

			//ImGui::PushFont(font);

			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
				mFindOpened = false;
		}

		/* DEBUGGER CONTROLS */
		if (IsDebugging())
		{
			ImFont* font = ImGui::GetFont();
			//ImGui::PopFont();

			ImGui::SetNextWindowPos(ImVec2(mFindOrigin.x + windowWidth / 2 - mDebugBarWidth / 2, mFindOrigin.y), ImGuiCond_Always);
			ImGui::BeginChild(("##ted_dbgcontrols" + std::string(aTitle)).c_str(), ImVec2(mDebugBarWidth, mDebugBarHeight), true, ImGuiWindowFlags_NoScrollbar);

			ImVec2 dbBarStart = ImGui::GetCursorPos();

			if (ImGui::Button(("Step##ted_dbgstep" + std::string(aTitle)).c_str()) && OnDebuggerAction)
				OnDebuggerAction(this, DebugAction::Step);
			ImGui::SameLine(0, 6);

			if (ImGui::Button(("Step In##ted_dbgstepin" + std::string(aTitle)).c_str()) && OnDebuggerAction)
				OnDebuggerAction(this, DebugAction::StepInto);
			ImGui::SameLine(0, 6);

			if (ImGui::Button(("Step Out##ted_dbgstepout" + std::string(aTitle)).c_str()) && OnDebuggerAction)
				OnDebuggerAction(this, DebugAction::StepOut);
			ImGui::SameLine(0, 6);

			if (ImGui::Button(("Continue##ted_dbgcontinue" + std::string(aTitle)).c_str()) && OnDebuggerAction)
				OnDebuggerAction(this, DebugAction::Continue);
			ImGui::SameLine(0, 6);

			if (ImGui::Button(("Stop##ted_dbgstop" + std::string(aTitle)).c_str()) && OnDebuggerAction)
				OnDebuggerAction(this, DebugAction::Stop);

			ImVec2 dbBarEnd = ImGui::GetCursorPos();
			mDebugBarHeight = dbBarEnd.y - dbBarStart.y + ImGui::GetStyle().WindowPadding.y * 2.0f;

			ImGui::SameLine(0, 6);

			dbBarEnd = ImGui::GetCursorPos();
			mDebugBarWidth = dbBarEnd.x - dbBarStart.x + ImGui::GetStyle().WindowPadding.x * 2.0f;

			ImGui::EndChild();

			//ImGui::PushFont(font);
		}

		//if (mHandleKeyboardInputs)
		//	ImGui::PopAllowKeyboardFocus();

		if (!mIgnoreImGuiChild)
			ImGui::EndChild();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();

		// breakpoint condition popup
		if (openBkptConditionWindow)
			ImGui::OpenPopup("Condition##condition");

		ImFont* font = ImGui::GetFont();
		//ImGui::PopFont();
		ImGui::SetNextWindowSize(ImVec2(430, 175), ImGuiCond_Once);
		if (ImGui::BeginPopupModal("Condition##condition")) {
			if (ImGui::Checkbox("Use condition", &mPopupCondition_Use)) {
				if (!mPopupCondition_Use)
					mPopupCondition_Condition[0] = 0;
			}

			if (!mPopupCondition_Use) {
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}
			ImGui::InputText("Condition", mPopupCondition_Condition, 512);
			if (!mPopupCondition_Use) {
				ImGui::PopStyleVar();
				ImGui::PopItemFlag();
			}

			if (ImGui::Button("Cancel"))
				ImGui::CloseCurrentPopup();
			ImGui::SameLine();
			if (ImGui::Button("OK")) {
				size_t clen = strlen(mPopupCondition_Condition);
				bool isEmpty = true;
				for (size_t i = 0; i < clen; i++)
					if (!isspace(mPopupCondition_Condition[i])) {
						isEmpty = false;
						break;
					}
				Breakpoint& bkpt = GetBreakpoint(mPopupCondition_Line);
				bkpt.mCondition = (mPopupCondition_Use && !isEmpty) ? mPopupCondition_Condition : "";

				if (OnBreakpointUpdate)
					OnBreakpointUpdate(this, bkpt.mLine, bkpt.mCondition, bkpt.mEnabled);

				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		//ImGui::PushFont(font);

		mWithinRender = false;
	}

	void TextEditor::SetText(const std::string& aText)
	{
		mLines.clear();
		mLines.emplace_back(Line());
		for (auto chr : aText)
		{
			if (chr == '\r')
			{
				// ignore the carriage return character
			}
			else if (chr == '\n')
				mLines.emplace_back(Line());
			else
			{
				mLines.back().emplace_back(Glyph(chr, PaletteIndex::Default));
			}
		}

		mTextChanged = true;
		mScrollToTop = true;

		mUndoBuffer.clear();
		mUndoIndex = 0;

		Colorize();
	}

	void TextEditor::SetTextLines(const std::vector<std::string>& aLines)
	{
		mLines.clear();

		if (aLines.empty())
		{
			mLines.emplace_back(Line());
		}
		else
		{
			mLines.resize(aLines.size());

			for (size_t i = 0; i < aLines.size(); ++i)
			{
				const std::string& aLine = aLines[i];

				mLines[i].reserve(aLine.size());
				for (size_t j = 0; j < aLine.size(); ++j)
					mLines[i].emplace_back(Glyph(aLine[j], PaletteIndex::Default));
			}
		}

		mTextChanged = true;
		mScrollToTop = true;

		mUndoBuffer.clear();
		mUndoIndex = 0;

		Colorize();
	}

	void TextEditor::EnterCharacter(ImWchar aChar, bool aShift)
	{
		assert(!mReadOnly);

		UndoRecord u;

		u.mBefore = mState;

		// 选取模式的情况
		if (HasSelection())
		{
			// 选取多行按Tab键的情况，自动向每行开始插入制表符或者每行都向前缩进
			if (aChar == '\t' && mState.mSelectionStart.mLine != mState.mSelectionEnd.mLine)
			{
				auto start = mState.mSelectionStart;
				auto end = mState.mSelectionEnd;
				auto originalEnd = end;

				// 先确保start是小于end的
				if (start > end)
					std::swap(start, end);
				start.mColumn = 0;
				//			end.mColumn = end.mLine < mLines.size() ? mLines[end.mLine].size() : 0;
				if (end.mColumn == 0 && end.mLine > 0)
					--end.mLine;
				if (end.mLine >= (int)mLines.size())
					end.mLine = mLines.empty() ? 0 : (int)mLines.size() - 1;
				end.mColumn = GetLineMaxColumn(end.mLine);

				//if (end.mColumn >= GetLineMaxColumn(end.mLine))
				//	end.mColumn = GetLineMaxColumn(end.mLine) - 1;

				u.mRemovedStart = start;
				u.mRemovedEnd = end;
				u.mRemoved = GetText(start, end);

				bool modified = false;

				for (int i = start.mLine; i <= end.mLine; i++)
				{
					auto& line = mLines[i];
					// 按了Tab同时按了Shift,代表每行都缩进
					if (aShift)
					{
						if (!line.empty())
						{
							if (line.front().mChar == '\t')
							{
								line.erase(line.begin());
								modified = true;
							}
							else
							{
								for (int j = 0; j < mTabSize && !line.empty() && line.front().mChar == ' '; j++)
								{
									line.erase(line.begin());
									modified = true;
								}
							}
						}
					}
					// 向每行插入制表符或者空格
					else
					{
						if (mInsertSpaces) {
							for (int i = 0; i < mTabSize; i++)
								line.insert(line.begin(), Glyph(' ', PaletteIndex::Background));
						}
						else
							line.insert(line.begin(), Glyph('\t', PaletteIndex::Background));
						modified = true;
					}
				}

				// 添加撤销操作
				if (modified)
				{
					start = Coordinates(start.mLine, GetCharacterColumn(start.mLine, 0));
					Coordinates rangeEnd;
					if (originalEnd.mColumn != 0)
					{
						end = Coordinates(end.mLine, GetLineMaxColumn(end.mLine));
						rangeEnd = end;
						u.mAdded = GetText(start, end);
					}
					else
					{
						end = Coordinates(originalEnd.mLine, 0);
						rangeEnd = Coordinates(end.mLine - 1, GetLineMaxColumn(end.mLine - 1));
						u.mAdded = GetText(start, rangeEnd);
					}

					u.mAddedStart = start;
					u.mAddedEnd = rangeEnd;
					u.mAfter = mState;

					mState.mSelectionStart = start;
					mState.mSelectionEnd = end;
					AddUndo(u);

					mTextChanged = true;
					if (OnContentUpdate != nullptr)
						OnContentUpdate(this);

					EnsureCursorVisible();
				}

				return;
			}
			else
			{
				// 如果不是制表符就删除这一行
				u.mRemoved = GetSelectedText();
				u.mRemovedStart = mState.mSelectionStart;
				u.mRemovedEnd = mState.mSelectionEnd;
				DeleteSelection();
			}
		}

		auto coord = GetActualCursorCoordinates();
		u.mAddedStart = coord;

		if (mLines.empty())
			mLines.push_back(Line());

		// 换行
		if (aChar == '\n')
		{
			InsertLine(coord.mLine + 1);
			auto& line = mLines[coord.mLine];
			auto& newLine = mLines[coord.mLine + 1];

			if (mLanguageDefinition.mAutoIndentation && mSmartIndent)
				for (size_t it = 0; it < line.size() && isascii(line[it].mChar) && isblank(line[it].mChar); ++it)
					newLine.push_back(line[it]);

			const size_t whitespaceSize = newLine.size();
			auto cindex = GetCharacterIndex(coord);
			newLine.insert(newLine.end(), line.begin() + cindex, line.end());
			line.erase(line.begin() + cindex, line.begin() + line.size());
			SetCursorPosition(Coordinates(coord.mLine + 1, GetCharacterColumn(coord.mLine + 1, (int)whitespaceSize)));
			u.mAdded = (char)aChar;
		}
		// 正常字符情况
		else
		{
			char buf[7];
			int e = ImTextCharToUtf8(buf, 7, aChar);
			if (e > 0)
			{
				if (mInsertSpaces && e == 1 && buf[0] == '\t') {
					for (int i = 0; i < mTabSize; i++)
						buf[i] = ' ';
					e = mTabSize;
				}
				buf[e] = '\0';

				auto& line = mLines[coord.mLine];
				auto cindex = GetCharacterIndex(coord);

				// 覆盖
				if (mOverwrite && cindex < (int)line.size())
				{
					auto d = UTF8CharLength(line[cindex].mChar);

					u.mRemovedStart = mState.mCursorPosition;
					u.mRemovedEnd = Coordinates(coord.mLine, GetCharacterColumn(coord.mLine, cindex + d));

					while (d-- > 0 && cindex < (int)line.size())
					{
						u.mRemoved += line[cindex].mChar;
						line.erase(line.begin() + cindex);
					}
				}

				// 插入字符到行
				for (auto p = buf; *p != '\0'; p++, ++cindex)
					line.insert(line.begin() + cindex, Glyph(*p, PaletteIndex::Default));
				u.mAdded = buf;

				SetCursorPosition(Coordinates(coord.mLine, GetCharacterColumn(coord.mLine, cindex)));
			}
			else
				return;
		}

		// 激活自动补全建议 active suggestions
		// 在此处修复中文补全
		char out_utf8_buf[5] = {};
		ImTextCharToUtf8(out_utf8_buf, 5, aChar);
		if (mActiveAutocomplete) {
			// 正常英文
			if (aChar <= 127 && (isalpha(aChar) || aChar == '_'))
			{
				m_requestAutocomplete = true;
				m_readyForAutocomplete = false;
			}
			// utf8
			else if (IsValidUtf8StartByte(out_utf8_buf[0]))
			{
				m_requestAutocomplete = true;
				m_readyForAutocomplete = false;
			}
		}

		mTextChanged = true;
		if (OnContentUpdate != nullptr)
			OnContentUpdate(this);

		u.mAddedEnd = GetActualCursorCoordinates();
		u.mAfter = mState;

		AddUndo(u);

		Colorize(coord.mLine - 1, 3);
		EnsureCursorVisible();

		// 自动括号补全 auto brace completion
		if (mCompleteBraces) {
			if (aChar == '{') {
				EnterCharacter('\n', false);
				EnterCharacter('}', false);
			}
			else if (aChar == '(')
				EnterCharacter(')', false);
			else if (aChar == '[')
				EnterCharacter(']', false);

			if (aChar == '{' || aChar == '(' || aChar == '[')
				mState.mCursorPosition.mColumn--;
		}

	}

	void TextEditor::SetReadOnly(bool aValue)
	{
		mReadOnly = aValue;
	}

	void TextEditor::SetColorizerEnable(bool aValue)
	{
		mColorizerEnabled = aValue;
	}

	void TextEditor::SetCursorPosition(const Coordinates& aPosition)
	{
		if (mState.mCursorPosition != aPosition)
		{
			mState.mCursorPosition = aPosition;
			mCursorPositionChanged = true;
			EnsureCursorVisible();
		}
	}

	void TextEditor::SetSelectionStart(const Coordinates& aPosition)
	{
		mState.mSelectionStart = SanitizeCoordinates(aPosition);
		if (mState.mSelectionStart > mState.mSelectionEnd)
			std::swap(mState.mSelectionStart, mState.mSelectionEnd);
	}

	void TextEditor::SetSelectionEnd(const Coordinates& aPosition)
	{
		mState.mSelectionEnd = SanitizeCoordinates(aPosition);
		if (mState.mSelectionStart > mState.mSelectionEnd)
			std::swap(mState.mSelectionStart, mState.mSelectionEnd);
	}

	void TextEditor::SetSelection(const Coordinates& aStart, const Coordinates& aEnd, SelectionMode aMode)
	{
		auto oldSelStart = mState.mSelectionStart;
		auto oldSelEnd = mState.mSelectionEnd;

		mState.mSelectionStart = SanitizeCoordinates(aStart);
		mState.mSelectionEnd = SanitizeCoordinates(aEnd);
		if (mState.mSelectionStart > mState.mSelectionEnd)
			std::swap(mState.mSelectionStart, mState.mSelectionEnd);

		switch (aMode)
		{
		case SelectionMode::Normal:
			break;
		case SelectionMode::Word:
		{
			mState.mSelectionStart = FindWordStart(mState.mSelectionStart);
			if (!IsOnWordBoundary(mState.mSelectionEnd))
				mState.mSelectionEnd = FindWordEnd(FindWordStart(mState.mSelectionEnd));
			break;
		}
		case SelectionMode::Line:
		{
			const auto lineNo = mState.mSelectionEnd.mLine;
			const auto lineSize = (size_t)lineNo < mLines.size() ? mLines[lineNo].size() : 0;
			mState.mSelectionStart = Coordinates(mState.mSelectionStart.mLine, 0);
			mState.mSelectionEnd = Coordinates(lineNo, GetLineMaxColumn(lineNo));
			break;
		}
		default:
			break;
		}

		if (mState.mSelectionStart != oldSelStart ||
			mState.mSelectionEnd != oldSelEnd)
			mCursorPositionChanged = true;

		// update mReplaceIndex
		mReplaceIndex = 0;
		for (size_t ln = 0; ln < mState.mCursorPosition.mLine; ln++)
			mReplaceIndex += GetLineCharacterCount(ln) + 1;
		mReplaceIndex += mState.mCursorPosition.mColumn;
	}

	void TextEditor::InsertText(const std::string& aValue, bool indent)
	{
		InsertText(aValue.c_str(), indent);
	}

	void TextEditor::InsertText(const char* aValue, bool indent)
	{
		if (aValue == nullptr)
			return;

		auto pos = GetActualCursorCoordinates();
		auto start = std::min<Coordinates>(pos, mState.mSelectionStart);
		int totalLines = pos.mLine - start.mLine;

		totalLines += InsertTextAt(pos, aValue, indent);

		SetSelection(pos, pos);
		SetCursorPosition(pos);
		Colorize(start.mLine - 1, totalLines + 2);
	}

	void TextEditor::DeleteSelection()
	{
		assert(mState.mSelectionEnd >= mState.mSelectionStart);

		if (mState.mSelectionEnd == mState.mSelectionStart)
			return;

		DeleteRange(mState.mSelectionStart, mState.mSelectionEnd);

		SetSelection(mState.mSelectionStart, mState.mSelectionStart);
		SetCursorPosition(mState.mSelectionStart);
		Colorize(mState.mSelectionStart.mLine, 1);
	}

	void TextEditor::MoveUp(int aAmount, bool aSelect)
	{
		auto oldPos = mState.mCursorPosition;
		mState.mCursorPosition.mLine = std::max<int>(0, mState.mCursorPosition.mLine - aAmount);
		if (oldPos != mState.mCursorPosition)
		{
			if (aSelect)
			{
				if (oldPos == mInteractiveStart)
					mInteractiveStart = mState.mCursorPosition;
				else if (oldPos == mInteractiveEnd)
					mInteractiveEnd = mState.mCursorPosition;
				else
				{
					mInteractiveStart = mState.mCursorPosition;
					mInteractiveEnd = oldPos;
				}
			}
			else
				mInteractiveStart = mInteractiveEnd = mState.mCursorPosition;
			SetSelection(mInteractiveStart, mInteractiveEnd);

			EnsureCursorVisible();
		}
	}

	void TextEditor::MoveDown(int aAmount, bool aSelect)
	{
		assert(mState.mCursorPosition.mColumn >= 0);
		auto oldPos = mState.mCursorPosition;
		mState.mCursorPosition.mLine = std::max<int>(0, std::min<int>((int)mLines.size() - 1, mState.mCursorPosition.mLine + aAmount));

		if (mState.mCursorPosition != oldPos)
		{
			if (aSelect)
			{
				if (oldPos == mInteractiveEnd)
					mInteractiveEnd = mState.mCursorPosition;
				else if (oldPos == mInteractiveStart)
					mInteractiveStart = mState.mCursorPosition;
				else
				{
					mInteractiveStart = oldPos;
					mInteractiveEnd = mState.mCursorPosition;
				}
			}
			else
				mInteractiveStart = mInteractiveEnd = mState.mCursorPosition;
			SetSelection(mInteractiveStart, mInteractiveEnd);

			EnsureCursorVisible();
		}
	}

	void TextEditor::MoveLeft(int aAmount, bool aSelect, bool aWordMode)
	{
		if (mLines.empty())
			return;

		auto oldPos = mState.mCursorPosition;
		mState.mCursorPosition = GetActualCursorCoordinates();
		auto line = mState.mCursorPosition.mLine;
		auto cindex = GetCharacterIndex(mState.mCursorPosition);

		while (aAmount-- > 0)
		{
			if (cindex == 0)
			{
				if (line > 0)
				{
					--line;
					if ((int)mLines.size() > line)
						cindex = (int)mLines[line].size();
					else
						cindex = 0;
				}
			}
			else
			{
				--cindex;
				if (cindex > 0)
				{
					if ((int)mLines.size() > line)
					{
						while (cindex > 0 && IsUTFSequence(mLines[line][cindex].mChar))
							--cindex;
					}
				}
			}

			mState.mCursorPosition = Coordinates(line, GetCharacterColumn(line, cindex));
			if (aWordMode)
			{
				mState.mCursorPosition = FindWordStart(mState.mCursorPosition);
				cindex = GetCharacterIndex(mState.mCursorPosition);
			}
		}

		mState.mCursorPosition = Coordinates(line, GetCharacterColumn(line, cindex));

		assert(mState.mCursorPosition.mColumn >= 0);
		if (aSelect)
		{
			mInteractiveStart = mState.mSelectionStart;
			mInteractiveEnd = mState.mSelectionEnd;

			if (oldPos == mInteractiveStart)
				mInteractiveStart = mState.mCursorPosition;
			else if (oldPos == mInteractiveEnd)
				mInteractiveEnd = mState.mCursorPosition;
			else
			{
				mInteractiveStart = mState.mCursorPosition;
				mInteractiveEnd = oldPos;
			}
		}
		else
			mInteractiveStart = mInteractiveEnd = mState.mCursorPosition;
		SetSelection(mInteractiveStart, mInteractiveEnd, SelectionMode::Normal);

		EnsureCursorVisible();
	}

	void TextEditor::MoveRight(int aAmount, bool aSelect, bool aWordMode)
	{
		auto oldPos = mState.mCursorPosition;

		if (mLines.empty() || oldPos.mLine >= mLines.size())
			return;

		mState.mCursorPosition = GetActualCursorCoordinates();
		auto line = mState.mCursorPosition.mLine;
		auto cindex = GetCharacterIndex(mState.mCursorPosition);

		while (aAmount-- > 0)
		{
			auto lindex = mState.mCursorPosition.mLine;
			auto& line = mLines[lindex];

			if (cindex >= line.size())
			{
				if (mState.mCursorPosition.mLine < mLines.size() - 1)
				{
					mState.mCursorPosition.mLine = std::max(0, std::min((int)mLines.size() - 1, mState.mCursorPosition.mLine + 1));
					mState.mCursorPosition.mColumn = 0;
				}
				else
					return;
			}
			else
			{
				cindex += UTF8CharLength(line[cindex].mChar);
				mState.mCursorPosition = Coordinates(lindex, GetCharacterColumn(lindex, cindex));
				if (aWordMode)
					mState.mCursorPosition = FindWordEnd(mState.mCursorPosition);
			}
		}

		if (aSelect)
		{
			mInteractiveStart = mState.mSelectionStart;
			mInteractiveEnd = mState.mSelectionEnd;

			if (oldPos == mInteractiveEnd)
				mInteractiveEnd = mState.mCursorPosition;
			else if (oldPos == mInteractiveStart)
				mInteractiveStart = mState.mCursorPosition;
			else
			{
				mInteractiveStart = oldPos;
				mInteractiveEnd = mState.mCursorPosition;
			}
		}
		else
			mInteractiveStart = mInteractiveEnd = mState.mCursorPosition;
		SetSelection(mInteractiveStart, mInteractiveEnd, SelectionMode::Normal);

		EnsureCursorVisible();
	}

	void TextEditor::MoveTop(bool aSelect)
	{
		auto oldPos = mState.mCursorPosition;
		SetCursorPosition(Coordinates(0, 0));

		if (mState.mCursorPosition != oldPos)
		{
			if (aSelect)
			{
				mInteractiveEnd = oldPos;
				mInteractiveStart = mState.mCursorPosition;
			}
			else
				mInteractiveStart = mInteractiveEnd = mState.mCursorPosition;
			SetSelection(mInteractiveStart, mInteractiveEnd);
		}
	}

	void TextEditor::MoveBottom(bool aSelect)
	{
		auto oldPos = GetCursorPosition();
		auto newPos = Coordinates((int)mLines.size() - 1, 0);
		SetCursorPosition(newPos);
		if (aSelect)
		{
			mInteractiveStart = oldPos;
			mInteractiveEnd = newPos;
		}
		else
			mInteractiveStart = mInteractiveEnd = newPos;
		SetSelection(mInteractiveStart, mInteractiveEnd);
	}

	void TextEditor::MoveHome(bool aSelect)
	{
		auto oldPos = mState.mCursorPosition;
		SetCursorPosition(Coordinates(mState.mCursorPosition.mLine, 0));

		if (mState.mCursorPosition != oldPos)
		{
			if (aSelect)
			{
				if (oldPos == mInteractiveStart)
					mInteractiveStart = mState.mCursorPosition;
				else if (oldPos == mInteractiveEnd)
					mInteractiveEnd = mState.mCursorPosition;
				else
				{
					mInteractiveStart = mState.mCursorPosition;
					mInteractiveEnd = oldPos;
				}
			}
			else
				mInteractiveStart = mInteractiveEnd = mState.mCursorPosition;
			SetSelection(mInteractiveStart, mInteractiveEnd);
		}
	}

	void TextEditor::MoveEnd(bool aSelect)
	{
		auto oldPos = mState.mCursorPosition;
		SetCursorPosition(Coordinates(mState.mCursorPosition.mLine, GetLineMaxColumn(oldPos.mLine)));

		if (mState.mCursorPosition != oldPos)
		{
			if (aSelect)
			{
				if (oldPos == mInteractiveEnd)
					mInteractiveEnd = mState.mCursorPosition;
				else if (oldPos == mInteractiveStart)
					mInteractiveStart = mState.mCursorPosition;
				else
				{
					mInteractiveStart = oldPos;
					mInteractiveEnd = mState.mCursorPosition;
				}
			}
			else
				mInteractiveStart = mInteractiveEnd = mState.mCursorPosition;
			SetSelection(mInteractiveStart, mInteractiveEnd);
		}
	}

	void TextEditor::Delete()
	{
		assert(!mReadOnly);

		if (mLines.empty())
			return;

		UndoRecord u;
		u.mBefore = mState;

		if (HasSelection())
		{
			u.mRemoved = GetSelectedText();
			u.mRemovedStart = mState.mSelectionStart;
			u.mRemovedEnd = mState.mSelectionEnd;

			DeleteSelection();
		}
		else
		{
			auto pos = GetActualCursorCoordinates();
			SetCursorPosition(pos);
			auto& line = mLines[pos.mLine];

			if (pos.mColumn == GetLineMaxColumn(pos.mLine))
			{
				if (pos.mLine == (int)mLines.size() - 1)
					return;

				u.mRemoved = '\n';
				u.mRemovedStart = u.mRemovedEnd = GetActualCursorCoordinates();
				Advance(u.mRemovedEnd);

				auto& nextLine = mLines[pos.mLine + 1];
				line.insert(line.end(), nextLine.begin(), nextLine.end());
				RemoveLine(pos.mLine + 1);
			}
			else
			{
				auto cindex = GetCharacterIndex(pos);
				u.mRemovedStart = u.mRemovedEnd = GetActualCursorCoordinates();
				u.mRemovedEnd.mColumn++;
				u.mRemoved = GetText(u.mRemovedStart, u.mRemovedEnd);

				auto d = UTF8CharLength(line[cindex].mChar);
				while (d-- > 0 && cindex < (int)line.size())
					line.erase(line.begin() + cindex);
			}

			mTextChanged = true;
			if (OnContentUpdate != nullptr)
				OnContentUpdate(this);

			Colorize(pos.mLine, 1);
		}

		u.mAfter = mState;
		AddUndo(u);
	}

	void TextEditor::Backspace()
	{
		assert(!mReadOnly);

		if (mLines.empty())
			return;

		UndoRecord u;
		u.mBefore = mState;

		if (HasSelection())
		{
			u.mRemoved = GetSelectedText();
			u.mRemovedStart = mState.mSelectionStart;
			u.mRemovedEnd = mState.mSelectionEnd;

			DeleteSelection();
		}
		else
		{
			auto pos = GetActualCursorCoordinates();
			SetCursorPosition(pos);

			if (mState.mCursorPosition.mColumn == 0)
			{
				if (mState.mCursorPosition.mLine == 0)
					return;

				u.mRemoved = '\n';
				u.mRemovedStart = u.mRemovedEnd = Coordinates(pos.mLine - 1, GetLineMaxColumn(pos.mLine - 1));
				Advance(u.mRemovedEnd);

				auto& line = mLines[mState.mCursorPosition.mLine];
				auto& prevLine = mLines[mState.mCursorPosition.mLine - 1];
				auto prevSize = GetLineMaxColumn(mState.mCursorPosition.mLine - 1);
				prevLine.insert(prevLine.end(), line.begin(), line.end());

				ErrorMarkers etmp;
				for (auto& i : mErrorMarkers)
					etmp.insert(ErrorMarkers::value_type(i.first - 1 == mState.mCursorPosition.mLine ? i.first - 1 : i.first, i.second));
				mErrorMarkers = std::move(etmp);

				RemoveLine(mState.mCursorPosition.mLine);
				--mState.mCursorPosition.mLine;
				mState.mCursorPosition.mColumn = prevSize;
			}
			else
			{
				auto& line = mLines[mState.mCursorPosition.mLine];
				auto cindex = GetCharacterIndex(pos) - 1;
				auto cend = cindex + 1;
				while (cindex > 0 && IsUTFSequence(line[cindex].mChar))
					--cindex;

				//if (cindex > 0 && UTF8CharLength(line[cindex].mChar) > 1)
				//	--cindex;

				int actualLoc = pos.mColumn;
				for (int i = 0; i < line.size(); i++) {
					if (line[i].mChar == '\t')
						actualLoc -= GetTabSize() - 1;
				}

				if (mCompleteBraces && actualLoc > 0 && actualLoc < line.size()) {
					if ((line[actualLoc - 1].mChar == '(' && line[actualLoc].mChar == ')') ||
						(line[actualLoc - 1].mChar == '{' && line[actualLoc].mChar == '}') ||
						(line[actualLoc - 1].mChar == '[' && line[actualLoc].mChar == ']'))
						Delete();
				}

				// 修复前的代码，中文退格有崩溃bug
				//u.mRemovedStart = u.mRemovedEnd = GetActualCursorCoordinates();
				//
				//while (cindex < line.size() && cend-- > cindex)
				//{
				//	uint8_t chVal = line[cindex].mChar;
				//
				//	u.mRemoved += line[cindex].mChar;
				//	line.erase(line.begin() + cindex);
				//
				//	u.mRemovedStart.mColumn -= (chVal == '\t') ? mTabSize : 1;
				//	mState.mCursorPosition.mColumn -= (chVal == '\t') ? mTabSize : 1;
				//}

				// 修复后的代码,添加了边界检查，但还是
				//u.mRemovedStart = u.mRemovedEnd = GetActualCursorCoordinates();
				//
				//while (cindex < line.size() && cend-- > cindex)
				//{
				//	uint8_t chVal = line[cindex].mChar;
				//
				//	u.mRemoved += line[cindex].mChar;
				//	line.erase(line.begin() + cindex);
				//
				//	u.mRemovedStart.mColumn -= (chVal == '\t') ? mTabSize : 1;
				//	mState.mCursorPosition.mColumn -= (chVal == '\t') ? mTabSize : 1;
				//	
				//	u.mRemovedStart.mColumn = std::max(0, u.mRemovedStart.mColumn);
				//	mState.mCursorPosition.mColumn = std::max(0, mState.mCursorPosition.mColumn);
				//}

				// 原版代码
				{
					u.mRemovedStart = u.mRemovedEnd = GetActualCursorCoordinates();
					--u.mRemovedStart.mColumn;
					--mState.mCursorPosition.mColumn;

					while (cindex < line.size() && cend-- > cindex)
					{
						u.mRemoved += line[cindex].mChar;
						line.erase(line.begin() + cindex);
					}
				}
			}

			mTextChanged = true;
			if (OnContentUpdate != nullptr)
				OnContentUpdate(this);

			EnsureCursorVisible();
			Colorize(mState.mCursorPosition.mLine, 1);
		}

		u.mAfter = mState;
		AddUndo(u);

		// autocomplete
		if (mActiveAutocomplete && mACOpened) {
			m_requestAutocomplete = true;
			m_readyForAutocomplete = false;
		}
	}

	void TextEditor::SelectWordUnderCursor()
	{
		auto c = GetCursorPosition();
		SetSelection(FindWordStart(c), FindWordEnd(c));
	}

	void TextEditor::SelectAll()
	{
		SetSelection(Coordinates(0, 0), Coordinates((int)mLines.size(), 0));
	}

	bool TextEditor::HasSelection() const
	{
		return mState.mSelectionEnd > mState.mSelectionStart;
	}

	void TextEditor::SetShortcut(ShortcutID id, Shortcut s) {
		m_shortcuts[(int)id].Key1 = s.Key1;
		m_shortcuts[(int)id].Key2 = s.Key2;
		if (m_shortcuts[(int)id].Ctrl != 2)
			m_shortcuts[(int)id].Ctrl = s.Ctrl;
		if (m_shortcuts[(int)id].Shift != 2)
			m_shortcuts[(int)id].Shift = s.Shift;
		if (m_shortcuts[(int)id].Alt != 2)
			m_shortcuts[(int)id].Alt = s.Alt;
	}

	void TextEditor::Copy()
	{
		if (HasSelection())
		{
			ImGui::SetClipboardText(GetSelectedText().c_str());
		}
		else
		{
			if (!mLines.empty())
			{
				std::string str;
				auto& line = mLines[GetActualCursorCoordinates().mLine];
				for (auto& g : line)
					str.push_back(g.mChar);
				ImGui::SetClipboardText(str.c_str());
			}
		}
	}

	void TextEditor::Cut()
	{
		if (IsReadOnly())
		{
			Copy();
		}
		else
		{
			if (HasSelection())
			{
				UndoRecord u;
				u.mBefore = mState;
				u.mRemoved = GetSelectedText();
				u.mRemovedStart = mState.mSelectionStart;
				u.mRemovedEnd = mState.mSelectionEnd;

				Copy();
				DeleteSelection();

				u.mAfter = mState;
				AddUndo(u);
			}
		}
	}

	void TextEditor::Paste()
	{
		if (IsReadOnly())
			return;

		auto clipText = ImGui::GetClipboardText();
		if (clipText != nullptr && strlen(clipText) > 0)
		{
			UndoRecord u;
			u.mBefore = mState;

			if (HasSelection())
			{
				u.mRemoved = GetSelectedText();
				u.mRemovedStart = mState.mSelectionStart;
				u.mRemovedEnd = mState.mSelectionEnd;
				DeleteSelection();
			}

			u.mAdded = clipText;
			u.mAddedStart = GetActualCursorCoordinates();

			InsertText(clipText, mAutoindentOnPaste);

			u.mAddedEnd = GetActualCursorCoordinates();
			u.mAfter = mState;
			AddUndo(u);
		}
	}

	bool TextEditor::CanUndo()
	{
		return !IsReadOnly() && mUndoIndex > 0;
	}

	bool TextEditor::CanRedo()
	{
		return !IsReadOnly() && mUndoIndex < (int)mUndoBuffer.size();
	}

	void TextEditor::Undo(int aSteps)
	{
		while (CanUndo() && aSteps-- > 0)
			mUndoBuffer[--mUndoIndex].Undo(this);
	}

	void TextEditor::Redo(int aSteps)
	{
		while (CanRedo() && aSteps-- > 0)
			mUndoBuffer[mUndoIndex++].Redo(this);
	}

	const Palette& TextEditor::GetDarkPalette()
	{
		const static Palette p = { {
			0xff7f7f7f,	// Default
			0xffd69c56,	// Keyword
			0xff00ff00,	// Number
			0xff7070e0,	// String
			0xff70a0e0, // Char literal
			0xffffffff, // Punctuation
			0xff408080,	// Preprocessor
			0xffaaaaaa, // Identifier
			0xff9bc64d, // Known identifier
			0xffc040a0, // Preproc identifier
			0xff206020, // Comment (single line)
			0xff406020, // Comment (multi line)
			0xff101010, // Background
			0xffe0e0e0, // Cursor
			0x80a06020, // Selection
			0x800020ff, // ErrorMarker
			0xff0000ff, // Breakpoint
			0xffffffff, // Breakpoint outline
			0xFF1DD8FF, // Current line indicator
			0xFF696969, // Current line indicator outline
			0xff707000, // Line number
			0x40000000, // Current line fill
			0x40808080, // Current line fill (inactive)
			0x40a0a0a0, // Current line edge
			0xff33ffff, // Error message
			0xffffffff, // BreakpointDisabled
			0xffaaaaaa, // UserFunction
			0xffb0c94e, // UserType
			0xffaaaaaa, // UniformType
			0xffaaaaaa, // GlobalVariable
			0xffaaaaaa, // LocalVariable
			0xff888888	// FunctionArgument
		} };
		return p;
	}

	const Palette& TextEditor::GetLightPalette()
	{
		const static Palette p = { {
			0xff7f7f7f,	// None
			0xffff0c06,	// Keyword
			0xff008000,	// Number
			0xff2020a0,	// String
			0xff304070, // Char literal
			0xff000000, // Punctuation
			0xff406060,	// Preprocessor
			0xff404040, // Identifier
			0xff606010, // Known identifier
			0xffc040a0, // Preproc identifier
			0xff205020, // Comment (single line)
			0xff405020, // Comment (multi line)
			0xffffffff, // Background
			0xff000000, // Cursor
			0x80DFBF80, // Selection
			0xa00010ff, // ErrorMarker
			0xff0000ff, // Breakpoint
			0xff000000, // Breakpoint outline
			0xFF1DD8FF, // Current line indicator
			0xFF696969, // Current line indicator outline
			0xff505000, // Line number
			0x20000000, // Current line fill
			0x20808080, // Current line fill (inactive)
			0x30000000, // Current line edge
			0xff3333ff, // Error message
			0xffffffff, // BreakpointDisabled
			0xff404040, // UserFunction
			0xffb0912b, // UserType
			0xff404040, // UniformType
			0xff404040, // GlobalVariable
			0xff404040, // LocalVariable
			0xff606060	// FunctionArgument
		} };
		return p;
	}

	const Palette& TextEditor::GetRetroBluePalette()
	{
		const static Palette p = { {
			0xff00ffff,	// None
			0xffffff00,	// Keyword
			0xff00ff00,	// Number
			0xff808000,	// String
			0xff808000, // Char literal
			0xffffffff, // Punctuation
			0xff008000,	// Preprocessor
			0xff00ffff, // Identifier
			0xffffffff, // Known identifier
			0xffff00ff, // Preproc identifier
			0xff808080, // Comment (single line)
			0xff404040, // Comment (multi line)
			0xff800000, // Background
			0xff0080ff, // Cursor
			0x80ffff00, // Selection
			0xa00000ff, // ErrorMarker
			0xff0000ff, // Breakpoint
			0xffffffff, // Breakpoint outline
			0xFF1DD8FF, // Current line indicator
			0xff808000, // Line number
			0x40000000, // Current line fill
			0x40808080, // Current line fill (inactive)
			0x40000000, // Current line edge
			0xffffff00, // Error message
			0xffffffff, // BreakpointDisabled
			0xff00ffff, // UserFunction
			0xff00ffff, // UserType
			0xff00ffff, // UniformType
			0xff00ffff, // GlobalVariable
			0xff00ffff, // LocalVariable
			0xff00ffff	// FunctionArgument
		} };
		return p;
	}

	std::string TextEditor::GetText() const
	{
		return GetText(Coordinates(), Coordinates((int)mLines.size(), 0));
	}

	void TextEditor::GetTextLines(std::vector<std::string>& result) const
	{
		result.reserve(mLines.size());

		for (auto& line : mLines)
		{
			std::string text;

			text.resize(line.size());

			for (size_t i = 0; i < line.size(); ++i)
				text[i] = line[i].mChar;

			result.emplace_back(std::move(text));
		}
	}

	std::string TextEditor::GetSelectedText() const
	{
		return GetText(mState.mSelectionStart, mState.mSelectionEnd);
	}

	std::string TextEditor::GetCurrentLineText()const
	{
		auto lineLength = GetLineMaxColumn(mState.mCursorPosition.mLine);
		return GetText(
			Coordinates(mState.mCursorPosition.mLine, 0),
			Coordinates(mState.mCursorPosition.mLine, lineLength));
	}

	void TextEditor::ProcessInputs()
	{
	}

	void TextEditor::Colorize(int aFromLine, int aLines)
	{
		int toLine = aLines == -1 ? (int)mLines.size() : std::min<int>((int)mLines.size(), aFromLine + aLines);
		mColorRangeMin = std::min<int>(mColorRangeMin, aFromLine);
		mColorRangeMax = std::max<int>(mColorRangeMax, toLine);
		mColorRangeMin = std::max<int>(0, mColorRangeMin);
		mColorRangeMax = std::max<int>(mColorRangeMin, mColorRangeMax);
		mCheckComments = true;
	}

	void TextEditor::ColorizeRange(int aFromLine, int aToLine)
	{
		if (mLines.empty() || !mColorizerEnabled)
			return;

		std::string buffer;
		std::cmatch results;
		std::string id;

		// 对每行进行着色
		int endLine = std::max(0, std::min((int)mLines.size(), aToLine));
		for (int i = aFromLine; i < endLine; ++i)
		{
			auto& line = mLines[i];

			if (line.empty())
				continue;

			buffer.resize(line.size());
			for (size_t j = 0; j < line.size(); ++j)
			{
				auto& col = line[j];
				buffer[j] = col.mChar;
				col.mColorIndex = PaletteIndex::Default;
			}

			const char* bufferBegin = &buffer.front();
			const char* bufferEnd = bufferBegin + buffer.size();

			auto last = bufferEnd;

			for (auto first = bufferBegin; first != last; )
			{
				const char* token_begin = nullptr;
				const char* token_end = nullptr;
				PaletteIndex token_color = PaletteIndex::Default;

				bool hasTokenizeResult = false;

				// 调用自定义着色识别函数
				if (mLanguageDefinition.mTokenize != nullptr)
				{
					if (mLanguageDefinition.mTokenize(first, last, token_begin, token_end, token_color))
						hasTokenizeResult = true;
				}

				if (hasTokenizeResult == false)
				{
					// todo : remove
						//printf("using regex for %.*s\n", first + 10 < last ? 10 : int(last - first), first);

					for (auto& p : mRegexList)
					{
						if (std::regex_search(first, last, results, p.first, std::regex_constants::match_continuous))
						{
							hasTokenizeResult = true;

							auto& v = *results.begin();
							token_begin = v.first;
							token_end = v.second;
							token_color = p.second;
							break;
						}
					}
				}

				// 如果检查不到匹配项,处理下一行
				if (hasTokenizeResult == false)
				{
					first++;
				}
				else
				{
					const size_t token_length = token_end - token_begin;

					if (token_color == PaletteIndex::Identifier)
					{
						id.assign(token_begin, token_end);

						// todo : allmost all language definitions use lower case to specify keywords, so shouldn't this use ::tolower ?
						if (!mLanguageDefinition.mCaseSensitive)
							std::transform(id.begin(), id.end(), id.begin(), ::toupper);

						if (!line[first - bufferBegin].mPreprocessor)
						{
							if (mLanguageDefinition.mKeywords.count(id) != 0)
								token_color = PaletteIndex::Keyword;
							else if (mLanguageDefinition.mIdentifiers.count(id) != 0)
								token_color = PaletteIndex::KnownIdentifier;
							else if (mLanguageDefinition.mPreprocIdentifiers.count(id) != 0)
								token_color = PaletteIndex::PreprocIdentifier;
							else {
								bool found = false;

								// functions, arguments, local variables
								for (const auto& func : mACFunctions) {
									if (strcmp(func.first.c_str(), id.c_str()) == 0) {
										token_color = PaletteIndex::UserFunction;
										found = true;
										break;
									}

									if (i >= func.second.LineStart - 3 && i <= func.second.LineEnd + 1) {
										for (const auto& arg : func.second.Arguments) {
											if (strcmp(arg.c_str(), id.c_str()) == 0) {
												token_color = PaletteIndex::FunctionArgument;
												found = true;
												break;
											}
										}
										if (!found) {
											for (const auto& loc : func.second.Locals) {
												if (strcmp(loc.c_str(), id.c_str()) == 0) {
													token_color = PaletteIndex::LocalVariable;
													found = true;
													break;
												}
											}
											if (found) break;
										}
										else
											break;
									}
								}

								// uniforms
								if (!found) {
									for (const auto& unif : mACUniforms) {
										if (strcmp(unif.c_str(), id.c_str()) == 0) {
											token_color = PaletteIndex::UniformVariable;
											found = true;
											break;
										}
									}
								}

								// globals
								if (!found) {
									for (const auto& glob : mACGlobals) {
										if (strcmp(glob.c_str(), id.c_str()) == 0) {
											token_color = PaletteIndex::GlobalVariable;
											found = true;
											break;
										}
									}
								}

								// user types
								if (!found) {
									for (const auto& userType : mACUserTypes) {
										if (strcmp(userType.c_str(), id.c_str()) == 0) {
											token_color = PaletteIndex::UserType;
											found = true;
											break;
										}
									}
								}
							}
						}
						else
						{
							if (mLanguageDefinition.mPreprocIdentifiers.count(id) != 0)
								token_color = PaletteIndex::PreprocIdentifier;
						}
					}

					for (size_t j = 0; j < token_length; ++j)
						line[(token_begin - bufferBegin) + j].mColorIndex = token_color;

					first = token_end;
				}
			}
		}
	}

	void TextEditor::ColorizeInternal()
	{
		if (mLines.empty() || !mColorizerEnabled)
			return;

		if (mCheckComments)
		{
			auto endLine = mLines.size();
			auto endIndex = 0;
			auto commentStartLine = endLine;
			auto commentStartIndex = endIndex;
			auto withinString = false;
			auto withinSingleLineComment = false;
			auto withinPreproc = false;
			auto firstChar = true;			// there is no other non-whitespace characters in the line before
			auto concatenate = false;		// '\' on the very end of the line
			auto currentLine = 0;
			auto currentIndex = 0;
			while (currentLine < endLine || currentIndex < endIndex)
			{
				auto& line = mLines[currentLine];

				if (currentIndex == 0 && !concatenate)
				{
					withinSingleLineComment = false;
					withinPreproc = false;
					firstChar = true;
				}

				concatenate = false;

				if (!line.empty())
				{
					auto& g = line[currentIndex];
					auto c = g.mChar;

					if (c != mLanguageDefinition.mPreprocChar && !isspace(c))
						firstChar = false;

					if (currentIndex == (int)line.size() - 1 && line[line.size() - 1].mChar == '\\')
						concatenate = true;

					bool inComment = (commentStartLine < currentLine || (commentStartLine == currentLine && commentStartIndex <= currentIndex));

					if (withinString)
					{
						line[currentIndex].mMultiLineComment = inComment;

						if (c == '\"')
						{
							if (currentIndex + 1 < (int)line.size() && line[currentIndex + 1].mChar == '\"')
							{
								currentIndex += 1;
								if (currentIndex < (int)line.size())
									line[currentIndex].mMultiLineComment = inComment;
							}
							else
								withinString = false;
						}
						else if (c == '\\')
						{
							currentIndex += 1;
							if (currentIndex < (int)line.size())
								line[currentIndex].mMultiLineComment = inComment;
						}
					}
					else
					{
						if (firstChar && c == mLanguageDefinition.mPreprocChar)
							withinPreproc = true;

						if (c == '\"')
						{
							withinString = true;
							line[currentIndex].mMultiLineComment = inComment;
						}
						else
						{
							auto pred = [](const char& a, const Glyph& b) { return a == b.mChar; };
							auto from = line.begin() + currentIndex;
							auto& startStr = mLanguageDefinition.mCommentStart;
							auto& singleStartStr = mLanguageDefinition.mSingleLineComment;

							if (singleStartStr.size() > 0 &&
								currentIndex + singleStartStr.size() <= line.size() &&
								equals(singleStartStr.begin(), singleStartStr.end(), from, from + singleStartStr.size(), pred))
							{
								withinSingleLineComment = true;
							}
							else if (!withinSingleLineComment && currentIndex + startStr.size() <= line.size() &&
								equals(startStr.begin(), startStr.end(), from, from + startStr.size(), pred))
							{
								commentStartLine = currentLine;
								commentStartIndex = currentIndex;
							}

							inComment = inComment = (commentStartLine < currentLine || (commentStartLine == currentLine && commentStartIndex <= currentIndex));

							line[currentIndex].mMultiLineComment = inComment;
							line[currentIndex].mComment = withinSingleLineComment;

							auto& endStr = mLanguageDefinition.mCommentEnd;
							if (currentIndex + 1 >= (int)endStr.size() &&
								equals(endStr.begin(), endStr.end(), from + 1 - endStr.size(), from + 1, pred))
							{
								commentStartIndex = endIndex;
								commentStartLine = endLine;
							}
						}
					}
					line[currentIndex].mPreprocessor = withinPreproc;
					currentIndex += UTF8CharLength(c);
					if (currentIndex >= (int)line.size())
					{
						currentIndex = 0;
						++currentLine;
					}
				}
				else
				{
					currentIndex = 0;
					++currentLine;
				}
			}
			mCheckComments = false;
		}

		if (mColorRangeMin < mColorRangeMax)
		{
			const int increment = (mLanguageDefinition.mTokenize == nullptr) ? 10 : 10000;
			const int to = std::min<int>(mColorRangeMin + increment, mColorRangeMax);
			ColorizeRange(mColorRangeMin, to);
			mColorRangeMin = to;

			if (mColorRangeMax == mColorRangeMin)
			{
				mColorRangeMin = std::numeric_limits<int>::max();
				mColorRangeMax = 0;
			}
			return;
		}
	}

	float TextEditor::TextDistanceToLineStart(const Coordinates& aFrom) const
	{
		auto& line = mLines[aFrom.mLine];
		float distance = 0.0f;
		float spaceSize = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, " ", nullptr, nullptr).x;
		int colIndex = GetCharacterIndex(aFrom);
		for (size_t it = 0u; it < line.size() && it < colIndex; )
		{
			if (line[it].mChar == '\t')
			{
				distance = (1.0f + std::floor((1.0f + distance) / (float(mTabSize) * spaceSize))) * (float(mTabSize) * spaceSize);
				++it;
			}
			else
			{
				auto d = UTF8CharLength(line[it].mChar);
				char tempCString[7];
				int i = 0;
				for (; i < 6 && d-- > 0 && it < (int)line.size(); i++, it++)
					tempCString[i] = line[it].mChar;

				tempCString[i] = '\0';
				distance += ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, tempCString, nullptr, nullptr).x;
			}
		}

		return distance;
	}

	void TextEditor::EnsureCursorVisible()
	{
		if (!mWithinRender)
		{
			mScrollToCursor = true;
			return;
		}

		float scrollX = ImGui::GetScrollX();
		float scrollY = ImGui::GetScrollY();

		auto height = ImGui::GetWindowHeight();
		auto width = mWindowWidth;

		auto top = 1 + (int)ceil(scrollY / mCharAdvance.y);
		auto bottom = (int)ceil((scrollY + height) / mCharAdvance.y);

		auto left = (int)ceil(scrollX / mCharAdvance.x);
		auto right = (int)ceil((scrollX + width) / mCharAdvance.x);

		auto pos = GetActualCursorCoordinates();
		auto len = TextDistanceToLineStart(pos);

		if (pos.mLine < top)
			ImGui::SetScrollY(std::max(0.0f, (pos.mLine - 1) * mCharAdvance.y));
		if (pos.mLine > bottom - 4)
			ImGui::SetScrollY(std::max(0.0f, (pos.mLine + 4) * mCharAdvance.y - height));
		if (pos.mColumn < left)
			ImGui::SetScrollX(std::max(0.0f, len + mTextStart - 11 * mCharAdvance.x));
		if (len + mTextStart > (right - 4) * mCharAdvance.x)
			ImGui::SetScrollX(std::max(0.0f, len + mTextStart + 4 * mCharAdvance.x - width));
	}

	void TextEditor::SetCurrentLineIndicator(int line) {
		mDebugCurrentLine = line;
		mDebugCurrentLineUpdated = line > 0;
	}

	int TextEditor::GetPageSize() const
	{
		auto height = ImGui::GetWindowHeight() - 20.0f;
		return (int)floor(height / mCharAdvance.y);
	}

	TextEditor::UndoRecord::UndoRecord(
		const std::string& aAdded,
		const Coordinates aAddedStart,
		const Coordinates aAddedEnd,
		const std::string& aRemoved,
		const Coordinates aRemovedStart,
		const Coordinates aRemovedEnd,
		TextEditor::EditorState& aBefore,
		TextEditor::EditorState& aAfter)
		: mAdded(aAdded)
		, mAddedStart(aAddedStart)
		, mAddedEnd(aAddedEnd)
		, mRemoved(aRemoved)
		, mRemovedStart(aRemovedStart)
		, mRemovedEnd(aRemovedEnd)
		, mBefore(aBefore)
		, mAfter(aAfter)
	{
		assert(mAddedStart <= mAddedEnd);
		assert(mRemovedStart <= mRemovedEnd);
	}

	void TextEditor::UndoRecord::Undo(TextEditor* aEditor)
	{
		if (!mAdded.empty())
		{
			aEditor->DeleteRange(mAddedStart, mAddedEnd);
			aEditor->Colorize(mAddedStart.mLine - 1, mAddedEnd.mLine - mAddedStart.mLine + 2);
		}

		if (!mRemoved.empty())
		{
			auto start = mRemovedStart;
			aEditor->InsertTextAt(start, mRemoved.c_str());
			aEditor->Colorize(mRemovedStart.mLine - 1, mRemovedEnd.mLine - mRemovedStart.mLine + 2);
		}

		aEditor->mState = mBefore;
		aEditor->EnsureCursorVisible();
	}

	void TextEditor::UndoRecord::Redo(TextEditor* aEditor)
	{
		if (!mRemoved.empty())
		{
			aEditor->DeleteRange(mRemovedStart, mRemovedEnd);
			aEditor->Colorize(mRemovedStart.mLine - 1, mRemovedEnd.mLine - mRemovedStart.mLine + 1);
		}

		if (!mAdded.empty())
		{
			auto start = mAddedStart;
			aEditor->InsertTextAt(start, mAdded.c_str());
			aEditor->Colorize(mAddedStart.mLine - 1, mAddedEnd.mLine - mAddedStart.mLine + 1);
		}

		aEditor->mState = mAfter;
		aEditor->EnsureCursorVisible();
	}
}
