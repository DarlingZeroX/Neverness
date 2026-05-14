#include <mutex>

#include "VGManagedCore/ManagedExports.h"
#include "VGManagedCore/ManagedRuntimeServices.h"

namespace
{
	std::once_flag g_defaultTableOnce;
	VGNativeAPI g_defaultTable{};
	const VGNativeAPI* g_defaultTablePtr = nullptr;
} // namespace

extern "C" const VGNativeAPI* VGNativeApi_GetDefaultTable(void)
{
	std::call_once(g_defaultTableOnce, [] {
		VGNativeApiTable_BuildDefault(&g_defaultTable);
		g_defaultTablePtr = &g_defaultTable;
	});
	return g_defaultTablePtr;
}
