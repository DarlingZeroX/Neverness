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

#include "../EngineConfig.h"
#include "NNRuntimeCore/Interface/SceneInterface.h"

namespace NN::Runtime
{
	class VG_ENGINE_API GameActor : public IGameActor
	{
	public:
		GameActor() = default;
		GameActor(const GameActor&) = default;
		GameActor& operator=(const GameActor&) = default;
		GameActor(GameActor&&) noexcept = default;
		GameActor& operator=(GameActor&&) noexcept = default;
		~GameActor() override = default;

		void SetVisible(bool visible) override;
		bool GetVisible() override;

		IComponent* GetComponentByType(const String& type) override;
		IComponent* AddComponentByType(const String& type) override;
	public:
		void Initialize(IScene* scene) override {};
	};
}