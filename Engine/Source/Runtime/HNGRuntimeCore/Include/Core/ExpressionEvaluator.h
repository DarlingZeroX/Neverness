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
#include <vector>
#include "../../Interface/Value.h"
#include "RuntimeContext.h"

namespace Horizon::NodeGraphRuntime
{
	// ----------------------------
	// Token（词法单元）
	//
	// 说明：
	// - TokenType / Token 原本定义在 .cpp 内部；为了让 CompiledExpression 缓存 tokens，
	//   需要将其提升到头文件中，以便外部能持有 std::vector<Token>。
	// - tokens 中会包含一个 End token 作为结束标记（CompileExpression 保证）。
	// ----------------------------
	enum class TokenType
	{
		End,
		Identifier,
		IntLiteral,
		FloatLiteral,
		StringLiteral,
		BoolLiteral,

		Plus, Minus, Star, Slash,
		LParen, RParen,
		Less, LessEqual,
		Greater, GreaterEqual,
		EqualEqual, NotEqual,
		AndAnd, OrOr
	};

	struct Token
	{
		TokenType type = TokenType::End;
		std::string text;
	};

	// ----------------------------
	// AST 节点定义
	// ----------------------------
	enum class ASTNodeType
	{
		Literal,
		Variable,
		Binary,
		Unary
	};

	struct ASTNode
	{
		ASTNodeType type = ASTNodeType::Literal;

		// Literal
		Value literal;

		// Variable
		std::string name;

		// Binary
		TokenType op = TokenType::End;
		ASTNode* left = nullptr;
		ASTNode* right = nullptr;

		// Unary
		ASTNode* operand = nullptr;
	};

	// 预编译表达式结构
	// 说明：
	// - expr   : 原始源码
	// - tokens : 词法分析结果（含 End token）
	// - root   : AST 根节点（仅构建一次，后续 Eval 复用）
	// - code   : 可选的字节码指令序列（Stack-based VM 使用）

	// ----------------------------
	// 字节码指令（Stack-based VM）
	// ----------------------------
	// 设计说明：
	// - 所有表达式执行都基于栈（后进先出）
	// - Push/Load 指令将 Value 压入栈顶
	// - 二元运算指令从栈顶弹出两个操作数，计算后压回结果
	// - 一元运算指令从栈顶弹出一个操作数，计算后压回结果
	// - Return 指令表示表达式求值结束，栈顶即为最终结果
	enum class OpCode
	{
		// 常量入栈
		PushInt,      // 使用 Instruction::intOperand
		PushFloat,    // 使用 Instruction::floatOperand
		PushBool,     // 使用 intOperand != 0 表示 true，否则 false
		PushString,   // 使用 Instruction::strOperand

		// 变量访问：从 ctx.variables 中按名称取值压栈（name 存在于 strOperand）
		LoadVar,

		// 算术运算
		Add,
		Sub,
		Mul,
		Div,

		// 比较运算
		Equal,
		NotEqual,
		Less,
		LessEqual,
		Greater,
		GreaterEqual,

		// 逻辑运算
		And,
		Or,

		// 一元运算
		Negate,

		// 返回结果
		Return
	};

	// 单条字节码指令
	// - op          : 操作码（指令类型）
	// - strOperand  : 字符串操作数（例如 PushString 的字面量、LoadVar 的变量名）
	// - floatOperand: 浮点操作数（例如 PushFloat）
	// - intOperand  : 整型操作数（例如 PushInt / PushBool）
	struct Instruction
	{
		OpCode op = OpCode::Return;
		std::string strOperand;
		float floatOperand = 0.0f;
		int intOperand = 0;
	};

	struct CompiledExpression
	{
		std::string expr;
		std::vector<Token> tokens;
		ASTNode* root = nullptr;
		std::vector<Instruction> code;
	};

	// 表达式预编译：
	// - 仅负责构建 CompiledExpression，不做求值
	H_NODE_GRAPH_API CompiledExpression CompileExpression(const std::string& expr);

	// 执行已编译表达式：
	// - 内部仍然使用同一套解析/求值逻辑
	// - 好处是：可以配合缓存，避免重复管理原始字符串
	H_NODE_GRAPH_API Value ExecuteExpression(const CompiledExpression& compiled, RuntimeContext& ctx);

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

