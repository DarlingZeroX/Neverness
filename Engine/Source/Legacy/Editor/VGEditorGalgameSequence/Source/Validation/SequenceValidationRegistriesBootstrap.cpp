/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Validation/SequenceValidationRegistriesBootstrap.h"

#include "ComponentRegistry/SequenceComponentRegistry.h"
#include "Validation/Builtin/EmptyDialogueValidator.h"
#include "Validation/Builtin/MissingResourcePathValidator.h"
#include "Validation/SequenceValidationRegistry.h"

namespace VisionGal::Editor
{
	void BootstrapSequenceValidationRegistry(SequenceValidationRegistry& registry, const SequenceComponentRegistry* components)
	{
		registry.Register(std::make_unique<EmptyDialogueValidator>(components));
		registry.Register(std::make_unique<MissingResourcePathValidator>(components));
	}
}
