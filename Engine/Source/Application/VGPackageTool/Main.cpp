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

#include "Include/VGPackageApp.h"
#include <VGCore/Include/Core/Core.h>
#include <VGCore/Include/Core/VFS.h>
#include <VGEngine/Include/Engine/VGEngine.h>
#include <VGEditorFramework/Include/EditorCore/EditorCore.h>

/// @brief 表示程序虚拟文件系统（VFS）中各类资源和设置的路径。
struct ApplicationVFSPath
{
	std::string editor;
	std::string engine;
};

/// @brief 初始化虚拟文件系统（VFS）。
/// @param path 路径结构体，包含桌面应用程序和引擎的资源路径。
void InitializeVFS(ApplicationVFSPath path)
{
	auto& vfs = VisionGal::VFS::GetInstance();

	// 添加桌面应用程序资源路径
	auto editorFS = std::make_unique<vfspp::NativeFileSystem>(path.editor);
	auto engineFS = std::make_unique<vfspp::NativeFileSystem>(path.engine);

	editorFS->Initialize();
	engineFS->Initialize();

	vfs->AddFileSystem(VisionGal::Editor::EditorCore::GetEditorResourcePathVFS(), std::move(editorFS));
	vfs->AddFileSystem(VisionGal::Core::GetEngineResourcePathVFS(), std::move(engineFS));
}

int main(int argc, char* argv[])
{
	using namespace VisionGal;

	std::string editorProjectRootDir;
#ifdef EDITOR_PROJECT_ROOT_DIR
	std::cout << "[Engine project root path]: " << EDITOR_PROJECT_ROOT_DIR << std::endl;
	editorProjectRootDir = EDITOR_PROJECT_ROOT_DIR;
#endif

	// 设置全局为UTF-8模式
	std::locale::global(std::locale(".utf8"));  // C++标准库使用UTF-8

	// 项目根目录
	std::string projectRootDir = editorProjectRootDir + "/Projects/New Project";

	// 初始化文件系统
	ApplicationVFSPath paths;
	paths.editor = editorProjectRootDir + "/Resource/Editor/";
	paths.engine = editorProjectRootDir + "/Resource/Engine/";
	InitializeVFS(paths);

	// 初始化打包系统应用
	Ref<Editor::VGPackageApp> application = CreateRef<Editor::VGPackageApp>();
	application->Initialize();
	VGEngine::Get()->AddApplication(application);

	// 初始化编辑器面板
	application->InitializeEditorPanels();

	// 运行引擎
	VGEngine::Get()->Run();
}