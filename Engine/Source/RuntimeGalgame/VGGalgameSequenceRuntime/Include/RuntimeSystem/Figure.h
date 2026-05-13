/*
 * VGSFigureRuntimeSystem — 角色立绘（Figure）类 Sequence 组件运行时域
 *
 * 当前支持：VGSSC_ChangeFigure
 */
#pragma once

#include "../../GSSExport.h"
#include "../../Interface/IVGSSequenceRuntimeSystem.h"

namespace VisionGal
{
	class VG_GSS_API VGSFigureRuntimeSystem final : public IVGSSequenceRuntimeSystem
	{
	public:
		~VGSFigureRuntimeSystem() override = default;

		[[nodiscard]] bool SupportsType(SequenceComponentTypeID id) const override;
		[[nodiscard]] bool CanExecute(IVGSSequenceComponent* component) const override;
		void Execute(IVGSSequenceComponent* component, SequenceRuntimeExecutionContext& context) override;
		[[nodiscard]] bool ShouldHoldPlaybackAfterExecute(IVGSSequenceComponent* component) const override;
	};
}
