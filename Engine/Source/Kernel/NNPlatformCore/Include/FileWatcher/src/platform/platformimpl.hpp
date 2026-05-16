#ifndef EFSW_PLATFORMIMPL_HPP
#define EFSW_PLATFORMIMPL_HPP

#include "base.hpp"

#if defined( EFSW_PLATFORM_POSIX )
#include "platform/posix/ThreadImpl.hpp"
#include "platform/posix/MutexImpl.hpp"
#include "platform/posix/SystemImpl.hpp"
#include "platform/posix/FileSystemImpl.hpp"
#elif EFSW_PLATFORM == EFSW_PLATFORM_WIN32
#include "platform/win/ThreadImpl.hpp"
#include "platform/win/MutexImpl.hpp"
#include "platform/win/SystemImpl.hpp"
#include "platform/win/FileSystemImpl.hpp"
#else
#error Thread, Mutex, and System not implemented for this platform.
#endif

#endif