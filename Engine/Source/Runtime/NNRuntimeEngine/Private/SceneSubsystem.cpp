#include "NNRuntimeEngine/SceneSubsystem.h"

#include <algorithm>
#include <cstring>

namespace visiongal::engine
{
VGTransform3 SceneSubsystem::DefaultTransform() noexcept
{
	VGTransform3 t{};
	t.position[0] = t.position[1] = t.position[2] = 0.f;
	t.rotation[0] = t.rotation[1] = t.rotation[2] = 0.f;
	t.scale[0] = t.scale[1] = t.scale[2] = 1.f;
	return t;
}

VGEntityHandle SceneSubsystem::AllocateEntity() noexcept
{
	const VGEntityHandle id = nextEntity_.fetch_add(1u, std::memory_order_relaxed);
	if (id == 0)
	{
		return 0;
	}
	Entity e{};
	e.id = id;
	e.transform = DefaultTransform();
	e.alive = true;
	entities_.emplace(id, std::move(e));
	return id;
}

void SceneSubsystem::RemoveFromParent(VGEntityHandle child) noexcept
{
	const auto itChild = entities_.find(child);
	if (itChild == entities_.end())
	{
		return;
	}
	const VGEntityHandle p = itChild->second.parent;
	if (p == 0)
	{
		return;
	}
	const auto itP = entities_.find(p);
	if (itP == entities_.end())
	{
		return;
	}
	auto& ch = itP->second.children;
	ch.erase(std::remove(ch.begin(), ch.end(), child), ch.end());
	itChild->second.parent = 0;
}

int SceneSubsystem::LoadScene(const char* sceneNameUtf8) noexcept
{
	std::lock_guard<std::mutex> lock(mutex_);
	activeScene_ = (sceneNameUtf8 != nullptr) ? sceneNameUtf8 : "";
	return 1;
}

VGEntityHandle SceneSubsystem::Spawn(const char* prefabVirtualPathUtf8) noexcept
{
	(void)prefabVirtualPathUtf8;
	std::lock_guard<std::mutex> lock(mutex_);
	return AllocateEntity();
}

void SceneSubsystem::Destroy(VGEntityHandle entity) noexcept
{
	if (entity == 0)
	{
		return;
	}
	std::lock_guard<std::mutex> lock(mutex_);
	RemoveFromParent(entity);
	entities_.erase(entity);
}

VGEntityHandle SceneSubsystem::Find(const char* entityNameUtf8) noexcept
{
	if (entityNameUtf8 == nullptr)
	{
		return 0;
	}
	const std::string want(entityNameUtf8);
	std::lock_guard<std::mutex> lock(mutex_);
	for (const auto& p : entities_)
	{
		if (p.second.alive && p.second.name == want)
		{
			return p.first;
		}
	}
	return 0;
}

void SceneSubsystem::Activate(VGEntityHandle entity, int active) noexcept
{
	if (entity == 0)
	{
		return;
	}
	std::lock_guard<std::mutex> lock(mutex_);
	const auto it = entities_.find(entity);
	if (it != entities_.end())
	{
		it->second.active = active;
	}
}

int SceneSubsystem::UnloadScene(const char* sceneNameUtf8) noexcept
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (sceneNameUtf8 == nullptr)
	{
		return -1;
	}
	if (activeScene_ == sceneNameUtf8)
	{
		activeScene_.clear();
		entities_.clear();
		return 0;
	}
	return -1;
}

int SceneSubsystem::GetActiveSceneName(char* outUtf8, std::size_t outCapacity) const noexcept
{
	if (outUtf8 == nullptr || outCapacity == 0u)
	{
		return -1;
	}
	std::lock_guard<std::mutex> lock(mutex_);
	if (activeScene_.empty())
	{
		outUtf8[0] = '\0';
		return 0;
	}
	if (activeScene_.size() + 1u > outCapacity)
	{
		return -1;
	}
	std::memcpy(outUtf8, activeScene_.data(), activeScene_.size());
	outUtf8[activeScene_.size()] = '\0';
	return static_cast<int>(activeScene_.size());
}

void SceneSubsystem::SetParent(VGEntityHandle child, VGEntityHandle parent) noexcept
{
	if (child == 0)
	{
		return;
	}
	std::lock_guard<std::mutex> lock(mutex_);
	const auto itC = entities_.find(child);
	if (itC == entities_.end())
	{
		return;
	}
	RemoveFromParent(child);
	if (parent != 0)
	{
		const auto itP = entities_.find(parent);
		if (itP != entities_.end())
		{
			itP->second.children.push_back(child);
			itC->second.parent = parent;
		}
	}
}

VGEntityHandle SceneSubsystem::GetParent(VGEntityHandle entity) const noexcept
{
	if (entity == 0)
	{
		return 0;
	}
	std::lock_guard<std::mutex> lock(mutex_);
	const auto it = entities_.find(entity);
	return it != entities_.end() ? it->second.parent : 0;
}

std::uint32_t SceneSubsystem::GetChildCount(VGEntityHandle entity) const noexcept
{
	if (entity == 0)
	{
		return 0u;
	}
	std::lock_guard<std::mutex> lock(mutex_);
	const auto it = entities_.find(entity);
	return it != entities_.end() ? static_cast<std::uint32_t>(it->second.children.size()) : 0u;
}

VGEntityHandle SceneSubsystem::GetChildAt(VGEntityHandle entity, std::uint32_t index) const noexcept
{
	if (entity == 0)
	{
		return 0;
	}
	std::lock_guard<std::mutex> lock(mutex_);
	const auto it = entities_.find(entity);
	if (it == entities_.end() || index >= it->second.children.size())
	{
		return 0;
	}
	return it->second.children[static_cast<std::size_t>(index)];
}

void SceneSubsystem::GetTransform(VGEntityHandle entity, VGTransform3* outTransform) const noexcept
{
	if (outTransform == nullptr || entity == 0)
	{
		return;
	}
	std::lock_guard<std::mutex> lock(mutex_);
	const auto it = entities_.find(entity);
	if (it != entities_.end())
	{
		*outTransform = it->second.transform;
	}
	else
	{
		*outTransform = DefaultTransform();
	}
}

void SceneSubsystem::SetTransform(VGEntityHandle entity, const VGTransform3* transform) noexcept
{
	if (transform == nullptr || entity == 0)
	{
		return;
	}
	std::lock_guard<std::mutex> lock(mutex_);
	const auto it = entities_.find(entity);
	if (it != entities_.end())
	{
		it->second.transform = *transform;
	}
}

int SceneSubsystem::SetEntityName(VGEntityHandle entity, const char* nameUtf8) noexcept
{
	if (entity == 0)
	{
		return -1;
	}
	std::lock_guard<std::mutex> lock(mutex_);
	const auto it = entities_.find(entity);
	if (it == entities_.end())
	{
		return -1;
	}
	it->second.name = (nameUtf8 != nullptr) ? nameUtf8 : "";
	return 0;
}

int SceneSubsystem::GetEntityName(VGEntityHandle entity, char* outUtf8, std::size_t outCapacity) const noexcept
{
	if (entity == 0 || outUtf8 == nullptr || outCapacity == 0u)
	{
		return -1;
	}
	std::lock_guard<std::mutex> lock(mutex_);
	const auto it = entities_.find(entity);
	if (it == entities_.end())
	{
		return -1;
	}
	const std::string& n = it->second.name;
	if (n.size() + 1u > outCapacity)
	{
		return -1;
	}
	std::memcpy(outUtf8, n.data(), n.size());
	outUtf8[n.size()] = '\0';
	return static_cast<int>(n.size());
}
} // namespace visiongal::engine
