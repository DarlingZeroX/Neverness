#pragma once

#include "RuntimeApplication.h"

namespace NN::Runtime::Application
{

/** @brief 进程内唯一 RuntimeApplication 实例（供 BuildApplicationApi / BuildWindowApi 共用）。 */
RuntimeApplication& GetRuntimeApplicationInstance();

} // namespace NN::Runtime::Application
