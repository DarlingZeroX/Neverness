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
#include "HMath.h"

namespace NN::Core
{
	struct Frustum
	{
	public:
		Frustum() = default;
		Frustum(const Frustum& other);
		Frustum& operator=(const Frustum& other);
		~Frustum() = default;
	public:
		void Tranform(const Mat4& mat);

		void SetNearPlane(float width, float height, float nearZ);

		void SetNearPlane(const Frustum& frustum, float scale);

		void SetFarPlane(const Frustum& frustum, float scale);

		void SetFarPlane(float width, float height, float farZ);

		float GetLongestDiagonal();


		const float3& operator[](std::size_t n) const
		{
			return points[n];
		}
	private:
		//Near
		const float3& NearLT() const
		{
			return points[0];
		}
		const float3& NearRT() const
		{
			return points[1];
		}
		const float3& NearRB() const
		{
			return points[2];
		}
		const float3& NearLB() const
		{
			return points[3];
		}
		//Far
		const float3& FarLT() const
		{
			return points[4];
		}
		const float3& FarRT() const
		{
			return points[5];
		}
		const float3& FarRB()const
		{
			return points[6];
		}
		const float3& FarLB() const
		{
			return points[7];
		}

	public:
		float m_NearZ;
		float m_FarZ;
		float m_NearW;
		float m_NearH;
		float m_FarW;
		float m_FarH;

		float3 points[8];
	};

	inline Frustum::Frustum(const Frustum& other)
	{
		*this = other;
	}

	inline Frustum& Frustum::operator=(const Frustum& other)
	{
		m_NearZ = other.m_NearZ;
		m_FarZ = other.m_FarZ;
		m_NearW = other.m_NearW;
		m_NearH = other.m_NearH;
		m_FarW = other.m_FarW;
		m_FarH = other.m_FarH;

		for (size_t i = 0; i < 8; i++)
		{
			points[i] = other[i];
		}

		return *this;
	}

	inline void Frustum::Tranform(const Mat4& mat)
	{
		for (size_t i = 0; i < 8; i++)
		{
			points[i] = mat * Vec4(points[i], 1.0f);
		}
	}

	inline void Frustum::SetNearPlane(float width, float height, float nearZ)
	{
		m_NearZ = nearZ;
		m_NearW = width;
		m_NearH = height;

		float w = width / 2.0f;
		float h = height / 2.0f;
		float z = -nearZ;

		points[0] = Vec3(-w, h, z);
		points[1] = Vec3(w, h, z);
		points[2] = Vec3(w, -h, z);
		points[3] = Vec3(-w, -h, z);
	}

	inline void Frustum::SetNearPlane(const Frustum& frustum, float scale)
	{
		float w = frustum.m_FarW - frustum.m_NearW;
		float h = frustum.m_FarH - frustum.m_NearH;
		float z = frustum.m_FarZ - frustum.m_NearZ;

		float nearW = frustum.m_NearW + scale * w;
		float nearH = frustum.m_NearH + scale * h;
		float nearZ = frustum.m_NearZ + scale * z;

		SetNearPlane(nearW, nearH, nearZ);
	}

	inline void Frustum::SetFarPlane(const Frustum& frustum, float scale)
	{
		float w = frustum.m_FarW - frustum.m_NearW;
		float h = frustum.m_FarH - frustum.m_NearH;
		float z = frustum.m_FarZ - frustum.m_NearZ;

		float farW = frustum.m_NearW + scale * w;
		float farH = frustum.m_NearH + scale * h;
		float farZ = frustum.m_NearZ + scale * z;

		SetFarPlane(farW, farH, farZ);
	}

	inline void Frustum::SetFarPlane(float width, float height, float farZ)
	{
		m_FarZ = farZ;
		m_FarW = width;
		m_FarH = height;

		float w = width / 2.0f;
		float h = height / 2.0f;
		float z = -farZ;

		points[4] = Vec3(-w, h, z);
		points[5] = Vec3(w, h, z);
		points[6] = Vec3(w, -h, z);
		points[7] = Vec3(-w, -h, z);
	}

	inline float Frustum::GetLongestDiagonal()
	{
		float3 longestDiagonal = points[0] - points[6];
		return glm::length(longestDiagonal);
	}
}
