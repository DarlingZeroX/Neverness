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

#include "Render/Effect.h"
#include "Graphics/OpenGL/Texture2D.h"

namespace VisionGal
{
	void FullScreenBlackEdge::SetWindowSize(int2 size)
	{
		WindowSize = size;
	}

	void FullScreenBlackEdge::SetImageSize(int2 size)
	{
		ImageSize = size;
	}

	void FullScreenBlackEdge::Calculate()
	{
		// 获取窗口尺寸
		int windowWidth, windowHeight;
		windowWidth = WindowSize.x;
		windowHeight = WindowSize.y;
		 
		// 计算缩放比例，保持宽高比
		float scaleX = (float)windowWidth / ImageSize.x;
		float scaleY = (float)windowHeight / ImageSize.y;
		float scale = std::min(scaleX, scaleY);

		// 计算显示区域的尺寸和位置
		int displayWidth = static_cast<int>(ImageSize.x * scale);
		int displayHeight = static_cast<int>(ImageSize.y * scale);
		int x = (windowWidth - displayWidth) / 2;
		int y = (windowHeight - displayHeight) / 2;

		// 计算变换后的顶点位置
		float xOffset = (float)x / windowWidth;
		float yOffset = (float)y / windowHeight;

		//	vertices[0] = 1.0f - xOffset * 2.0f; vertices[1] = 1.0f - yOffset * 2.0f; // 右上
		//	vertices[5] = 1.0f - xOffset * 2.0f; vertices[6] = -1.0f + yOffset * 2.0f; // 右下
		//	vertices[10] = -1.0f + xOffset * 2.0f; vertices[11] = -1.0f + yOffset * 2.0f; // 左下
		//	vertices[15] = -1.0f + xOffset * 2.0f; vertices[16] = 1.0f - yOffset * 2.0f; // 左上


		RightTop = { 1.0f - xOffset * 2.0f, 1.0f - yOffset * 2.0f };		// 右上
		RightBottom = { 1.0f - xOffset * 2.0f, -1.0f + yOffset * 2.0f };	// 右下
		LeftBottom = { -1.0f + xOffset * 2.0f, -1.0f + yOffset * 2.0f };	// 左下
		LeftTop = { -1.0f + xOffset * 2.0f, 1.0f - yOffset * 2.0f };		// 左上
	}

	void FullScreenBlackEdge::ApplyEffect(ISprite* sprite)
	{
		//auto* spritegl = sprite->GetSprite();
		//OpenGL::Texture2D* texgl = dynamic_cast<OpenGL::Texture2D*>(sprite->GetITexture()->GetITexture().get());

		//WindowSize = { VGBackend::Get()->GetWindow()->WindowWidth(), VGBackend::Get()->GetWindow()->WindowHeight() };
		//ImageSize = { 1.f , texgl->Height() };
		Calculate();

		//spritegl->SetPosition(RightTop, RightBottom, LeftBottom, LeftTop);
	}
}