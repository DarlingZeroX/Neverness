/*
 * SSExecutorResourceManager 实现
 */

#include "ExecutorResourceManager.h"

#include <algorithm>
#include <utility>

#include "VGGalgameCore/Interface/IGameObject.h"

namespace VisionGal
{
	namespace
	{
		[[nodiscard]] std::vector<VGSSRuntimeObjectHandle> CopyLayerHandles(
			const std::unordered_map<std::string, std::vector<VGSSRuntimeObjectHandle>>& layers,
			const std::string& layer)
		{
			const auto it = layers.find(layer);
			if (it == layers.end())
				return {};
			return it->second;
		}
	}

	SSExecutorResourceManager::SSExecutorResourceManager() = default;

	SSExecutorResourceManager::~SSExecutorResourceManager() = default;

	VGSSRuntimeObjectHandle SSExecutorResourceManager::MakeHandle(
		const VGSSRuntimeObjectKind kind,
		const std::uint32_t rawId) noexcept
	{
		return VGSSRuntimeObjectHandle{ kind, rawId };
	}

	bool SSExecutorResourceManager::IsHandleRegisteredLocked(const VGSSRuntimeObjectHandle& handle) const noexcept
	{
		switch (handle.Kind)
		{
		case VGSSRuntimeObjectKind::Character:
			return m_Characters.find(static_cast<VGSSCharacterObjectID>(handle.Id)) != m_Characters.end();
		case VGSSRuntimeObjectKind::Sprite:
			return m_Sprites.find(static_cast<VGSSSpriteObjectID>(handle.Id)) != m_Sprites.end();
		case VGSSRuntimeObjectKind::Audio:
			return m_Audios.find(static_cast<VGSSAudioObjectID>(handle.Id)) != m_Audios.end();
		case VGSSRuntimeObjectKind::Video:
			return m_Videos.find(static_cast<VGSSVideoObjectID>(handle.Id)) != m_Videos.end();
		default:
			return false;
		}
	}

	void SSExecutorResourceManager::RemoveHandleFromLayerIndexLocked(
		const VGSSRuntimeObjectHandle& handle,
		const std::string& layer)
	{
		if (!handle.IsValid() || layer.empty())
			return;

		const auto it = m_LayerToObjects.find(layer);
		if (it == m_LayerToObjects.end())
			return;

		std::vector<VGSSRuntimeObjectHandle>& vec = it->second;
		vec.erase(std::remove(vec.begin(), vec.end(), handle), vec.end());
		if (vec.empty())
			m_LayerToObjects.erase(it);
	}

	void SSExecutorResourceManager::AddHandleToLayerIndexLocked(
		const VGSSRuntimeObjectHandle& handle,
		const std::string& layer)
	{
		if (!handle.IsValid() || layer.empty())
			return;

		std::vector<VGSSRuntimeObjectHandle>& vec = m_LayerToObjects[layer];
		if (std::find(vec.begin(), vec.end(), handle) == vec.end())
			vec.push_back(handle);
	}

	void SSExecutorResourceManager::EraseMetadataAndLayerLocked(const VGSSRuntimeObjectHandle& handle)
	{
		const auto it = m_Metadata.find(handle);
		if (it == m_Metadata.end())
			return;

		RemoveHandleFromLayerIndexLocked(handle, it->second.Layer);
		m_Metadata.erase(it);
	}

	VGSSCharacterObjectID SSExecutorResourceManager::RegisterCharacter(std::shared_ptr<GalGame::IGalCharacter> character)
	{
		if (!character)
			return VGSS_INVALID_OBJECT_ID;

		std::lock_guard<std::mutex> lock(m_Mutex);
		const VGSSCharacterObjectID id = m_IdGenerator.GenerateCharacterID();
		m_Characters.emplace(id, std::move(character));
		return id;
	}

	void SSExecutorResourceManager::RemoveCharacter(const VGSSCharacterObjectID id)
	{
		if (!ValidateId(id))
			return;

		std::lock_guard<std::mutex> lock(m_Mutex);
		const auto it = m_Characters.find(id);
		if (it == m_Characters.end())
			return;

		const VGSSRuntimeObjectHandle handle = MakeHandle(VGSSRuntimeObjectKind::Character, id);
		EraseMetadataAndLayerLocked(handle);
		m_Characters.erase(it);
	}

	GalGame::IGalCharacter* SSExecutorResourceManager::GetCharacter(const VGSSCharacterObjectID id) const
	{
		if (!ValidateId(id))
			return nullptr;

		std::lock_guard<std::mutex> lock(m_Mutex);
		const auto it = m_Characters.find(id);
		if (it == m_Characters.end())
			return nullptr;
		return it->second.get();
	}

	bool SSExecutorResourceManager::HasCharacter(const VGSSCharacterObjectID id) const
	{
		if (!ValidateId(id))
			return false;

		std::lock_guard<std::mutex> lock(m_Mutex);
		return m_Characters.find(id) != m_Characters.end();
	}

	VGSSSpriteObjectID SSExecutorResourceManager::RegisterSprite(std::shared_ptr<GalGame::IGalSprite> sprite)
	{
		if (!sprite)
			return VGSS_INVALID_OBJECT_ID;

		std::lock_guard<std::mutex> lock(m_Mutex);
		const VGSSSpriteObjectID id = m_IdGenerator.GenerateSpriteID();
		m_Sprites.emplace(id, std::move(sprite));
		return id;
	}

	void SSExecutorResourceManager::RemoveSprite(const VGSSSpriteObjectID id)
	{
		if (!ValidateId(id))
			return;

		std::lock_guard<std::mutex> lock(m_Mutex);
		const auto it = m_Sprites.find(id);
		if (it == m_Sprites.end())
			return;

		const VGSSRuntimeObjectHandle handle = MakeHandle(VGSSRuntimeObjectKind::Sprite, id);
		EraseMetadataAndLayerLocked(handle);
		m_Sprites.erase(it);
	}

	GalGame::IGalSprite* SSExecutorResourceManager::GetSprite(const VGSSSpriteObjectID id) const
	{
		if (!ValidateId(id))
			return nullptr;

		std::lock_guard<std::mutex> lock(m_Mutex);
		const auto it = m_Sprites.find(id);
		if (it == m_Sprites.end())
			return nullptr;
		return it->second.get();
	}

	bool SSExecutorResourceManager::HasSprite(const VGSSSpriteObjectID id) const
	{
		if (!ValidateId(id))
			return false;

		std::lock_guard<std::mutex> lock(m_Mutex);
		return m_Sprites.find(id) != m_Sprites.end();
	}

	VGSSAudioObjectID SSExecutorResourceManager::RegisterAudio(std::shared_ptr<GalGame::IGalAudio> audio)
	{
		if (!audio)
			return VGSS_INVALID_OBJECT_ID;

		std::lock_guard<std::mutex> lock(m_Mutex);
		const VGSSAudioObjectID id = m_IdGenerator.GenerateAudioID();
		m_Audios.emplace(id, std::move(audio));
		return id;
	}

	void SSExecutorResourceManager::RemoveAudio(const VGSSAudioObjectID id)
	{
		if (!ValidateId(id))
			return;

		std::lock_guard<std::mutex> lock(m_Mutex);
		const auto it = m_Audios.find(id);
		if (it == m_Audios.end())
			return;

		const VGSSRuntimeObjectHandle handle = MakeHandle(VGSSRuntimeObjectKind::Audio, id);
		EraseMetadataAndLayerLocked(handle);
		m_Audios.erase(it);
	}

	GalGame::IGalAudio* SSExecutorResourceManager::GetAudio(const VGSSAudioObjectID id) const
	{
		if (!ValidateId(id))
			return nullptr;

		std::lock_guard<std::mutex> lock(m_Mutex);
		const auto it = m_Audios.find(id);
		if (it == m_Audios.end())
			return nullptr;
		return it->second.get();
	}

	bool SSExecutorResourceManager::HasAudio(const VGSSAudioObjectID id) const
	{
		if (!ValidateId(id))
			return false;

		std::lock_guard<std::mutex> lock(m_Mutex);
		return m_Audios.find(id) != m_Audios.end();
	}

	VGSSVideoObjectID SSExecutorResourceManager::RegisterVideo(std::shared_ptr<GalGame::IGalVideo> video)
	{
		if (!video)
			return VGSS_INVALID_OBJECT_ID;

		std::lock_guard<std::mutex> lock(m_Mutex);
		const VGSSVideoObjectID id = m_IdGenerator.GenerateVideoID();
		m_Videos.emplace(id, std::move(video));
		return id;
	}

	void SSExecutorResourceManager::RemoveVideo(const VGSSVideoObjectID id)
	{
		if (!ValidateId(id))
			return;

		std::lock_guard<std::mutex> lock(m_Mutex);
		const auto it = m_Videos.find(id);
		if (it == m_Videos.end())
			return;

		const VGSSRuntimeObjectHandle handle = MakeHandle(VGSSRuntimeObjectKind::Video, id);
		EraseMetadataAndLayerLocked(handle);
		m_Videos.erase(it);
	}

	GalGame::IGalVideo* SSExecutorResourceManager::GetVideo(const VGSSVideoObjectID id) const
	{
		if (!ValidateId(id))
			return nullptr;

		std::lock_guard<std::mutex> lock(m_Mutex);
		const auto it = m_Videos.find(id);
		if (it == m_Videos.end())
			return nullptr;
		return it->second.get();
	}

	bool SSExecutorResourceManager::HasVideo(const VGSSVideoObjectID id) const
	{
		if (!ValidateId(id))
			return false;

		std::lock_guard<std::mutex> lock(m_Mutex);
		return m_Videos.find(id) != m_Videos.end();
	}

	void SSExecutorResourceManager::Clear()
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		m_Characters.clear();
		m_Sprites.clear();
		m_Audios.clear();
		m_Videos.clear();
		m_LayerToObjects.clear();
		m_Metadata.clear();
		// 有意不重置 m_IdGenerator：防止 Clear 后立即复用旧 ID，与序列化句柄 / 外部缓存发生碰撞。
	}

	void SSExecutorResourceManager::ForEachCharacter(
		const std::function<void(VGSSCharacterObjectID id, const std::shared_ptr<GalGame::IGalCharacter>&)>& fn) const
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		for (const auto& kv : m_Characters)
			fn(kv.first, kv.second);
	}

	void SSExecutorResourceManager::ForEachSprite(
		const std::function<void(VGSSSpriteObjectID id, const std::shared_ptr<GalGame::IGalSprite>&)>& fn) const
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		for (const auto& kv : m_Sprites)
			fn(kv.first, kv.second);
	}

	void SSExecutorResourceManager::ForEachAudio(
		const std::function<void(VGSSAudioObjectID id, const std::shared_ptr<GalGame::IGalAudio>&)>& fn) const
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		for (const auto& kv : m_Audios)
			fn(kv.first, kv.second);
	}

	void SSExecutorResourceManager::ForEachVideo(
		const std::function<void(VGSSVideoObjectID id, const std::shared_ptr<GalGame::IGalVideo>&)>& fn) const
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		for (const auto& kv : m_Videos)
			fn(kv.first, kv.second);
	}

	void SSExecutorResourceManager::ForEachObjectHandle(
		const std::function<void(const VGSSRuntimeObjectHandle&)>& fn) const
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		for (const auto& kv : m_Characters)
			fn(MakeHandle(VGSSRuntimeObjectKind::Character, kv.first));
		for (const auto& kv : m_Sprites)
			fn(MakeHandle(VGSSRuntimeObjectKind::Sprite, kv.first));
		for (const auto& kv : m_Audios)
			fn(MakeHandle(VGSSRuntimeObjectKind::Audio, kv.first));
		for (const auto& kv : m_Videos)
			fn(MakeHandle(VGSSRuntimeObjectKind::Video, kv.first));
	}

	std::shared_ptr<GalGame::IGalCharacter> SSExecutorResourceManager::TryGetCharacterShared(
		const VGSSCharacterObjectID id) const
	{
		if (!ValidateId(id))
			return {};

		std::lock_guard<std::mutex> lock(m_Mutex);
		const auto it = m_Characters.find(id);
		if (it == m_Characters.end())
			return {};
		return it->second;
	}

	std::shared_ptr<GalGame::IGalSprite> SSExecutorResourceManager::TryGetSpriteShared(const VGSSSpriteObjectID id) const
	{
		if (!ValidateId(id))
			return {};

		std::lock_guard<std::mutex> lock(m_Mutex);
		const auto it = m_Sprites.find(id);
		if (it == m_Sprites.end())
			return {};
		return it->second;
	}

	std::shared_ptr<GalGame::IGalAudio> SSExecutorResourceManager::TryGetAudioShared(const VGSSAudioObjectID id) const
	{
		if (!ValidateId(id))
			return {};

		std::lock_guard<std::mutex> lock(m_Mutex);
		const auto it = m_Audios.find(id);
		if (it == m_Audios.end())
			return {};
		return it->second;
	}

	std::shared_ptr<GalGame::IGalVideo> SSExecutorResourceManager::TryGetVideoShared(const VGSSVideoObjectID id) const
	{
		if (!ValidateId(id))
			return {};

		std::lock_guard<std::mutex> lock(m_Mutex);
		const auto it = m_Videos.find(id);
		if (it == m_Videos.end())
			return {};
		return it->second;
	}

	std::vector<VGSSRuntimeObjectHandle> SSExecutorResourceManager::GetObjectsInLayer(const std::string& layer) const
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		return CopyLayerHandles(m_LayerToObjects, layer);
	}

	void SSExecutorResourceManager::RemoveLayer(const std::string& layer)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		const auto it = m_LayerToObjects.find(layer);
		if (it == m_LayerToObjects.end())
			return;

		for (const VGSSRuntimeObjectHandle& h : it->second)
		{
			const auto metaIt = m_Metadata.find(h);
			if (metaIt != m_Metadata.end())
				metaIt->second.Layer.clear();
		}

		m_LayerToObjects.erase(it);
	}

	void SSExecutorResourceManager::ClearLayer(const std::string& layer)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		const auto it = m_LayerToObjects.find(layer);
		if (it == m_LayerToObjects.end())
			return;

		for (const VGSSRuntimeObjectHandle& h : it->second)
		{
			const auto metaIt = m_Metadata.find(h);
			if (metaIt != m_Metadata.end())
				metaIt->second.Layer.clear();
		}

		it->second.clear();
	}

	void SSExecutorResourceManager::SetMetadata(const VGSSRuntimeObjectHandle handle, VGSSObjectMetadata meta)
	{
		if (!handle.IsValid())
			return;

		std::lock_guard<std::mutex> lock(m_Mutex);
		if (!IsHandleRegisteredLocked(handle))
			return;

		std::string oldLayer;
		const auto existing = m_Metadata.find(handle);
		if (existing != m_Metadata.end())
			oldLayer = existing->second.Layer;

		const std::string& newLayer = meta.Layer;
		if (oldLayer != newLayer)
		{
			RemoveHandleFromLayerIndexLocked(handle, oldLayer);
			AddHandleToLayerIndexLocked(handle, newLayer);
		}

		m_Metadata.insert_or_assign(handle, std::move(meta));
	}

	bool SSExecutorResourceManager::GetMetadata(const VGSSRuntimeObjectHandle handle, VGSSObjectMetadata& outMeta) const
	{
		if (!handle.IsValid())
			return false;

		std::lock_guard<std::mutex> lock(m_Mutex);
		const auto it = m_Metadata.find(handle);
		if (it == m_Metadata.end())
			return false;

		outMeta = it->second;
		return true;
	}

	void SSExecutorResourceManager::RefreshLayerFromResource(const VGSSRuntimeObjectHandle handle)
	{
		if (!handle.IsValid())
			return;

		std::lock_guard<std::mutex> lock(m_Mutex);
		if (!IsHandleRegisteredLocked(handle))
			return;

		std::string layerFromResource;
		switch (handle.Kind)
		{
		case VGSSRuntimeObjectKind::Sprite:
			if (const auto it = m_Sprites.find(static_cast<VGSSSpriteObjectID>(handle.Id)); it != m_Sprites.end())
				layerFromResource = it->second->GetResourceLayer();
			break;
		case VGSSRuntimeObjectKind::Audio:
			if (const auto it = m_Audios.find(static_cast<VGSSAudioObjectID>(handle.Id)); it != m_Audios.end())
				layerFromResource = it->second->GetResourceLayer();
			break;
		case VGSSRuntimeObjectKind::Video:
			if (const auto it = m_Videos.find(static_cast<VGSSVideoObjectID>(handle.Id)); it != m_Videos.end())
				layerFromResource = it->second->GetResourceLayer();
			break;
		case VGSSRuntimeObjectKind::Character:
			// IGalCharacter 非 IGalGameResource，图层仅来源于 Metadata / 上层绑定。
			return;
		default:
			return;
		}

		VGSSObjectMetadata meta;
		if (const auto metaIt = m_Metadata.find(handle); metaIt != m_Metadata.end())
			meta = metaIt->second;

		const std::string oldLayer = meta.Layer;
		meta.Layer = std::move(layerFromResource);

		if (oldLayer != meta.Layer)
		{
			RemoveHandleFromLayerIndexLocked(handle, oldLayer);
			AddHandleToLayerIndexLocked(handle, meta.Layer);
		}

		m_Metadata.insert_or_assign(handle, std::move(meta));
	}
}
