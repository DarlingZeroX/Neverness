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
#include "VGCore/Interface/RenderInterface.h"

namespace VisionGal
{
	struct FullScreenBlackEdge: public ISpriteEffect
	{
		FullScreenBlackEdge() = default;
		~FullScreenBlackEdge() = default;

		// 通过 SpriteEffect 继承
		void ApplyEffect(ISprite* sprite) override;
	private:
		void SetWindowSize(int2 size);

		void SetImageSize(int2 size);

		void Calculate();
		 
		// 获取窗口尺寸
		int2 WindowSize = int2();
		int2 ImageSize = int2();

		float2 RightTop = float2();
		float2 RightBottom = float2();
		float2 LeftTop = float2();
		float2 LeftBottom = float2();
	};
}