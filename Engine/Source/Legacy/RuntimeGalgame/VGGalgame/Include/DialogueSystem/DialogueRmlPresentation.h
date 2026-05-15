/*
 * DialogueRmlPresentation — 对白 Rml 数据模型绑定（Phase 8 Presentation 子层）
 *
 * 中文：仅负责 `dialog` DataModel 与 `dialog_name` / `dialog_text` 绑定及脏标记；
 * 不包含打字机、自动播放、脚本 Continue 等运行时逻辑。
 */

#pragma once
#include "../VGGalgameConfig.h"
#include <RmlUi/Core.h>
#include <string>

namespace VisionGal::GalGame
{
	class VG_GALGAME_API DialogueRmlPresentation final
	{
	public:
		DialogueRmlPresentation() = default;
		~DialogueRmlPresentation() = default;

		/// 中文：在 Rml Context 上创建 `dialog` 模型并绑定到内部字符串。
		bool InitialiseDataModel(Rml::Context* context);

		/// 中文：每帧标记全部绑定变量脏，驱动 UI 刷新（原 DialogueSystem::Update 末尾逻辑）。
		void MarkAllVariablesDirty();

		[[nodiscard]] std::string& MutableBoundDialogText() noexcept { return m_DialogText; }
		[[nodiscard]] const std::string& BoundDialogName() const noexcept { return m_DialogName; }
		void SetBoundDialogName(std::string name) { m_DialogName = std::move(name); }

	private:
		std::string m_DialogName;
		std::string m_DialogText;
		Rml::DataModelHandle m_ModelHandle{};
	};
}
