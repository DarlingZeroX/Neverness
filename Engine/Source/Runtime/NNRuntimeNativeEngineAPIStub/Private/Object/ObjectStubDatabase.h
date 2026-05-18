#pragma once

/**
 * @file ObjectStubDatabase.h
 * @brief Stub **NNObjectAPI** 之進程內物件槽位表（引用計數與型別名）。
 */

#include <cstddef>
#include <cstdint>

#include "NNNativeEngineAPI/Include/EngineHandles.h"

namespace NN::StubRuntime::Object
{
NNObjectHandle CreateObject(const char* typeNameUtf8);
void DestroyObject(NNObjectHandle object);
void RetainObject(NNObjectHandle object);
void ReleaseObject(NNObjectHandle object);
std::uint32_t GetRefCount(NNObjectHandle object);
bool IsAlive(NNObjectHandle object);
/** @return 寫入字節數；失敗返回 -1。 */
int GetTypeName(NNObjectHandle object, char* outUtf8, std::size_t outCapacity);
} // namespace NN::StubRuntime::Object
