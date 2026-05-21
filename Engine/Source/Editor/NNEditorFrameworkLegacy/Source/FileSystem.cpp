/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
 * GitHub page: https://github.com/DarlingZeroX/VisionGal
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "FileSystem.h"
#include <NNRuntimeImGui/IncludeImGui.h>
#include <NNRuntimeImGui/Include/ImGuiEx/IconFont/IconsFontAwesome5Pro.h>

namespace NN::Editor
{
	EditorFileSystem::EditorFileSystem(const NN::Core::fsPath& path)
	{
		m_ProjectDirectory = path;
		OpenRootDirectory();
	}

	void EditorFileSystem::OpenRootDirectory()
	{
		m_RootNode.Directories.clear();
		m_RootNode.name = "Content";
		m_RootNode.path = m_ProjectDirectory;
		m_RootNode.uiFlags = ImGuiTreeNodeFlags_SpanFullWidth;

		RefreshDirectoryFile(m_RootNode);
	}

	void EditorFileSystem::RefreshDirectoryFile(EditorFileSystemDirectory& node)
	{
		auto path = node.path;
		node.needOpen = false;

		for (auto& entry : std::filesystem::directory_iterator(path))
		{
			pfsPath relativePath = NN::Core::HFileSystem::RelativePath(entry.path(), m_ProjectDirectory);

			if (entry.is_directory())
			{
				EditorFileSystemDirectory directory;

				int uiFlags = ImGuiTreeNodeFlags_SpanFullWidth;
				if (entry == m_CurrentDirectoryOrFile)
				{
					uiFlags = uiFlags | ImGuiTreeNodeFlags_Selected;
				}

				// Ŀ¼�ǿյ�
				if (NN::Core::HFileSystem::DirectoryEmpty(entry))
				{
					uiFlags |= ImGuiTreeNodeFlags_Leaf;
				}

				std::string nodeName = ICON_FA_FOLDER " ";
				directory.name += relativePath.filename().string();
				directory.uiFlags = uiFlags;
				directory.path = entry.path();

				node.Directories.emplace_back(directory);
			}
			else
			{
				EditorFileSystemFile file;

				file.path = entry.path();
				file.pathStr = file.path.string();
				file.ext = file.path.extension().string();
				auto relativePath = std::filesystem::relative(file.path, path);
				file.filename = relativePath.filename().string();
				file.uiFlags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_Leaf;

				node.Files.emplace_back(file);
			}
		}
	}

	FileSystemPanel::FileSystemPanel()
	{
		m_pContentBrowser = ContentBrowser::GetInstancePtr();
		m_ContentPath = m_pContentBrowser->GetProjectDirectory();
		m_EditorContentBrowser = MakeRef<EditorFileSystem>(m_ContentPath);
	}

	FileSystemPanel::FileSystemPanel(const Runtime::String& contentPath)
	{
		m_ContentPath = contentPath;
	}

	void FileSystemPanel::OnGUI()
	{
		if (ImGui::Begin("ContentBrowser"))
		{
			DrawContentBrowser();
		}
		ImGui::End();
	}

	void FileSystemPanel::DrawContentBrowser()
	{
		//��Ŀ¼��֧
		if (ImGui::CollapsingHeader("Content", ImGuiTreeNodeFlags_DefaultOpen))
		{

			for (auto& directory : m_EditorContentBrowser->GetRootNode().Directories)
			{
				DrawDirectoryTree(directory);
			}

			for (auto& file : m_EditorContentBrowser->GetRootNode().Files)
			{
				DrawFile(file);
			}
			//DrawDirectoryTree(m_DirectoryTreeRootNode);
		}
	}

	void FileSystemPanel::DrawDirectoryTree(EditorFileSystemDirectory& node)
	{
		std::string nodeName = ICON_FA_FOLDER;
		nodeName += " ";
		nodeName += node.name;

		if (ImGui::TreeNodeEx(nodeName.c_str(), node.uiFlags))
		{
			if (node.needOpen)
			{
				m_EditorContentBrowser->RefreshDirectoryFile(node);
			}

			for (auto& child : node.Directories)
			{
				DrawDirectoryTree(child);
			}

			for (auto& child : node.Files)
			{
				DrawFile(child);
			}

			ImGui::TreePop();
		}
	}

	void FileSystemPanel::DrawFile(EditorFileSystemFile& file)
	{
		if (ImGui::TreeNodeEx(file.filename.c_str(), file.uiFlags))
		{
			ImGui::TreePop();
		}
	}
}
