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

#include "ApplicationInitializer.h"
#include "Include/VGDesktopApplication.h"
#include <VGEngine/Include/Engine/VGEngine.h>
#include "HCore/Include/System/HFileSystem.h"

 /// @brief 程序入口点。Windows平台使用WinMain，其他平台使用main。
#ifdef _WIN32
#include <Windows.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main()
#endif
{
	using namespace VisionGal;

	std::string editorProjectRootDir;
#ifdef EDITOR_PROJECT_ROOT_DIR
	std::cout << "Engine Project root is: " << EDITOR_PROJECT_ROOT_DIR << std::endl;
	editorProjectRootDir = EDITOR_PROJECT_ROOT_DIR;
#endif

	// 设置全局为UTF-8模式
	std::locale::global(std::locale(".utf8"));  // C++标准库使用UTF-8

	// 读取项目根目录
	std::string projectRootDir = editorProjectRootDir + "/Projects/Test Project";

	Horizon::HFileSystem::CreateDirectoryWhenNoExist("Intermediate");

	// 初始化文件系统
	VGDesktopApplicationVFSPath paths;
	paths.assets = projectRootDir + "/Assets/";
	paths.projectSettings = projectRootDir + "/ProjectSettings/";
	paths.editor = editorProjectRootDir + "/Resource/Editor/";
	paths.engine = editorProjectRootDir + "/Resource/Engine/";
	paths.projectIntermediate = "Intermediate/";
	VGDesktopApplicationInitializer::InitializeVFS(paths);

	// 加载项目
	VGEngine::Get()->LoadProject();

	// 初始化桌面应用程序
	Ref<Editor::VGDesktopApplication> application = CreateRef<Editor::VGDesktopApplication>();
	application->Initialize();

	// 初始化编辑器面板
	application->InitializeEditorPanels();

	// 加载项目主场景
	VGEngine::Get()->LoadProjectMainScene();

	// 运行场景
	application->RunScene();

	// 运行引擎
	VGEngine::Get()->AddApplication(application);
	VGEngine::Get()->Run();
}