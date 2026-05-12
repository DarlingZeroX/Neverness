/*
 * VGSBackgroundRuntimeSystem — 背景（Background）类 Sequence 组件运行时域
 *
 * 当前支持：VGSSC_ChangeBackground
 */
#pragma once

#include "../../GSSExport.h"
#include "../../Interface/IVGSSequenceRuntimeSystem.h"

namespace VisionGal
{
	class VG_GSS_API VGSBackgroundRuntimeSystem final : public IVGSSequenceRuntimeSystem
	{
	public:
		~VGSBackgroundRuntimeSystem() override = default;

		[[nodiscard]] bool SupportsType(SequenceComponentTypeID id) const override;
		[[nodiscard]] bool CanExecute(IVGSSequenceComponent* component) const override;
		void Execute(IVGSSequenceComponent* component, SequenceRuntimeExecutionContext& context) override;
		[[nodiscard]] bool ShouldHoldPlaybackAfterExecute(IVGSSequenceComponent* component) const override;
	};
}
