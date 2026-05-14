#pragma once

#include <cstddef>

/**
 * Opaque runtime snapshot for diagnostics / future hot-reload hooks.
 * Engine code must treat all fields as opaque — only VGManagedHost reads them internally.
 */
struct VGManagedRuntimeContext
{
	void* hostfxrHostContext{};
	void* loadAssemblyAndGetFunctionPointerDelegate{};
};
