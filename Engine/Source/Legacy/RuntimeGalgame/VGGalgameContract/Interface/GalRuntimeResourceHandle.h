/*
 * GalRuntimeResourceHandle — 运行时资源句柄与 Owner 契约（Phase 8E 骨架）
 *
 * 中文：统一 **GalSprite** / **GalAudio** / **GalVideo** / **IGalCharacter** 等在热重载、切场景、**ResetRuntime** 下的回收键；
 * **RuntimeResourceRegistry**（未落地）将 **Handle → WeakRef + Owner**；本头仅放类型别名与 Owner 接口形状。
 */

#pragma once

#include <cstdint>

namespace VisionGal::GalGame
{
	using GalRuntimeResourceHandle = std::uint64_t;

	struct IGalRuntimeResourceOwner
	{
		virtual ~IGalRuntimeResourceOwner() = default;

		/// 中文：当注册表销毁某 **Handle** 对应资源时回调；**Handle** 此后失效。
		virtual void OnGalRuntimeResourceDestroyed(GalRuntimeResourceHandle handle) noexcept = 0;
	};
}
