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
#include <VGGalgameCore/Include/Components.h>
#include <VGEngine/Include/Scene/Components.h>

namespace VisionGal::Editor
{
	struct TransformComponentDrawer : public IComponentDrawer
	{
		~TransformComponentDrawer() override = default;

		void OnGUI(IEntity* entity) override;
		const String GetBindType() const override;
	};

	struct CameraComponentDrawer : public IComponentDrawer
	{
		~CameraComponentDrawer() override = default;

		void OnGUI(IEntity* entity) override;
		const String GetBindType() const override;
	};

	struct SpriteRendererComponentDrawer: public IComponentDrawer
	{
		~SpriteRendererComponentDrawer() override = default;

		void OnGUI(IEntity* entity) override;
		const String GetBindType() const override;

		void SpriteBeginDropTarget(SpriteRendererComponent* com);
	};

	struct ScriptComponentDrawer: public IComponentDrawer
	{
		~ScriptComponentDrawer() override = default;

		void OnGUI(IEntity* entity) override;
		const String GetBindType() const override;

		void ScriptBeginDropTarget(IScript* script, IScriptVariable& var);
	};

	struct RmlUIDocumentComponentDrawer : public IComponentDrawer
	{
		~RmlUIDocumentComponentDrawer() override = default;

		void OnGUI(IEntity* entity) override;
		const String GetBindType() const override;

		void DocumentBeginDropTarget(RmlUIDocumentComponent* com);
	};

	struct AudioSourceComponentDrawer : public IComponentDrawer
	{
		~AudioSourceComponentDrawer() override = default;

		void OnGUI(IEntity* entity) override;
		const String GetBindType() const override;

		void AudioSourceBeginDropTarget(AudioSourceComponent* com);
	};

	struct VideoPlayerComponentDrawer : public IComponentDrawer
	{
		~VideoPlayerComponentDrawer() override = default;

		void OnGUI(IEntity* entity) override;
		const String GetBindType() const override;

		void VideoPlayerBeginDropTarget(VideoPlayerComponent* com);
	};
}