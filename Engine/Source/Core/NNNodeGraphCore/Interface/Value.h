/*
* 运行时 NodeGraph 实现（无虚函数、POD 风格、函数指针驱动）
* 提供基础的数据结构：Value, RuntimeSlot, RuntimeEdge, RuntimeNode, RuntimeGraph, RuntimeContext
*/

#pragma once

#include <string>
#include <cstdint>

namespace NN::Core::NodeGraphRuntime
{
	// Value 类型标记（简单类型系统）
	enum class ValueType : uint8_t
	{
		None = 0,
		Int,
		Float,
		Bool,
		String
	};

    // 简单的 Value 变体（支持 int / float / bool / string）
	// 说明：
	// - 用作节点槽值以及 RuntimeContext::variables 中的通用数据容器
	// - 采用显式字段而非 std::variant，方便与脚本/序列化系统集成
	// - 通过静态工厂函数 FromXXX 与 AsXXX 访问接口简化使用
	struct Value
	{
		ValueType type;

		// 显式存储每种基础类型的值
		int   intValue;
		float floatValue;
		bool  boolValue;
		std::string stringValue;

		// POD 风格构造：默认 None
		Value() noexcept
			: type(ValueType::None)
			, intValue(0)
			, floatValue(0.0f)
			, boolValue(false)
			, stringValue()
		{
		}

		// 兼容构造函数（便于旧代码使用）
		explicit Value(int64_t v) noexcept
			: type(ValueType::Int)
			, intValue(static_cast<int>(v))
			, floatValue(0.0f)
			, boolValue(false)
			, stringValue()
		{
		}

		explicit Value(double v) noexcept
			: type(ValueType::Float)
			, intValue(0)
			, floatValue(static_cast<float>(v))
			, boolValue(false)
			, stringValue()
		{
		}

		explicit Value(bool v) noexcept
			: type(ValueType::Bool)
			, intValue(0)
			, floatValue(0.0f)
			, boolValue(v)
			, stringValue()
		{
		}

		explicit Value(const std::string& v)
			: type(ValueType::String)
			, intValue(0)
			, floatValue(0.0f)
			, boolValue(false)
			, stringValue(v)
		{
		}

		explicit Value(std::string&& v)
			: type(ValueType::String)
			, intValue(0)
			, floatValue(0.0f)
			, boolValue(false)
			, stringValue(std::move(v))
		{
		}

		// 工厂函数：用于显式构造特定类型的 Value
		static Value FromInt(int v) noexcept
		{
			Value val;
			val.type = ValueType::Int;
			val.intValue = v;
			return val;
		}

		static Value FromFloat(float v) noexcept
		{
			Value val;
			val.type = ValueType::Float;
			val.floatValue = v;
			return val;
		}

		static Value FromBool(bool v) noexcept
		{
			Value val;
			val.type = ValueType::Bool;
			val.boolValue = v;
			return val;
		}

		static Value FromString(const std::string& v)
		{
			Value val;
			val.type = ValueType::String;
			val.stringValue = v;
			return val;
		}

		// 将 Value 清空为 None
		void SetNone() noexcept
		{
			type = ValueType::None;
			intValue = 0;
			floatValue = 0.0f;
			boolValue = false;
			stringValue.clear();
		}

		// 读取接口：根据当前类型返回对应值，否则返回类型的默认值
		int64_t AsInt() const noexcept
		{
			return (type == ValueType::Int) ? static_cast<int64_t>(intValue) : 0;
		}

		double AsFloat() const noexcept
		{
			if (type == ValueType::Float) return static_cast<double>(floatValue);
			if (type == ValueType::Int)   return static_cast<double>(intValue);
			return 0.0;
		}

		bool AsBool() const noexcept
		{
			if (type == ValueType::Bool)  return boolValue;
			if (type == ValueType::Int)   return intValue != 0;
			if (type == ValueType::Float) return floatValue != 0.0f;
			return false;
		}

		const std::string& AsString() const noexcept
		{
			return stringValue;
		}
	};

}
