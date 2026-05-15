/*
 * GalScriptRuntimeRegistry 实现（Phase 8B）
 */

#include "ScriptSystem/GalScriptRuntimeRegistry.h"

namespace VisionGal::GalGame
{
	void GalScriptRuntimeRegistry::RegisterRuntime(Ref<IScriptRuntime> runtime)
	{
		if (runtime)
			m_Runtimes.push_back(std::move(runtime));
	}

	void GalScriptRuntimeRegistry::Clear() noexcept
	{
		m_Runtimes.clear();
	}

	IScriptRuntime* GalScriptRuntimeRegistry::FindRuntimeForPath(const String& path) const noexcept
	{
		for (auto it = m_Runtimes.rbegin(); it != m_Runtimes.rend(); ++it)
		{
			IScriptRuntime* rt = it->get();
			if (rt && rt->CanLoad(path))
				return rt;
		}
		return nullptr;
	}
}
