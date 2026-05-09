/*
 * VGSDialogueRuntimeSystem — 对话类 Sequence 组件运行时域
 *
 * 当前支持：VGSSC_CommonDialogue
 */
#pragma once

#include "../../GSSExport.h"
#include "../../Interface/IVGSSequenceRuntimeSystem.h"

namespace VisionGal
{
	class VG_GSS_API VGSDialogueRuntimeSystem final : public IVGSSequenceRuntimeSystem
	{
	public:
		~VGSDialogueRuntimeSystem() override = default;

		[[nodiscard]] bool CanExecute(IVGSSequenceComponent* component) const override;
		void Execute(IVGSSequenceComponent* component, SSSequenceExecutionContext& context) override;
		[[nodiscard]] bool ShouldHoldPlaybackAfterExecute(IVGSSequenceComponent* component) const override;
	};
}
