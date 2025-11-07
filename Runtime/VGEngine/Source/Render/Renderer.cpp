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

#include "Render/Renderer.h"
#include "Resource/Texture2D.h"
#include "Render/Material.h"
#include "Render/Sprite.h"
#include "Scene/Components.h"
#include "Graphics/OpenGL/Core.h"

namespace VisionGal
{
	SpriteRendererHandler::SpriteRendererHandler()
	{

	}

	//void SpriteRendererHandler::Render(std::vector<GameActor*>& sprites, ICamera* camera)
	//{
	//	for (auto& obj : sprites)
	//	{
	//		Render(obj, camera);
	//	}
	//}

	void SpriteRendererHandler::Render(GameActor* actor, ICamera* camera, uint pipelineIndex)
	{
		TransformComponent* transform = actor->GetComponent<TransformComponent>();
		SpriteRendererComponent* spriteRenderer = actor->GetComponent<SpriteRendererComponent>();

		if (transform->visible == false)
			return;

		// 如果颜色透明则不渲染
		if (spriteRenderer->color.a == 0.f)
			return;

		if (spriteRenderer->pipelineIndex != pipelineIndex)
			return;

		auto* tex = spriteRenderer->sprite->GetITexture();
		auto* program = spriteRenderer->material->GetShaderProgram();

		VGFX::UseProgram(program);
		VGFX::SetTexture(0, "texture1", tex);
		VGFX::SetUniformFloat4("color", spriteRenderer->color);
		VGFX::SetUniformInt2("flip", spriteRenderer->flip);
		VGFX::SetUniformMatrix4("model", transform->model);
		VGFX::SetUniformMatrix4("projection", camera->GetMatrix());
		VGFX::RenderMesh(spriteRenderer->sprite->GetMesh());
	}

	void FullScreenRendererHandler::Render(FullScreenRendererComponent* renderer)
	{
		VGFX::ITexture* tex = renderer->image->GetTexture();
		auto* program = renderer->shader;

		VGFX::UseProgram(program);
		VGFX::SetTexture(0,"texture1", tex);
		VGFX::RenderMesh(renderer->image->GetMesh());
	}
}
