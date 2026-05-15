/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Workspace/SequenceWorkspaceState.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace VisionGal::Editor
{
	namespace
	{
		std::string DefaultWorkspacePath()
		{
#ifdef _WIN32
			char buf[MAX_PATH];
			const DWORD n = GetEnvironmentVariableA("APPDATA", buf, MAX_PATH);
			if (n > 0 && n < MAX_PATH)
				return std::string(buf) + "\\VisionGal\\SequenceEditorWorkspace.ini";
#endif
			return "SequenceEditorWorkspace.ini";
		}
	}

	SequenceWorkspaceState::SequenceWorkspaceState()
		: m_storagePath(DefaultWorkspacePath())
	{
		Load();
	}

	SequenceWorkspaceState::~SequenceWorkspaceState()
	{
		Save();
	}

	bool SequenceWorkspaceState::IsWindowVisible(const char* key) const
	{
		const auto it = m_visible.find(key);
		if (it == m_visible.end())
			return true;
		return it->second;
	}

	void SequenceWorkspaceState::SetWindowVisible(const char* key, const bool visible)
	{
		m_visible[key] = visible;
	}

	void SequenceWorkspaceState::Load()
	{
		m_visible.clear();
		std::ifstream in(m_storagePath.c_str());
		if (!in)
			return;
		std::string line;
		while (std::getline(in, line))
		{
			const auto eq = line.find('=');
			if (eq == std::string::npos)
				continue;
			const std::string k = line.substr(0, eq);
			const std::string v = line.substr(eq + 1);
			m_visible[k] = (v == "1" || v == "true");
		}
	}

	void SequenceWorkspaceState::Save() const
	{
		try
		{
			const std::filesystem::path p(m_storagePath);
			if (p.has_parent_path())
				std::filesystem::create_directories(p.parent_path());
			std::ofstream out(m_storagePath.c_str(), std::ios::trunc);
			if (!out)
				return;
			for (const auto& kv : m_visible)
				out << kv.first << '=' << (kv.second ? "1" : "0") << '\n';
		}
		catch (...)
		{
		}
	}
}
