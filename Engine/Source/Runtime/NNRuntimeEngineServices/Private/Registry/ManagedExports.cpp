#include <mutex>

#include "ManagedExports.h"
#include "ManagedRuntimeServices.h"

namespace
{
	std::once_flag g_defaultTableOnce;
	NNNativeAPI g_defaultTable{};
	const NNNativeAPI* g_defaultTablePtr = nullptr;
} // namespace

extern "C" const NNNativeAPI* NNNativeApi_GetDefaultTable(void)
{
	std::call_once(g_defaultTableOnce, [] {
		NNNativeApiTable_BuildDefault(&g_defaultTable);
		g_defaultTablePtr = &g_defaultTable;
	});
	return g_defaultTablePtr;
}
