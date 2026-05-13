/*
 * SSExecutorResourceManager — Visual Galgame Sequence Runtime Registry
 *
 * 这不是「加载纹理/音频文件的 ResourceManager」，而是：
 * - Runtime Object Registry：ObjectID → 运行态 gameplay 接口（IGal*）
 * - Sequencer Binding Layer：Layer 字符串 → 一组对象 Handle（便于 Timeline Track / Stage）
 * - Metadata Sidecar：名称 / 图层 / 持久化标记 — 服务于 Editor Inspector、调试器与存档系统
 *
 * 安全模型：
 * - 内部 std::shared_ptr 延长对象生命周期；对外查询返回裸指针（ nullptr = 不存在或无引用）。
 * - 调用方不得在 Unregister 之后持有裸指针；若需延长寿命，请使用 TryGet*Shared。
 *
 * 扩展指引：
 * - Timeline / Property Animation：在上层引入 Curve / BindingId，勿直接修改本类存储布局；
 * - Cutscene State：作为 Component 挂在 Context 或独立 PlayerState；
 * - 序列化：导出 VGSSRuntimeObjectHandle + Metadata + 外部资产 GUID，不在此文件耦合具体格式。
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "../GSSExport.h"
#include "../Interface/VGSTypeDefine.h"
#include "../Interface/VGSSObjectIDGenerator.h"

namespace VisionGal::GalGame
{
	struct IGalCharacter;
	struct IGalSprite;
	struct IGalAudio;
	struct IGalVideo;
}

namespace VisionGal
{
	/**
	 * @brief 运行时对象种类（用于 Layer 索引与元数据键）。
	 *
	 * 显式枚举数值以便存档 / 网络同步时保持稳定（勿随意重排）。
	 */
	enum class VGSSRuntimeObjectKind : std::uint8_t
	{
		Character = 0,
		Sprite = 1,
		Audio = 2,
		Video = 3,
	};

	/**
	 * @brief 统一的运行时对象 Handle（种类 + ObjectID）。
	 *
	 * 说明：不同种类的 ID 可能数值相等但仍代表不同对象；比较时必须连同 Kind 一并判断。
	 */
	struct VGSSRuntimeObjectHandle
	{
		VGSSRuntimeObjectKind Kind = VGSSRuntimeObjectKind::Character;
		std::uint32_t Id = VGSS_INVALID_OBJECT_ID;

		[[nodiscard]] bool IsValid() const noexcept
		{
			return Id != VGSS_INVALID_OBJECT_ID;
		}

		[[nodiscard]] bool operator==(const VGSSRuntimeObjectHandle& o) const noexcept
		{
			return Kind == o.Kind && Id == o.Id;
		}

		[[nodiscard]] bool operator!=(const VGSSRuntimeObjectHandle& o) const noexcept
		{
			return !(*this == o);
		}
	};

	struct VGSSRuntimeObjectHandleHash
	{
		[[nodiscard]] std::size_t operator()(const VGSSRuntimeObjectHandle& h) const noexcept
		{
			// 折叠哈希以适配 32-bit size_t 平台
			const std::size_t a = static_cast<std::size_t>(h.Id);
			const std::size_t b = static_cast<std::size_t>(h.Kind);
			return a ^ (b + 0x9e3779b9u + (a << 6) + (a >> 2));
		}
	};

	/**
	 * @brief 与 Registry 条目并行存储的元数据（Sidecar）。
	 *
	 * Layer 字段与 SSExecutorResourceManager 内建的 Layer 索引保持一致：
	 * 修改 Metadata.Layer 会触发索引更新（通过 SetMetadata）。
	 */
	struct VGSSObjectMetadata
	{
		std::string Name;
		std::string Layer;
		bool Persistent = false;
	};

	/**
	 * @brief Visual Sequence 运行时对象注册表（核心类）。
	 */
	class VG_GSS_API SSExecutorResourceManager : public std::enable_shared_from_this<SSExecutorResourceManager>
	{
	public:
		SSExecutorResourceManager();
		~SSExecutorResourceManager();

		SSExecutorResourceManager(const SSExecutorResourceManager&) = delete;
		SSExecutorResourceManager& operator=(const SSExecutorResourceManager&) = delete;
		SSExecutorResourceManager(SSExecutorResourceManager&&) = delete;
		SSExecutorResourceManager& operator=(SSExecutorResourceManager&&) = delete;

		// ---------------------------------------------------------------------
		// Character
		// ---------------------------------------------------------------------
		[[nodiscard]] VGSSCharacterObjectID RegisterCharacter(std::shared_ptr<GalGame::IGalCharacter> character);
		void RemoveCharacter(VGSSCharacterObjectID id);
		[[nodiscard]] GalGame::IGalCharacter* GetCharacter(VGSSCharacterObjectID id) const;
		[[nodiscard]] bool HasCharacter(VGSSCharacterObjectID id) const;

		// ---------------------------------------------------------------------
		// Sprite（由 VGSSTextureObjectID 作为槽位 ID）
		// ---------------------------------------------------------------------
		[[nodiscard]] VGSSSpriteObjectID RegisterSprite(std::shared_ptr<GalGame::IGalSprite> sprite);
		void RemoveSprite(VGSSSpriteObjectID id);
		[[nodiscard]] GalGame::IGalSprite* GetSprite(VGSSSpriteObjectID id) const;
		[[nodiscard]] bool HasSprite(VGSSSpriteObjectID id) const;

		// ---------------------------------------------------------------------
		// Audio
		// ---------------------------------------------------------------------
		[[nodiscard]] VGSSAudioObjectID RegisterAudio(std::shared_ptr<GalGame::IGalAudio> audio);
		void RemoveAudio(VGSSAudioObjectID id);
		[[nodiscard]] GalGame::IGalAudio* GetAudio(VGSSAudioObjectID id) const;
		[[nodiscard]] bool HasAudio(VGSSAudioObjectID id) const;

		// ---------------------------------------------------------------------
		// Video
		// ---------------------------------------------------------------------
		[[nodiscard]] VGSSVideoObjectID RegisterVideo(std::shared_ptr<GalGame::IGalVideo> video);
		void RemoveVideo(VGSSVideoObjectID id);
		[[nodiscard]] GalGame::IGalVideo* GetVideo(VGSSVideoObjectID id) const;
		[[nodiscard]] bool HasVideo(VGSSVideoObjectID id) const;

		// ---------------------------------------------------------------------
		// 泛型 Registry API（与需求文档 Register / Unregister / Find / Exists / Clear / Iterate 对齐）
		// ---------------------------------------------------------------------
		void UnregisterCharacter(VGSSCharacterObjectID id) { RemoveCharacter(id); }
		void UnregisterSprite(VGSSSpriteObjectID id) { RemoveSprite(id); }
		void UnregisterAudio(VGSSAudioObjectID id) { RemoveAudio(id); }
		void UnregisterVideo(VGSSVideoObjectID id) { RemoveVideo(id); }

		[[nodiscard]] GalGame::IGalCharacter* FindCharacter(VGSSCharacterObjectID id) const { return GetCharacter(id); }
		[[nodiscard]] GalGame::IGalSprite* FindSprite(VGSSSpriteObjectID id) const { return GetSprite(id); }
		[[nodiscard]] GalGame::IGalAudio* FindAudio(VGSSAudioObjectID id) const { return GetAudio(id); }
		[[nodiscard]] GalGame::IGalVideo* FindVideo(VGSSVideoObjectID id) const { return GetVideo(id); }

		[[nodiscard]] bool ExistsCharacter(VGSSCharacterObjectID id) const { return HasCharacter(id); }
		[[nodiscard]] bool ExistsSprite(VGSSSpriteObjectID id) const { return HasSprite(id); }
		[[nodiscard]] bool ExistsAudio(VGSSAudioObjectID id) const { return HasAudio(id); }
		[[nodiscard]] bool ExistsVideo(VGSSVideoObjectID id) const { return HasVideo(id); }

		void Clear();

		/// @warning 回调执行期间持有内部互斥锁：请勿在回调内再次调用 Register / Remove / Clear / SetMetadata，
		///          除非上层改用 std::recursive_mutex（默认实现为非递归 std::mutex）。
		void ForEachCharacter(
			const std::function<void(VGSSCharacterObjectID id, const std::shared_ptr<GalGame::IGalCharacter>&)>& fn) const;
		void ForEachSprite(
			const std::function<void(VGSSSpriteObjectID id, const std::shared_ptr<GalGame::IGalSprite>&)>& fn) const;
		void ForEachAudio(
			const std::function<void(VGSSAudioObjectID id, const std::shared_ptr<GalGame::IGalAudio>&)>& fn) const;
		void ForEachVideo(
			const std::function<void(VGSSVideoObjectID id, const std::shared_ptr<GalGame::IGalVideo>&)>& fn) const;
		void ForEachObjectHandle(const std::function<void(const VGSSRuntimeObjectHandle&)>& fn) const;

		// ---------------------------------------------------------------------
		// Editor / 绑定辅助：以 shared_ptr 延长生命周期（避免 Undo 窗口内的悬空裸指针）
		// ---------------------------------------------------------------------
		[[nodiscard]] std::shared_ptr<GalGame::IGalCharacter> TryGetCharacterShared(VGSSCharacterObjectID id) const;
		[[nodiscard]] std::shared_ptr<GalGame::IGalSprite> TryGetSpriteShared(VGSSSpriteObjectID id) const;
		[[nodiscard]] std::shared_ptr<GalGame::IGalAudio> TryGetAudioShared(VGSSAudioObjectID id) const;
		[[nodiscard]] std::shared_ptr<GalGame::IGalVideo> TryGetVideoShared(VGSSVideoObjectID id) const;

		// ---------------------------------------------------------------------
		// Layer 索引（Stage / Timeline Track / Audio Bus 等的运行时镜像）
		// ---------------------------------------------------------------------
		/// 返回拷贝以避免在释放互斥锁后仍持有对内部容器的悬空引用。
		[[nodiscard]] std::vector<VGSSRuntimeObjectHandle> GetObjectsInLayer(const std::string& layer) const;
		void RemoveLayer(const std::string& layer);
		void ClearLayer(const std::string& layer);

		// ---------------------------------------------------------------------
		// Metadata
		// ---------------------------------------------------------------------
		void SetMetadata(VGSSRuntimeObjectHandle handle, VGSSObjectMetadata meta);
		[[nodiscard]] bool GetMetadata(VGSSRuntimeObjectHandle handle, VGSSObjectMetadata& outMeta) const;

		/// 显式同步：若 IGalGameResource 已有 Layer，可调用此方法把 Metadata.Layer 写回索引。
		void RefreshLayerFromResource(VGSSRuntimeObjectHandle handle);

		// ---------------------------------------------------------------------
		// 组件访问（调试 / 单元测试）
		// ---------------------------------------------------------------------
		[[nodiscard]] const VGSSObjectIDGenerator& GetIdGenerator() const noexcept { return m_IdGenerator; }
		[[nodiscard]] VGSSObjectIDGenerator& GetIdGenerator() noexcept { return m_IdGenerator; }

	private:
		[[nodiscard]] static bool ValidateId(unsigned int id) noexcept { return id != VGSS_INVALID_OBJECT_ID; }
		//[[nodiscard]] static bool ValidateId(VGSSCharacterObjectID id) noexcept { return id != VGSS_INVALID_OBJECT_ID; }
		//[[nodiscard]] static bool ValidateId(VGSSTextureObjectID id) noexcept { return id != VGSS_INVALID_OBJECT_ID; }
		//[[nodiscard]] static bool ValidateId(VGSSAudioObjectID id) noexcept { return id != VGSS_INVALID_OBJECT_ID; }
		//[[nodiscard]] static bool ValidateId(VGSSVideoObjectID id) noexcept { return id != VGSS_INVALID_OBJECT_ID; }

		[[nodiscard]] static VGSSRuntimeObjectHandle MakeHandle(VGSSRuntimeObjectKind kind, std::uint32_t rawId) noexcept;

		void RemoveHandleFromLayerIndexLocked(const VGSSRuntimeObjectHandle& handle, const std::string& layer);
		void AddHandleToLayerIndexLocked(const VGSSRuntimeObjectHandle& handle, const std::string& layer);

		/// 调用方已持有 m_Mutex。
		[[nodiscard]] bool IsHandleRegisteredLocked(const VGSSRuntimeObjectHandle& handle) const noexcept;

		void EraseMetadataAndLayerLocked(const VGSSRuntimeObjectHandle& handle);

	private:
		mutable std::mutex m_Mutex;

		VGSSObjectIDGenerator m_IdGenerator;

		std::unordered_map<VGSSCharacterObjectID, std::shared_ptr<GalGame::IGalCharacter>> m_Characters;
		std::unordered_map<VGSSSpriteObjectID, std::shared_ptr<GalGame::IGalSprite>> m_Sprites;
		std::unordered_map<VGSSAudioObjectID, std::shared_ptr<GalGame::IGalAudio>> m_Audios;
		std::unordered_map<VGSSVideoObjectID, std::shared_ptr<GalGame::IGalVideo>> m_Videos;

		std::unordered_map<std::string, std::vector<VGSSRuntimeObjectHandle>> m_LayerToObjects;

		std::unordered_map<VGSSRuntimeObjectHandle, VGSSObjectMetadata, VGSSRuntimeObjectHandleHash> m_Metadata;
	};
}
