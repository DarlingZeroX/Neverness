/**
 * @file RuntimeApplicationInstance.cpp
 * @brief 进程内 **RuntimeApplication** 单例存储。
 */

#include "RuntimeApplicationInstance.h"

namespace NN::Runtime::Application
{
namespace
{
RuntimeApplication GInstance;
}

RuntimeApplication& GetRuntimeApplicationInstance()
{
	return GInstance;
}

} // namespace NN::Runtime::Application
