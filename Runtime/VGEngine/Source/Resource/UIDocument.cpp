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

#include "Resource/UIDocument.h"

#include "Engine/Manager/SceneManager.h"

namespace VisionGal
{
	RmlUIDocument::RmlUIDocument()
	{
	}

	RmlUIDocument::~RmlUIDocument()
	{
		Close();
	}

	void RmlUIDocument::Close()
	{
		if (document && !isClosed)
		{
			document->Close();
			isClosed = true;
		}

	}

	void RmlUIDocument::AddUpdateCallback(const sol::function& callback)
	{
		m_LuaUpdateCallbacks.push_back(callback);
	}

	void RmlUIDocument::Update()
	{
		if (SceneManager::Get()->IsPlayMode() == false)
			return;

		// 调用显示回调
		for (auto& callback : m_LuaUpdateCallbacks)
		{
			sol::protected_function_result res = callback();
			if (!res.valid()) {
				sol::error err = res;
				H_LOG_ERROR("%s", err.what());
			}
		}
	}
}
