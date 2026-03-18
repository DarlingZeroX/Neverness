/*
* 简易表达式求值器（ExpressionEvaluator）
*
* 目标：
* - 为 NodeGraph 提供轻量级“脚本能力”，支持在节点中编写简单表达式
* - 支持的基础特性：
*   - 算术运算：+ - * /
*   - 比较运算：< <= > >= == !=
*   - 逻辑运算：&& ||
*   - 字面量：整数、浮点数、布尔（true/false）、字符串（"..."）
*   - 变量引用：从 RuntimeContext::variables 中读取 Value
*
* 语义约定：
* - 所有中间结果使用 Value 表示，遵循 ValueType 的基本转换规则：
*   - 算术运算：Int / Float 自动提升为 Float
*   - 比较运算：对数字按数值比较，对字符串按字典序比较，对 Bool 按 true/false 比较
*   - 逻辑运算：先将参与者转换为 Bool 再进行 && / ||
*/

#pragma once

#include <string>
#include "../../Interface/Value.h"
#include "RuntimeContext.h"

namespace Horizon::NodeGraphRuntime
{
	// 表达式求值入口
	// 参数：
	// - expr : 表达式源码（例如："hp > 10 && name == \"Alice\""）
	// - ctx  : 当前运行时上下文，用于读取变量表 ctx.variables
	//
	// 返回：
	// - 计算结果封装为 Value
	// - 解析/运行时出错时，返回 type == ValueType::None 的 Value
	H_NODE_GRAPH_API Value EvaluateExpression(const std::string& expr, RuntimeContext& ctx);
}

