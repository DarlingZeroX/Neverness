// Copyright (c) 2025 梦旅缘心
// This file is part of VisionGal and is licensed under the MIT License.
// See the LICENSE file in the project root for details.

#pragma once
#include "../Serialization/cereal.hpp"
#include "../Serialization/types/string.hpp" // 如果使用了string
#include "../Serialization/types/vector.hpp" // 如果使用了vector等类型
#include "../Serialization/types/unordered_map.hpp"
#include "../Serialization/archives/binary.hpp"
#include "../Serialization/archives/json.hpp"
#include "../Math/HMath.h"
#include <fstream>

namespace cereal {
    // int2
    template <class Archive>
    void serialize(Archive& ar, Horizon::int2& v) {
        ar(cereal::make_nvp("x", v.x),
            cereal::make_nvp("y", v.y));
    }

    // int3
    template <class Archive>
    void serialize(Archive& ar, Horizon::int3& v) {
        ar(cereal::make_nvp("x", v.x),
            cereal::make_nvp("y", v.y),
            cereal::make_nvp("z", v.z));
    }

    // int4
    template <class Archive>
    void serialize(Archive& ar, Horizon::int4& v) {
        ar(cereal::make_nvp("x", v.x),
            cereal::make_nvp("y", v.y),
            cereal::make_nvp("z", v.z),
            cereal::make_nvp("w", v.w));
    }

    // uint2
    template <class Archive>
    void serialize(Archive& ar, Horizon::uint2& v) {
        ar(cereal::make_nvp("x", v.x),
            cereal::make_nvp("y", v.y));
    }

    // uint3
    template <class Archive>
    void serialize(Archive& ar, Horizon::uint3& v) {
        ar(cereal::make_nvp("x", v.x),
            cereal::make_nvp("y", v.y),
            cereal::make_nvp("z", v.z));
    }

    // uint4
    template <class Archive>
    void serialize(Archive& ar, Horizon::uint4& v) {
        ar(cereal::make_nvp("x", v.x),
            cereal::make_nvp("y", v.y),
            cereal::make_nvp("z", v.z),
            cereal::make_nvp("w", v.w));
    }

    // float2
    template <class Archive>
    void serialize(Archive& ar, Horizon::float2& v) {
        ar(cereal::make_nvp("x", v.x),
            cereal::make_nvp("y", v.y));
    }

    // float3
    template <class Archive>
    void serialize(Archive& ar, Horizon::float3& v) {
        ar(cereal::make_nvp("x", v.x),
            cereal::make_nvp("y", v.y),
            cereal::make_nvp("z", v.z));
    }

    // float4
    template <class Archive>
    void serialize(Archive& ar, Horizon::float4& v) {
        ar(cereal::make_nvp("x", v.x),
            cereal::make_nvp("y", v.y),
            cereal::make_nvp("z", v.z),
            cereal::make_nvp("w", v.w));
    }

    // quaternion
    template <class Archive>
    void serialize(Archive& ar, Horizon::quaternion& q) {
        ar(cereal::make_nvp("x", q.x),
            cereal::make_nvp("y", q.y),
            cereal::make_nvp("z", q.z),
            cereal::make_nvp("w", q.w));
    }

    // matrix2x2
    template <class Archive>
    void serialize(Archive& ar, Horizon::matrix2x2& m) {
        // 扁平化矩阵为数组进行序列化
        std::array<float, 4> elements;
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                elements[i * 2 + j] = m[i][j];
            }
        }
        ar(cereal::make_nvp("elements", elements));

        // 反序列化时重新构建矩阵
        if constexpr (Archive::is_loading::value) {
            for (int i = 0; i < 2; ++i) {
                for (int j = 0; j < 2; ++j) {
                    m[i][j] = elements[i * 2 + j];
                }
            }
        }
    }

    // 为matrix3x3添加cereal支持
    template <class Archive>
    void serialize(Archive& ar, Horizon::matrix3x3& m) {
        // 扁平化矩阵为数组进行序列化
        std::array<float, 9> elements;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                elements[i * 3 + j] = m[i][j];
            }
        }
        ar(cereal::make_nvp("elements", elements));

        // 反序列化时重新构建矩阵
        if constexpr (Archive::is_loading::value) {
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    m[i][j] = elements[i * 3 + j];
                }
            }
        }
    }

    // 为matrix4x4添加cereal支持
    template <class Archive>
    void serialize(Archive& ar, Horizon::matrix4x4& m) {
        // 扁平化矩阵为数组进行序列化
        std::array<float, 16> elements;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                elements[i * 4 + j] = m[i][j];
            }
        }
        ar(cereal::make_nvp("elements", elements));

        // 反序列化时重新构建矩阵
        if constexpr (Archive::is_loading::value) {
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    m[i][j] = elements[i * 4 + j];
                }
            }
        }
    }
}