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
#include "../../EngineConfig.h"
#include "../../Core/Core.h"

namespace VisionGal::VGFX
{
	struct IShader
	{
		virtual ~IShader() = default;

		virtual unsigned int GetResourceHandle() = 0;
		virtual std::string GetType() = 0;
	};

	struct IShaderProgram
	{
		virtual ~IShaderProgram() = default;

		virtual void UseProgram() = 0;
		 
		virtual void SetInt(const std::string& name, int v) = 0;
		virtual void SetInt2(const std::string& name, int2 v) = 0;
		virtual void SetInt3(const std::string& name, int3 v) = 0;
		virtual void SetInt4(const std::string& name, int4 v) = 0;
		virtual void SetBool(const std::string& name, bool v) = 0;
		virtual void SetFloat(const std::string& name, float v) = 0;
		virtual void SetFloat2(const std::string& name, const Horizon::float2& v) = 0;
		virtual void SetFloat3(const std::string& name, const Horizon::float3& v3) = 0;
		virtual void SetFloat4(const std::string& name, const Horizon::float4& v4) = 0;
		virtual void SetMatrix2(const std::string& name, const Horizon::matrix2x2& matrix) = 0;
		virtual void SetMatrix3(const std::string& name, const Horizon::matrix3x3& matrix) = 0;
		virtual void SetMatrix4(const std::string& name, const Horizon::matrix4x4& matrix) = 0;
	};
}