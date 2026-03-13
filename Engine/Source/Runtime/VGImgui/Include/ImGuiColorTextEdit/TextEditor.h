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

#pragma once
#include "TextEditorDefines.h"

#include <array>
#include <functional>
#include <thread>
#include <regex>

namespace ImGuiTextEditor {
	class IMGUI_API TextEditor {
	public:

		TextEditor();
		~TextEditor();
		 
		// 设置语言定义
		void SetLanguageDefinition(const LanguageDefinition& aLanguageDef);
		// 获取当前的语言定义
		const LanguageDefinition& GetLanguageDefinition() const { return mLanguageDefinition; }

		// 获取调色板
		const Palette& GetPalette() const { return mPaletteBase; }
		// 设置调色板
		void SetPalette(const Palette& aValue);

		// 设置错误标记
		void SetErrorMarkers(const ErrorMarkers& aMarkers) { mErrorMarkers = aMarkers; }

		// 断点
		bool HasBreakpoint(int line);
		void AddBreakpoint(int line, std::string condition = "", bool enabled = true);
		void RemoveBreakpoint(int line);
		void SetBreakpointEnabled(int line, bool enable);
		Breakpoint& GetBreakpoint(int line);
		inline const std::vector<Breakpoint>& GetBreakpoints() { return mBreakpoints; }
		void SetCurrentLineIndicator(int line);

		// 是否正在调试
		bool IsDebugging() const noexcept { return mDebugCurrentLine > 0; }

		// 渲染
		void Render(const char* aTitle, const ImVec2& aSize = ImVec2(), bool aBorder = false);

		// 编辑器文本
		void SetText(const std::string& aText);
		std::string GetText() const;

		// 文本设置获取
		void SetTextLines(const std::vector<std::string>& aLines);	//设置每一行文本
		void GetTextLines(std::vector<std::string>& out) const;		//获取每一行文本
		std::string GetSelectedText() const;		// 获取选择的文本
		std::string GetCurrentLineText() const;		// 获取当前行文本

		// 获取行数
		int GetTotalLines() const { return (int)mLines.size(); }

		// 编辑器状态
		bool IsOverwrite() const { return mOverwrite; }  // 判断是否是覆盖模式
		bool IsFocused() const { return mFocused; }  // 判断是否获得焦点
		void SetReadOnly(bool aValue);  // 设置只读状态
		bool IsReadOnly() const noexcept { return mReadOnly || IsDebugging(); }  // 判断是否为只读模式
		bool IsTextChanged() const { return mTextChanged; }  // 判断文本是否已改变
		bool IsCursorPositionChanged() const { return mCursorPositionChanged; }  // 判断光标位置是否已改变
		inline void ResetTextChanged() { mTextChanged = false; }  // 重置文本改变状态

		bool IsColorizerEnabled() const { return mColorizerEnabled; }
		void SetColorizerEnable(bool aValue);

		// 光标
		Coordinates GetCursorPosition() const { return GetActualCursorCoordinates(); }
		void SetCursorPosition(const Coordinates& aPosition);

		// 鼠标输入
		inline void SetHandleMouseInputs(bool aValue) { mHandleMouseInputs = aValue; }
		inline bool IsHandleMouseInputsEnabled() const { return mHandleKeyboardInputs; }

		// 键盘输入
		inline void SetHandleKeyboardInputs(bool aValue) { mHandleKeyboardInputs = aValue; }
		inline bool IsHandleKeyboardInputsEnabled() const { return mHandleKeyboardInputs; }

		inline void SetImGuiChildIgnored(bool aValue) { mIgnoreImGuiChild = aValue; }
		inline bool IsImGuiChildIgnored() const { return mIgnoreImGuiChild; }

		// 是否用点代表空格显示
		inline void SetShowWhitespaces(bool aValue) { mShowWhitespaces = aValue; }
		inline bool IsShowingWhitespaces() const { return mShowWhitespaces; }

		// 是否用箭头代表制表符,也就是按键Tab
		void SetShowTabArrow(bool aValue) { mShowTabArrow = aValue; }
		bool IsShowingShowTabArrow() const { return mShowTabArrow; }

		// 插入文本
		void InsertText(const std::string& aValue, bool indent = false);
		void InsertText(const char* aValue, bool indent = false);

		// 移动
		void MoveUp(int aAmount = 1, bool aSelect = false);
		void MoveDown(int aAmount = 1, bool aSelect = false);
		void MoveLeft(int aAmount = 1, bool aSelect = false, bool aWordMode = false);
		void MoveRight(int aAmount = 1, bool aSelect = false, bool aWordMode = false);
		void MoveTop(bool aSelect = false);
		void MoveBottom(bool aSelect = false);
		void MoveHome(bool aSelect = false);
		void MoveEnd(bool aSelect = false);

		// 选取
		void SetSelectionStart(const Coordinates& aPosition);
		void SetSelectionEnd(const Coordinates& aPosition);
		void SetSelection(const Coordinates& aStart, const Coordinates& aEnd, SelectionMode aMode = SelectionMode::Normal);
		void SelectWordUnderCursor();
		void SelectAll();
		bool HasSelection() const;

		// 功能键
		void Copy();
		void Cut();
		void Paste();
		void Delete();
		bool CanUndo();
		bool CanRedo();
		void Undo(int aSteps = 1);
		void Redo(int aSteps = 1);

		// 制表符在编辑占用大小
		inline void SetTabSize(int s) { mTabSize = std::max<int>(0, std::min<int>(32, s)); }
		inline int GetTabSize() { return mTabSize; }

		// 插入空格
		inline void SetInsertSpaces(bool s) { mInsertSpaces = s; }
		inline int GetInsertSpaces() { return mInsertSpaces; }

		inline void SetSmartIndent(bool s) { mSmartIndent = s; }
		inline void SetAutoIndentOnPaste(bool s) { mAutoindentOnPaste = s; }
		inline void SetHighlightLine(bool s) { mHighlightLine = s; }
		inline void SetCompleteBraces(bool s) { mCompleteBraces = s; }
		inline void SetHorizontalScroll(bool s) { mHorizontalScroll = s; }
		inline void SetSmartPredictions(bool s) { mAutocomplete = s; }
		inline void SetFunctionTooltips(bool s) { mFuncTooltips = s; }
		inline void SetActiveAutocomplete(bool cac) { mActiveAutocomplete = cac; }
		inline void SetScrollbarMarkers(bool markers) { mScrollbarMarkers = markers; }
		inline void SetSidebarVisible(bool s) { mSidebar = s; }
		inline void SetSearchEnabled(bool s) { mHasSearch = s; }
		inline void SetHiglightBrackets(bool s) { mHighlightBrackets = s; }

		// UI 设置
		inline void SetUIScale(float scale) { mUIScale = scale; }
		inline void SetUIFontSize(float size) { mUIFontSize = size; }
		inline void SetEditorFontSize(float size) { mEditorFontSize = size; }

		// 快捷键
		void SetShortcut(ShortcutID id, Shortcut s);
		static const std::vector<Shortcut> GetDefaultShortcuts();

		inline void SetShowLineNumbers(bool s) { mShowLineNumbers = s;	mTextStart = (s ? 20 : 6);	mLeftMargin = (s ? (DebugDataSpace + LineNumberSpace) : (DebugDataSpace - LineNumberSpace)); }
		inline int GetTextStart() const { return mShowLineNumbers ? 7 : 3; }

		// 文本着色
		void Colorize(int aFromLine = 0, int aCount = -1);
		void ColorizeRange(int aFromLine = 0, int aToLine = 0);
		void ColorizeInternal();

		// 自动补全相关功能
		inline void ClearAutocompleteData() { mACFunctions.clear(); mACUserTypes.clear(); mACUniforms.clear(); mACGlobals.clear(); }
		inline void ClearAutocompleteEntries() { mACEntries.clear(); mACEntrySearch.clear(); }
		inline const std::unordered_map<std::string, FunctionData>& GetAutocompleteFunctions() { return mACFunctions; }
		inline const std::vector<std::string>& GetAutocompleteUserTypes() { return mACUserTypes; }
		inline const std::vector<std::string>& GetAutocompleteUniforms() { return mACUniforms; }
		inline const std::vector<std::string>& GetAutocompleteGlobals() { return mACGlobals; }
		inline void AddAutocompleteFunction(const std::string& fname, int lineStart, int lineEnd, const std::vector<std::string>& args, const std::vector<std::string>& locals) { mACFunctions[fname] = FunctionData(lineStart, lineEnd, args, locals); }
		inline void AddAutocompleteUserType(const std::string& fname) { mACUserTypes.push_back(fname); }
		inline void AddAutocompleteUniform(const std::string& fname) { mACUniforms.push_back(fname); }
		inline void AddAutocompleteGlobal(const std::string& fname) { mACGlobals.push_back(fname); }
		inline void AddAutocompleteEntry(const std::string& search, const std::string& display, const std::string& value) { mACEntrySearch.push_back(search); mACEntries.push_back(std::make_pair(display, value)); }

		// 调色板
		static const Palette& GetDarkPalette();
		static const Palette& GetLightPalette();
		static const Palette& GetRetroBluePalette();

		std::function<void(TextEditor*, const std::string&)> OnSave;
		std::function<void(TextEditor*, int)> OnDebuggerJump;
		std::function<void(TextEditor*, DebugAction)> OnDebuggerAction;
		std::function<void(TextEditor*, const std::string&)> OnIdentifierHover;
		std::function<bool(TextEditor*, const std::string&)> HasIdentifierHover;
		std::function<void(TextEditor*, const std::string&)> OnExpressionHover;
		std::function<bool(TextEditor*, const std::string&)> HasExpressionHover;
		std::function<void(TextEditor*, int)> OnBreakpointRemove;
		std::function<void(TextEditor*, int, const std::string&, bool)> OnBreakpointUpdate;

		std::function<void(TextEditor*)> OnContentUpdate;

		// 文本路径
		inline void SetPath(const std::string& path) { mPath = path; }
		inline const std::string& GetPath() { return mPath; }
	private:
		std::string mPath;

		typedef std::vector<std::pair<std::regex, PaletteIndex>> RegexList;

		struct EditorState
		{
			Coordinates mSelectionStart;
			Coordinates mSelectionEnd;
			Coordinates mCursorPosition;
		};

		class UndoRecord
		{
		public:
			UndoRecord() {}
			~UndoRecord() {}

			UndoRecord(
				const std::string& aAdded,
				const Coordinates aAddedStart,
				const Coordinates aAddedEnd,

				const std::string& aRemoved,
				const Coordinates aRemovedStart,
				const Coordinates aRemovedEnd,

				TextEditor::EditorState& aBefore,
				TextEditor::EditorState& aAfter);

			void Undo(TextEditor* aEditor);
			void Redo(TextEditor* aEditor);

			std::string mAdded;
			Coordinates mAddedStart;
			Coordinates mAddedEnd;

			std::string mRemoved;
			Coordinates mRemovedStart;
			Coordinates mRemovedEnd;

			EditorState mBefore;
			EditorState mAfter;
		};

		typedef std::vector<UndoRecord> UndoBuffer;

		// 处理输入
		void ProcessInputs();
		float TextDistanceToLineStart(const Coordinates& aFrom) const;
		void EnsureCursorVisible();
		// 获取页面大小
		int GetPageSize() const;
		// 获取文本
		std::string GetText(const Coordinates& aStart, const Coordinates& aEnd) const;
		Coordinates GetActualCursorCoordinates() const;
		Coordinates SanitizeCoordinates(const Coordinates& aValue) const;
		void Advance(Coordinates& aCoordinates) const;
		// 范围删除
		void DeleteRange(const Coordinates& aStart, const Coordinates& aEnd);
		int InsertTextAt(Coordinates& aWhere, const char* aValue, bool indent = false);
		// 添加撤销
		void AddUndo(UndoRecord& aValue);
		// 坐标映射
		Coordinates ScreenPosToCoordinates(const ImVec2& aPosition) const;
		Coordinates MousePosToCoordinates(const ImVec2& aPosition) const;
		ImVec2 CoordinatesToScreenPos(const Coordinates& aPosition) const;
		// 查找字
		Coordinates FindWordStart(const Coordinates& aFrom) const;
		Coordinates FindWordEnd(const Coordinates& aFrom) const;
		Coordinates FindNextWord(const Coordinates& aFrom) const;
		int GetCharacterIndex(const Coordinates& aCoordinates) const;
		int GetCharacterColumn(int aLine, int aIndex) const;
		int GetLineCharacterCount(int aLine) const;
		int GetLineMaxColumn(int aLine) const;
		bool IsOnWordBoundary(const Coordinates& aAt) const;
		// 行
		void RemoveLine(int aStart, int aEnd);
		void RemoveLine(int aIndex);
		Line& InsertLine(int aIndex);
		// 插入字符
		void EnterCharacter(ImWchar aChar, bool aShift);
		// 退格键
		void Backspace();
		// 删除选择的字
		void DeleteSelection();
		std::string GetWordUnderCursor() const;
		std::string GetWordAt(const Coordinates& aCoords) const;
		ImU32 GetGlyphColor(const Glyph& aGlyph) const;

		// 输入处理
		void HandleKeyboardInputs();
		void HandleMouseInputs();

		// 渲染内部实现
		void RenderInternal(const char* aTitle);

		bool mFuncTooltips;

		// UI字体相关
		float mUIScale, mUIFontSize, mEditorFontSize;
		inline float mUICalculateSize(float h) { return h * (mUIScale + mUIFontSize / 18.0f - 1.0f); }
		inline float mEditorCalculateSize(float h) { return h * (mUIScale + mEditorFontSize / 18.0f - 1.0f); }

		float mLineSpacing;
		Lines mLines;
		EditorState mState;
		UndoBuffer mUndoBuffer;
		int mUndoIndex;
		int mReplaceIndex;

		bool mSidebar;
		bool mHasSearch;

		// 查找相关定义
		char mFindWord[256];
		bool mFindOpened;
		bool mFindJustOpened;
		bool mFindNext;
		bool mFindFocused, mReplaceFocused;
		bool mReplaceOpened;
		char mReplaceWord[256];

		std::vector<std::string> mACEntrySearch;
		std::vector<std::pair<std::string, std::string>> mACEntries;

		bool mIsSnippet;
		std::vector<Coordinates> mSnippetTagStart, mSnippetTagEnd;
		std::vector<int> mSnippetTagID;
		std::vector<bool> mSnippetTagHighlight;
		int mSnippetTagSelected, mSnippetTagLength, mSnippetTagPreviousLength;
		std::string mAutcompleteParse(const std::string& str, const Coordinates& start);
		void mAutocompleteSelect();

		// 自动补全相关定义
		bool m_requestAutocomplete, m_readyForAutocomplete;
		void m_buildSuggestions(bool* keepACOpened = nullptr);	// 自动补全显示菜单
		bool mActiveAutocomplete;
		bool mAutocomplete;
		std::unordered_map<std::string, FunctionData> mACFunctions;
		std::vector<std::string> mACUserTypes, mACUniforms, mACGlobals;
		std::string mACWord;
		std::vector<std::pair<std::string, std::string>> mACSuggestions;
		int mACIndex;
		bool mACOpened;
		bool mACSwitched; // if == true then allow selection with enter
		Coordinates mACPosition;

		std::vector<Shortcut> m_shortcuts;

		bool mScrollbarMarkers;
		bool mHorizontalScroll;
		bool mCompleteBraces;
		bool mShowLineNumbers;
		bool mHighlightLine;
		bool mHighlightBrackets;
		bool mInsertSpaces;
		bool mSmartIndent;
		bool mFocused;
		int mTabSize;
		bool mOverwrite;
		bool mReadOnly;
		bool mWithinRender;
		bool mScrollToCursor;
		bool mScrollToTop;
		bool mTextChanged;
		bool mColorizerEnabled;
		float mTextStart;                   // position (in pixels) where a code line starts relative to the left of the TextEditor.
		int  mLeftMargin;
		bool mCursorPositionChanged;
		int mColorRangeMin, mColorRangeMax;
		SelectionMode mSelectionMode;
		bool mHandleKeyboardInputs;
		bool mHandleMouseInputs;
		bool mIgnoreImGuiChild;
		bool mShowWhitespaces = false;	// 用点代表显示空格
		bool mShowTabArrow = false;		// 用箭头代表显示制表符
		bool mAutoindentOnPaste;

		Palette mPaletteBase;
		Palette mPalette;
		LanguageDefinition mLanguageDefinition;
		RegexList mRegexList;			// 正则匹配列表

		float mDebugBarWidth, mDebugBarHeight;

		bool mDebugCurrentLineUpdated;
		int mDebugCurrentLine;
		ImVec2 mUICursorPos, mFindOrigin;
		float mWindowWidth;
		std::vector<Breakpoint> mBreakpoints;
		ImVec2 mRightClickPos;

		int mPopupCondition_Line;
		bool mPopupCondition_Use;
		char mPopupCondition_Condition[512];

		bool mCheckComments;
		ErrorMarkers mErrorMarkers;
		ImVec2 mCharAdvance;
		Coordinates mInteractiveStart, mInteractiveEnd;
		std::string mLineBuffer;
		uint64_t mStartTime;

		Coordinates mLastHoverPosition;
		std::chrono::steady_clock::time_point mLastHoverTime;

		float mLastClick;
	};
}