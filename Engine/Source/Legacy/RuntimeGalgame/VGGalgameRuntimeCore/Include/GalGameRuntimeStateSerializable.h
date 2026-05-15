/*
 * GalGameRuntimeStateSerializable — GalGameRuntimeState 与 JSON 的桥接（Phase 8D）
 *
 * 中文：实现 **ISerializableRuntimeState**，供 **GalRuntimeCoordinator::SaveRuntimeState** / **RestoreRuntimeState**
 * 做内存快照（回滚 / 调试器 / 未来 SaveArchive 子段）；**不**序列化 **screenshotPixels**（GPU 像素块），恢复时置空。
 */

#pragma once
#include "../VGGRCExport.h"
#include "GalGameRuntimeState.h"
#include "ISerializableRuntimeState.h"

namespace VisionGal::GalGame
{
	class VG_RUNTIME_GALCORE_API GalGameRuntimeStateSerializable final : public ISerializableRuntimeState
	{
	public:
		/// 中文：**state** 须由宿主保证在 Save/Load 调用期内有效；可为 **nullptr**（此时读写空对象）。
		explicit GalGameRuntimeStateSerializable(GalGameRuntimeState* state) noexcept
			: m_State(state)
		{
		}

		void SaveToJson(nlohmann::json& out) const override;
		bool LoadFromJson(const nlohmann::json& in) override;

	private:
		GalGameRuntimeState* m_State = nullptr;
	};
}
