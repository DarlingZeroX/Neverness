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

#include "pch.h"
#include "System/HFileWatcher.h"
#include <Platform/FileWatcher/include/efsw.hpp>

namespace Horizon
{
	struct HFileWatcher::HFileWatcherImpl : public efsw::FileWatcher
	{
	};

	/// Basic interface for listening for file events.
	/// @class FileWatchListener
	struct HFileWatcher::HFileWatchListener : public efsw::FileWatchListener
	{
	public:
		HFileWatchListener(HFileActionCallback callback)
			:m_Callback(callback)
		{}

		~HFileWatchListener() override {}

		/// Handles the action file action
		/// @param watchid The watch id for the directory
		/// @param dir The directory
		/// @param filename The filename that was accessed (not full path)
		/// @param action Action that was performed
		/// @param oldFilename The name of the file or directory moved
		void handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename,
			efsw::Action action, std::string oldFilename)
		{
			HFileActionData data(dir, filename);
			data.WatchID = watchid;
			data.action = static_cast<HFileActionType>(action);
			data.OldFilename = oldFilename;

			//std::cout << TypeOf(HFileActionType).GetEnumName(data.action) << std::endl;

			m_Callback(data);

			//return HandleFileAction(watchid, dir, filename, action, oldFilename);
		}

	private:
		HFileActionCallback m_Callback;
	};

	HFileWatcher::HFileWatcher()
	{
		m_Impl = std::make_unique<HFileWatcherImpl>();
	}

	HFileWatcher::HFileWatcher(bool useGenericFileWatcher)
		:HFileWatcher()
	{
	}

	HFileWatcher::~HFileWatcher()
	{
	}

	HFileWatcher* HFileWatcher::GetInstance()
	{
		static HFileWatcher* s_instance = new HFileWatcher();

		return s_instance;
	}

	HWatchID HFileWatcher::AddWatch(const std::string& directory, HFileActionCallback watcher)
	{
		auto* listener = new HFileWatchListener(watcher);

		const auto watchID = m_Impl->addWatch(directory, listener);

		return watchID;
	}

	HWatchID HFileWatcher::AddWatch(const std::string& directory, HFileActionCallback watcher, bool recursive)
	{
		auto* listener = new HFileWatchListener(watcher);

		const auto watchID = m_Impl->addWatch(directory, listener, recursive);

		return watchID;
	}

	void HFileWatcher::RemoveWatch(const std::string& directory)
	{
		m_Impl->removeWatch(directory);
	}

	void HFileWatcher::RemoveWatch(HWatchID watchid)
	{
		m_Impl->removeWatch(watchid);
	}

	void HFileWatcher::Watch()
	{
		m_Impl->watch();
	}

	std::list<std::string> HFileWatcher::Directories()
	{
		return m_Impl->directories();
	}

	void HFileWatcher::FollowSymlinks(bool follow)
	{
		return m_Impl->followSymlinks(follow);
	}

	const bool& HFileWatcher::FollowSymlinks() const
	{
		return m_Impl->followSymlinks();
	}

	void HFileWatcher::AllowOutOfScopeLinks(bool allow)
	{
		return m_Impl->allowOutOfScopeLinks(allow);
	}

	const bool& HFileWatcher::AllowOutOfScopeLinks() const
	{
		return m_Impl->allowOutOfScopeLinks();
	}
}