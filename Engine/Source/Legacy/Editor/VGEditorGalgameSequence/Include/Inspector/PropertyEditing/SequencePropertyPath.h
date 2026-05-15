/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

namespace VisionGal::Editor::SequencePropertyPath
{
	/// 与 `EditSequencePropertyCommand` / 未来 binding 对齐的逻辑路径常量。
	inline constexpr const char kCommonDialogueText[] = "CommonDialogue.DialogueText";
	inline constexpr const char kCommonDialogueCharacterName[] = "CommonDialogue.CharacterName";
	inline constexpr const char kChangeFigureTexturePath[] = "ChangeFigure.TextureResourcePath";
	inline constexpr const char kChangeBackgroundTexturePath[] = "ChangeBackground.TextureResourcePath";
	inline constexpr const char kChangeFigureShowState[] = "ChangeFigure.ShowState";
	inline constexpr const char kChangeFigureWait[] = "ChangeFigure.Wait";
	inline constexpr const char kChangeBackgroundShowState[] = "ChangeBackground.ShowState";
	inline constexpr const char kChangeBackgroundWait[] = "ChangeBackground.Wait";
}
