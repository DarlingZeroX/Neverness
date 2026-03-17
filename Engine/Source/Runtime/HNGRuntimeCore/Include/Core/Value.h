/*
* 运行时 NodeGraph 实现（无虚函数、POD 风格、函数指针驱动）
* 提供基础的数据结构：Value, RuntimeSlot, RuntimeEdge, RuntimeNode, RuntimeGraph, RuntimeContext
*/

#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>

namespace Horizon::NodeGraphRuntime
{
	// Value 类型标记
	enum class ValueType : uint8_t
	{
		None = 0,
		Int,
		Float,
		Bool,
		String
	};

    // 简单的 Value 变体（支持 int/float/bool/string）
	// 说明：用于节点间或全局变量表的数据承载；带有访问器方法以方便读取不同类型
	struct Value
	{
		ValueType type;
		union
		{
			int64_t i;
			double f;
			bool b;
		} data;
		std::string s;

		// POD 风格构造
		Value() noexcept : type(ValueType::None) { data.i = 0; }
		explicit Value(int64_t v) noexcept : type(ValueType::Int) { data.i = v; }
		explicit Value(double v) noexcept : type(ValueType::Float) { data.f = v; }
		explicit Value(bool v) noexcept : type(ValueType::Bool) { data.b = v; }
		explicit Value(const std::string& v) : type(ValueType::String), s(v) {}
		explicit Value(std::string&& v) : type(ValueType::String), s(std::move(v)) {}

        // 将 Value 清空为 None
		void SetNone() noexcept { type = ValueType::None; data.i = 0; s.clear(); }

        // 读取接口：根据当前类型返回对应值，否则返回类型的默认值
		int64_t AsInt() const noexcept { return (type == ValueType::Int) ? data.i : 0; }
		double AsFloat() const noexcept { return (type == ValueType::Float) ? data.f : 0.0; }
		bool AsBool() const noexcept { return (type == ValueType::Bool) ? data.b : false; }
		const std::string& AsString() const noexcept { return s; }
	};

}
