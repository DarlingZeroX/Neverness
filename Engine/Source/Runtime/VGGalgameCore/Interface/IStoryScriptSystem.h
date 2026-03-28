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

#pragma once
#include <string>

namespace VisionGal::GalGame
{
	struct IStoryScriptSystem
	{
	public:
		// 剧情脚本相关接口
		virtual bool ReloadStoryScript() = 0;	/// 重新加载剧情脚本
		virtual bool LoadStoryScript(const std::string& path) = 0;	/// 加载指定路径的剧情脚本
		virtual bool LoadStoryScriptOnUpdate(const std::string& path) = 0;	/// 在更新时加载指定路径的剧情脚本

		virtual std::string GetCurrentStoryScriptPath() = 0;
		virtual std::filesystem::file_time_type GetScriptLastWriteTime() const = 0;

		virtual void DoChoice(const std::string& id, const std::vector<std::string>& options) = 0;	/// 处理剧情选择事件。
		virtual void DoInput(const std::string& id, const std::string& title, const std::string& button) = 0;	/// 处理输入事件。

		virtual bool LoadSceneStoryScript(IScene* scene) = 0;
		virtual bool LoadSceneStoryScriptOnUpdate(IScene* scene) = 0;

		virtual void Wait(float duration) = 0;	/// 等待指定的时间长度。

		virtual bool LoadArchive(const SaveArchive& archive) = 0;
	};
}