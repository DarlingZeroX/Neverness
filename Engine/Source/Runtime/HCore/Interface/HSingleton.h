///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016-2021, Intel Corporation
//
// SPDX-License-Identifier: MIT
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Author(s):  Filip Strugar (filip.strugar@intel.com)
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include<vector>
#include<iostream>

// Frequently used includes
#include <assert.h>

namespace Horizon
{
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Simple base class for a singleton.
	//  - ensures that the class is indeed a singleton
	//  - provides access to it
	//  1.) inherit YourClass from HSingletonBase<YourClass>
	//  2.) you're responsible for creation/destruction of the object and its thread safety!
	//
	template< class SingletonType >
	class HSingletonBase
	{
	private:
		static SingletonType* s_instance;
		static bool                             s_instanceValid;

	protected:
#pragma optimize( "", off )
		HSingletonBase()
		{
			SingletonType* previous = s_instance;
			s_instance = reinterpret_cast<SingletonType*>(this);

			//assert( vaThreading::IsMainThread() );
			assert(previous == NULL);
			s_instanceValid = true;
		}
#pragma optimize( "", on )
		virtual ~HSingletonBase()
		{
		//	InvalidateInstance();
		//	delete s_instance;
		}
		void InvalidateInstance() { s_instanceValid = false; }

	public:

		static SingletonType& GetInstance() { SingletonType* retVal = s_instance; assert(retVal != NULL); return *retVal; }
		static SingletonType* GetInstancePtr() { return s_instance; }
		static bool                             GetInstanceValid() { return s_instanceValid; }
	};

#pragma optimize( "", off )
	template <class SingletonType>
	SingletonType* HSingletonBase<SingletonType>::s_instance = nullptr;
	template <class SingletonType>
	bool HSingletonBase<SingletonType>::s_instanceValid = false;
#pragma optimize( "", on )
	//////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////
	//// This is a simple class that gives each instance a new 64bit ID at construction.
	//// There is no tracking of any kind so the cost is minimal (one InterlockedAdd)
	template< class TrackableType >
	class HRuntimeID
	{
	private:
	    static std::atomic_uint64_t         s_runtimeIDCounter;
	    uint64 const                        m_runtimeID;
	protected:
		HRuntimeID() : m_runtimeID(s_runtimeIDCounter.fetch_add(1ull)) { }
	public:
	    uint64                              RuntimeIDGet() const noexcept { return m_runtimeID; }
	};
	template< class TrackableType >
	std::atomic_uint64_t HRuntimeID<TrackableType>::s_runtimeIDCounter = 0ull;

	////////////////////////////////////////////////////////////////////////////////////////////////
}