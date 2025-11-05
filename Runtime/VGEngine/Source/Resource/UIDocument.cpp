#include "Resource/UIDocument.h"

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
