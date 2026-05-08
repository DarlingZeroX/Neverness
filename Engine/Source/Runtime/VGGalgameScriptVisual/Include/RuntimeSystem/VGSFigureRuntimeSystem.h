/*
 * VGSFigureRuntimeSystem — 角色立绘（Figure）类 Sequence 组件运行时域
 *
 * 当前支持：VGSSC_ChangeFigure
 */
#pragma once

#include "../../VGGalScriptVisualConfig.h"
#include "../../Interface/IVGSSequenceRuntimeSystem.h"

namespace VisionGal
{
	class VG_GALGAME_VISUAL_SCRIPT_API VGSFigureRuntimeSystem final : public IVGSSequenceRuntimeSystem
	{
	public:
		~VGSFigureRuntimeSystem() override = default;

		[[nodiscard]] bool CanExecute(IVGSSequenceComponent* component) const override;
		void Execute(IVGSSequenceComponent* component, SSSequenceExecutionContext& context) override;
		[[nodiscard]] bool ShouldHoldPlaybackAfterExecute(IVGSSequenceComponent* component) const override;
	};
}
