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

#pragma once
#include "../../Interface/IComponentDrawer.h"
#include <NNEngineLegacy/Include/Scene/Components.h>

namespace NN::Editor
{
	struct TransformComponentDrawer : public IComponentDrawer
	{
		~TransformComponentDrawer() override = default;

		void OnGUI(Runtime::IEntity* entity) override;
		const Runtime::String GetBindType() const override;
	};

	struct CameraComponentDrawer : public IComponentDrawer
	{
		~CameraComponentDrawer() override = default;

		void OnGUI(Runtime::IEntity* entity) override;
		const Runtime::String GetBindType() const override;
	};

	struct SpriteRendererComponentDrawer: public IComponentDrawer
	{
		~SpriteRendererComponentDrawer() override = default;

		void OnGUI(Runtime::IEntity* entity) override;
		const Runtime::String GetBindType() const override;

		void SpriteBeginDropTarget(Runtime::SpriteRendererComponent* com);
	};

	struct ScriptComponentDrawer: public IComponentDrawer
	{
		~ScriptComponentDrawer() override = default;

		void OnGUI(Runtime::IEntity* entity) override;
		const Runtime::String GetBindType() const override;

		void ScriptBeginDropTarget(Runtime::IScript* script, Runtime::IScriptVariable& var);
	};

	struct RmlUIDocumentComponentDrawer : public IComponentDrawer
	{
		~RmlUIDocumentComponentDrawer() override = default;

		void OnGUI(Runtime::IEntity* entity) override;
		const Runtime::String GetBindType() const override;

		void DocumentBeginDropTarget(Runtime::RmlUIDocumentComponent* com);
	};

	struct AudioSourceComponentDrawer : public IComponentDrawer
	{
		~AudioSourceComponentDrawer() override = default;

		void OnGUI(Runtime::IEntity* entity) override;
		const Runtime::String GetBindType() const override;

		void AudioSourceBeginDropTarget(Runtime::AudioSourceComponent* com);
	};

	struct VideoPlayerComponentDrawer : public IComponentDrawer
	{
		~VideoPlayerComponentDrawer() override = default;

		void OnGUI(Runtime::IEntity* entity) override;
		const Runtime::String GetBindType() const override;

		void VideoPlayerBeginDropTarget(Runtime::VideoPlayerComponent* com);
	};
}