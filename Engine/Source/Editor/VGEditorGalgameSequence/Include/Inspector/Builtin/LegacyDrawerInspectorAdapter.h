/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Inspector/ISequenceInspector.h"

#include <string>

namespace VisionGal::Editor
{
	/// Bridges legacy `IGalSeqComDrawer` to `ISequenceInspector` until native inspectors exist.
	class LegacyDrawerInspectorAdapter final : public ISequenceInspector
	{
	public:
		explicit LegacyDrawerInspectorAdapter(std::string typeNameID, std::string displayName);

		void OnInspectorGUI(unsigned int index, VisionGal::IVGSSequenceComponent* component, SequenceEditorContext* context) override;

		std::string GetDisplayName() const override { return m_displayName; }

		std::string GetBoundTypeNameID() const override { return m_typeNameID; }

	private:
		std::string m_typeNameID;
		std::string m_displayName;

		/// Staging for `VGSSC_CommonDialogue` undo-aware fields (index 0xFFFFFFFFu = invalid).
		unsigned m_commonStagingIndex = 0xFFFFFFFFu;
		std::string m_commonStagingName;
		std::string m_commonStagingText;
	};
}
