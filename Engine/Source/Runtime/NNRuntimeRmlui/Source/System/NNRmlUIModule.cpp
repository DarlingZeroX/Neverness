/**
 * @file NNRmlUIModule.cpp
 * @brief RmlUI 模块入口实现——创建和销毁 NNRmlUISystem。
 */

#include "System/NNRmlUIModule.h"
#include "System/NNRmlUISystem.h"

namespace NN::Runtime::RmlUI
{
	NNRmlUISystem* CreateRmlUISystem()
	{
		return new NNRmlUISystem();
	}

	void DestroyRmlUISystem(NNRmlUISystem* system)
	{
		delete system;
	}

} // namespace NN::Runtime::RmlUI
