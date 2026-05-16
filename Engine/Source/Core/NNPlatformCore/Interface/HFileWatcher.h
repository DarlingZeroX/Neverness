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
#include <memory>
#include <functional>
#include "../HCorePlatformConfig.h"

namespace NN::Core
{
	using HWatchID = long;

	enum class HFileActionType
	{
		/// Sent when a file is created or renamed
		Add = 1,
		/// Sent when a file is deleted or renamed
		Delete = 2,
		/// Sent when a file is modified
		Modified = 3,
		/// Sent when a file is moved
		Moved = 4
	};

	struct HFileActionData
	{
		HWatchID WatchID;
		const std::string& Directory;
		const std::string& Filename;
		HFileActionType action;
		std::string OldFilename = "";

		HFileActionData(
			const std::string& dir,
			const std::string& filename)
			:
			Directory(dir),
			Filename(filename)
		{}
	};

	class HWatchHandle;

	using HFileActionCallback = std::function<void(const HFileActionData&)>;

	/// Listens to files and directories and dispatches events
	/// to notify the listener of files and directories changes.
	/// @class FileWatcher
	class H_CORE_PLATFORM_API HFileWatcher
	{
		/// Default constructor, will use the default platform file watcher
		HFileWatcher();

		/// Constructor that lets you force the use of the Generic File Watcher
		explicit HFileWatcher(bool useGenericFileWatcher);
	public:

		virtual ~HFileWatcher();

		static HFileWatcher* GetInstance();

		/// Add a directory watch. Same as the other addWatch, but doesn't have recursive option.
		/// For backwards compatibility.
		/// On error returns WatchID with Error type.
		//HWatchID AddWatch(const std::string& directory, HFileWatchListener* watcher);

		/// Add a directory watch
		/// On error returns WatchID with Error type.
		//HWatchID AddWatch(const std::string& directory, HFileWatchListener* watcher, bool recursive);

		/// Add a directory watch. Same as the other addWatch, but doesn't have recursive option.
		/// For backwards compatibility.
		/// On error returns WatchID with Error type.
		HWatchID AddWatch(const std::string& directory, HFileActionCallback watcher);

		/// Add a directory watch
		/// On error returns WatchID with Error type.
		HWatchID AddWatch(const std::string& directory, HFileActionCallback watcher, bool recursive);

		/// Remove a directory watch. This is a brute force search O(nlogn).
		void RemoveWatch(const std::string& directory);

		/// Remove a directory watch. This is a map lookup O(logn).
		void RemoveWatch(HWatchID watchid);

		/// Starts watching ( in other thread )
		void Watch();

		/// @return Returns a list of the directories that are being watched
		std::list<std::string> Directories();

		/** Allow recursive watchers to follow symbolic links to other directories
		* followSymlinks is disabled by default
		*/
		void FollowSymlinks(bool follow);

		/** @return If can follow symbolic links to directorioes */
		const bool& FollowSymlinks() const;

		/** When enable this it will allow symlinks to watch recursively out of the pointed directory.
		* follorSymlinks must be enabled to this work.
		* For example, added symlink to /home/folder, and the symlink points to /, this by default is not allowed,
		* it's only allowed to symlink anything from /home/ and deeper. This is to avoid great levels of recursion.
		* Enabling this could lead in infinite recursion, and crash the watcher ( it will try not to avoid this ).
		* Buy enabling out of scope links, it will allow this behavior.
		* allowOutOfScopeLinks are disabled by default.
		*/
		void AllowOutOfScopeLinks(bool allow);

		/// @return Returns if out of scope links are allowed
		const bool& AllowOutOfScopeLinks() const;
	private:
		struct HFileWatcherImpl;

		struct HFileWatchListener;

		std::unique_ptr<HFileWatcherImpl> m_Impl;

		std::unordered_map<HWatchID, HFileWatchListener*> m_Listeners;
	};

	struct HWatchHandle
	{
		HWatchHandle() = default;

		HWatchHandle(HWatchID id)
			:
			ID(id)
		{
		}

		HWatchHandle& operator=(HWatchID id)
		{
			ID = id;
			return *this;
		}

		~HWatchHandle()
		{
			HFileWatcher::GetInstance()->RemoveWatch(ID);
		}

		HWatchID ID;
	};
}
