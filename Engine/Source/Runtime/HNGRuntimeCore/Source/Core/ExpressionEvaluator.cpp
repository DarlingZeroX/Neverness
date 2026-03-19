#include "Core/ExpressionEvaluator.h"
#include <cctype>
#include <vector>
#include <unordered_map>

namespace Horizon::NodeGraphRuntime
{
	// ---------------- 全局表达式缓存 ----------------
	//
	// 设计目标：
	// - 避免对相同表达式字符串反复执行“词法 + 语法”解析
	// - 本实现中：CompileExpression 只做一次 Lexer + AST 构建，后续执行直接遍历 AST。
	static std::unordered_map<std::string, CompiledExpression> g_ExpressionCache;

	// ---------------- Lexer ----------------

	class Lexer
	{
	public:
		explicit Lexer(const std::string& src) : m_Src(src), m_Pos(0) {}

		Token Next()
		{
			SkipSpaces();
			if (m_Pos >= m_Src.size())
				return { TokenType::End, "" };

			char c = m_Src[m_Pos];

			// 标识符 / 关键字
			if (std::isalpha(static_cast<unsigned char>(c)) || c == '_')
			{
				std::string id;
				while (m_Pos < m_Src.size())
				{
					char ch = m_Src[m_Pos];
					if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_')
					{
						id.push_back(ch);
						++m_Pos;
					}
					else break;
				}
				if (id == "true" || id == "false")
					return { TokenType::BoolLiteral, id };
				return { TokenType::Identifier, id };
			}

			// 数字字面量（简单实现：以 '.' 是否出现区分 Int / Float）
			if (std::isdigit(static_cast<unsigned char>(c)))
			{
				std::string num;
				bool hasDot = false;
				while (m_Pos < m_Src.size())
				{
					char ch = m_Src[m_Pos];
					if (std::isdigit(static_cast<unsigned char>(ch)))
					{
						num.push_back(ch);
						++m_Pos;
					}
					else if (ch == '.' && !hasDot)
					{
						hasDot = true;
						num.push_back(ch);
						++m_Pos;
					}
					else break;
				}
				return { hasDot ? TokenType::FloatLiteral : TokenType::IntLiteral, num };
			}

			// 字符串字面量（支持简单的 "..."）
			if (c == '"')
			{
				++m_Pos; // skip "
				std::string s;
				while (m_Pos < m_Src.size())
				{
					char ch = m_Src[m_Pos++];
					if (ch == '"')
						break;
					// 简单实现：不特别处理转义字符
					s.push_back(ch);
				}
				return { TokenType::StringLiteral, s };
			}

			// 其他符号 / 运算符
			++m_Pos;
			switch (c)
			{
			case '+': return { TokenType::Plus, "+" };
			case '-': return { TokenType::Minus, "-" };
			case '*': return { TokenType::Star, "*" };
			case '/': return { TokenType::Slash, "/" };
			case '(': return { TokenType::LParen, "(" };
			case ')': return { TokenType::RParen, ")" };
			case '<':
				if (Match('=')) return { TokenType::LessEqual, "<=" };
				return { TokenType::Less, "<" };
			case '>':
				if (Match('=')) return { TokenType::GreaterEqual, ">=" };
				return { TokenType::Greater, ">" };
			case '=':
				if (Match('=')) return { TokenType::EqualEqual, "==" };
				break;
			case '!':
				if (Match('=')) return { TokenType::NotEqual, "!=" };
				break;
			case '&':
				if (Match('&')) return { TokenType::AndAnd, "&&" };
				break;
			case '|':
				if (Match('|')) return { TokenType::OrOr, "||" };
				break;
			default:
				break;
			}
			// 未识别字符，返回 End（调用方应做容错）
			return { TokenType::End, "" };
		}

	private:
		void SkipSpaces()
		{
			while (m_Pos < m_Src.size() && std::isspace(static_cast<unsigned char>(m_Src[m_Pos])))
				++m_Pos;
		}

		bool Match(char expected)
		{
			if (m_Pos < m_Src.size() && m_Src[m_Pos] == expected)
			{
				++m_Pos;
				return true;
			}
			return false;
		}

		const std::string& m_Src;
		size_t m_Pos;
	};

	// ---------------- AST Parser（语法分析：tokens -> AST） ----------------

	class ExpressionParserAST
	{
	public:
		explicit ExpressionParserAST(const std::vector<Token>& tokens)
			: m_Tokens(tokens), m_Index(0)
		{
		}

		ASTNode* ParseExpressionAST()
		{
			return ParseOr();
		}

	private:
		// 语法：
		// orExpr  := andExpr ( '||' andExpr )*
		// andExpr := equality ( '&&' equality )*
		// equality := relational ( ( '==' | '!=' ) relational )*
		// relational := additive ( ( '<' | '>' | '<=' | '>=' ) additive )*
		// additive := term ( ('+' | '-') term )*
		// term     := unary ( ('*' | '/') unary )*
		// unary    := ('-' | '!') unary | primary
		// primary  := literal | identifier | '(' orExpr ')'

		ASTNode* ParseOr()
		{
			ASTNode* left = ParseAnd();
			while (Current().type == TokenType::OrOr)
			{
				Advance();
				ASTNode* right = ParseAnd();

				auto* bin = new ASTNode();
				bin->type = ASTNodeType::Binary;
				bin->op   = TokenType::OrOr;
				bin->left = left;
				bin->right= right;
				left = bin;
			}
			return left;
		}

		ASTNode* ParseAnd()
		{
			ASTNode* left = ParseEquality();
			while (Current().type == TokenType::AndAnd)
			{
				Advance();
				ASTNode* right = ParseEquality();

				auto* bin = new ASTNode();
				bin->type = ASTNodeType::Binary;
				bin->op   = TokenType::AndAnd;
				bin->left = left;
				bin->right= right;
				left = bin;
			}
			return left;
		}

		ASTNode* ParseEquality()
		{
			ASTNode* left = ParseRelational();
			while (Current().type == TokenType::EqualEqual || Current().type == TokenType::NotEqual)
			{
				TokenType op = Current().type;
				Advance();
				ASTNode* right = ParseRelational();

				auto* bin = new ASTNode();
				bin->type = ASTNodeType::Binary;
				bin->op   = op;
				bin->left = left;
				bin->right= right;
				left = bin;
			}
			return left;
		}

		ASTNode* ParseRelational()
		{
			ASTNode* left = ParseAdditive();
			while (Current().type == TokenType::Less || Current().type == TokenType::LessEqual ||
				   Current().type == TokenType::Greater || Current().type == TokenType::GreaterEqual)
			{
				TokenType op = Current().type;
				Advance();
				ASTNode* right = ParseAdditive();

				auto* bin = new ASTNode();
				bin->type = ASTNodeType::Binary;
				bin->op   = op;
				bin->left = left;
				bin->right= right;
				left = bin;
			}
			return left;
		}

		ASTNode* ParseAdditive()
		{
			ASTNode* left = ParseTerm();
			while (Current().type == TokenType::Plus || Current().type == TokenType::Minus)
			{
				TokenType op = Current().type;
				Advance();
				ASTNode* right = ParseTerm();

				auto* bin = new ASTNode();
				bin->type = ASTNodeType::Binary;
				bin->op   = op;
				bin->left = left;
				bin->right= right;
				left = bin;
			}
			return left;
		}

		ASTNode* ParseTerm()
		{
			ASTNode* left = ParseUnary();
			while (Current().type == TokenType::Star || Current().type == TokenType::Slash)
			{
				TokenType op = Current().type;
				Advance();
				ASTNode* right = ParseUnary();

				auto* bin = new ASTNode();
				bin->type = ASTNodeType::Binary;
				bin->op   = op;
				bin->left = left;
				bin->right= right;
				left = bin;
			}
			return left;
		}

		ASTNode* ParseUnary()
		{
			if (Current().type == TokenType::Minus)
			{
				TokenType op = Current().type;
				Advance();
				ASTNode* operand = ParseUnary();

				auto* un = new ASTNode();
				un->type = ASTNodeType::Unary;
				un->op   = op;
				un->operand = operand;
				return un;
			}
			return ParsePrimary();
		}

		ASTNode* ParsePrimary()
		{
			switch (Current().type)
			{
			case TokenType::IntLiteral:
			{
				auto* n = new ASTNode();
				n->type = ASTNodeType::Literal;
				n->literal = Value::FromInt(std::stoi(Current().text));
				Advance();
				return n;
			}
			case TokenType::FloatLiteral:
			{
				auto* n = new ASTNode();
				n->type = ASTNodeType::Literal;
				n->literal = Value::FromFloat(std::stof(Current().text));
				Advance();
				return n;
			}
			case TokenType::StringLiteral:
			{
				auto* n = new ASTNode();
				n->type = ASTNodeType::Literal;
				n->literal = Value::FromString(Current().text);
				Advance();
				return n;
			}
			case TokenType::BoolLiteral:
			{
				auto* n = new ASTNode();
				n->type = ASTNodeType::Literal;
				n->literal = Value::FromBool(Current().text == "true");
				Advance();
				return n;
			}
			case TokenType::Identifier:
			{
				auto* n = new ASTNode();
				n->type = ASTNodeType::Variable;
				n->name = Current().text;
				Advance();
				return n;
			}
			case TokenType::LParen:
			{
				Advance();
				ASTNode* v = ParseOr();
				if (Current().type == TokenType::RParen)
					Advance();
				return v;
			}
			default:
				break;
			}
			// 语法错误：返回 Literal(None)
			auto* n = new ASTNode();
			n->type = ASTNodeType::Literal;
			n->literal = Value();
			return n;
		}

		const Token& Current() const
		{
			// 约定：tokens 末尾一定有 End token，因此这里安全返回
			if (m_Index >= m_Tokens.size())
				return m_EndToken;
			return m_Tokens[m_Index];
		}

		void Advance()
		{
			if (m_Index < m_Tokens.size())
				++m_Index;
		}

	private:
		const std::vector<Token>& m_Tokens;
		size_t m_Index;
		Token m_EndToken{ TokenType::End, "" };
	};

	// ---------------- AST 执行（EvalAST） ----------------

	static bool CompareEqual(const Value& a, const Value& b)
	{
		// 同类型：按各自类型比较
		if (a.type == b.type)
		{
			switch (a.type)
			{
			case ValueType::Int:    return a.AsInt()   == b.AsInt();
			case ValueType::Float:  return a.AsFloat() == b.AsFloat();
			case ValueType::Bool:   return a.AsBool()  == b.AsBool();
			case ValueType::String: return a.AsString() == b.AsString();
			default:                return true; // None 视为相等
			}
		}
		// 不同类型时：尝试按数值比较
		if ((a.type == ValueType::Int || a.type == ValueType::Float) &&
			(b.type == ValueType::Int || b.type == ValueType::Float))
		{
			return a.AsFloat() == b.AsFloat();
		}
		return false;
	}

	static bool CompareRelational(const Value& a, const Value& b, TokenType op)
	{
		// 数值比较
		if ((a.type == ValueType::Int || a.type == ValueType::Float) &&
			(b.type == ValueType::Int || b.type == ValueType::Float))
		{
			double lf = a.AsFloat();
			double rf = b.AsFloat();
			switch (op)
			{
			case TokenType::Less:         return lf <  rf;
			case TokenType::LessEqual:    return lf <= rf;
			case TokenType::Greater:      return lf >  rf;
			case TokenType::GreaterEqual: return lf >= rf;
			default: return false;
			}
		}

		// 字符串比较：按字典序
		if (a.type == ValueType::String && b.type == ValueType::String)
		{
			const auto& sa = a.AsString();
			const auto& sb = b.AsString();
			switch (op)
			{
			case TokenType::Less:         return sa <  sb;
			case TokenType::LessEqual:    return sa <= sb;
			case TokenType::Greater:      return sa >  sb;
			case TokenType::GreaterEqual: return sa >= sb;
			default: return false;
			}
		}

		// 其他类型组合：不做有意义比较
		return false;
	}

	static Value EvalAST(ASTNode* node, RuntimeContext& ctx)
	{
		if (!node) return Value();

		switch (node->type)
		{
		case ASTNodeType::Literal:
			return node->literal;

		case ASTNodeType::Variable:
		{
			auto it = ctx.variables.find(node->name);
			if (it != ctx.variables.end())
				return it->second;
			return Value();
		}

		case ASTNodeType::Unary:
		{
			Value v = EvalAST(node->operand, ctx);
			if (node->op == TokenType::Minus)
			{
				if (v.type == ValueType::Int)
					return Value::FromInt(static_cast<int>(-v.AsInt()));
				return Value::FromFloat(static_cast<float>(-v.AsFloat()));
			}
			// 以后可扩展逻辑非等
			return v;
		}

		case ASTNodeType::Binary:
		{
			Value l = EvalAST(node->left, ctx);
			Value r = EvalAST(node->right, ctx);

			switch (node->op)
			{
			// 算术运算
			case TokenType::Plus:
			{
				// 字符串拼接
				if (l.type == ValueType::String || r.type == ValueType::String)
				{
					std::string a = l.AsString();
					std::string b = r.AsString();
					return Value::FromString(a + b);
				}
				double lf = l.AsFloat();
				double rf = r.AsFloat();
				return Value::FromFloat(static_cast<float>(lf + rf));
			}
			case TokenType::Minus:
			{
				double lf = l.AsFloat();
				double rf = r.AsFloat();
				return Value::FromFloat(static_cast<float>(lf - rf));
			}
			case TokenType::Star:
			{
				double lf = l.AsFloat();
				double rf = r.AsFloat();
				return Value::FromFloat(static_cast<float>(lf * rf));
			}
			case TokenType::Slash:
			{
				double lf = l.AsFloat();
				double rf = r.AsFloat();
				if (rf == 0.0) return Value::FromFloat(0.0f);
				return Value::FromFloat(static_cast<float>(lf / rf));
			}

			// 等值比较
			case TokenType::EqualEqual:
			case TokenType::NotEqual:
			{
				bool eq = CompareEqual(l, r);
				if (node->op == TokenType::NotEqual) eq = !eq;
				return Value::FromBool(eq);
			}

			// 大小比较
			case TokenType::Less:
			case TokenType::LessEqual:
			case TokenType::Greater:
			case TokenType::GreaterEqual:
			{
				bool res = CompareRelational(l, r, node->op);
				return Value::FromBool(res);
			}

			// 逻辑
			case TokenType::AndAnd:
			{
				bool lb = l.AsBool();
				bool rb = r.AsBool();
				return Value::FromBool(lb && rb);
			}
			case TokenType::OrOr:
			{
				bool lb = l.AsBool();
				bool rb = r.AsBool();
				return Value::FromBool(lb || rb);
			}

			default:
				break;
			}
		}
		}

		return Value();
	}

	// ---------------- Bytecode 编译（AST -> 指令序列） ----------------

	// 从 AST 递归生成字节码（后序遍历：先子节点，再当前节点）
	static void CompileBytecode(ASTNode* node, std::vector<Instruction>& out)
	{
		if (!node) return;

		switch (node->type)
		{
		case ASTNodeType::Literal:
		{
			Instruction ins;
			switch (node->literal.type)
			{
			case ValueType::Int:
				ins.op = OpCode::PushInt;
				ins.intOperand = static_cast<int>(node->literal.AsInt());
				break;
			case ValueType::Float:
				ins.op = OpCode::PushFloat;
				ins.floatOperand = static_cast<float>(node->literal.AsFloat());
				break;
			case ValueType::Bool:
				ins.op = OpCode::PushBool;
				ins.intOperand = node->literal.AsBool() ? 1 : 0;
				break;
			case ValueType::String:
				ins.op = OpCode::PushString;
				ins.strOperand = node->literal.AsString();
				break;
			default:
				// None：压入一个 None 值（这里简单用 PushInt 0 代表）
				ins.op = OpCode::PushInt;
				ins.intOperand = 0;
				break;
			}
			out.push_back(std::move(ins));
			break;
		}

		case ASTNodeType::Variable:
		{
			Instruction ins;
			ins.op = OpCode::LoadVar;
			ins.strOperand = node->name;
			out.push_back(std::move(ins));
			break;
		}

		case ASTNodeType::Unary:
		{
			// 先编译 operand，再发 Negate 指令
			CompileBytecode(node->operand, out);
			if (node->op == TokenType::Minus)
			{
				Instruction ins;
				ins.op = OpCode::Negate;
				out.push_back(std::move(ins));
			}
			break;
		}

		case ASTNodeType::Binary:
		{
			// 左右子树按顺序编译，结果会依次压栈：left, right
			CompileBytecode(node->left, out);
			CompileBytecode(node->right, out);

			Instruction ins;
			switch (node->op)
			{
			case TokenType::Plus:         ins.op = OpCode::Add; break;
			case TokenType::Minus:        ins.op = OpCode::Sub; break;
			case TokenType::Star:         ins.op = OpCode::Mul; break;
			case TokenType::Slash:        ins.op = OpCode::Div; break;

			case TokenType::EqualEqual:   ins.op = OpCode::Equal;        break;
			case TokenType::NotEqual:     ins.op = OpCode::NotEqual;     break;
			case TokenType::Less:         ins.op = OpCode::Less;         break;
			case TokenType::LessEqual:    ins.op = OpCode::LessEqual;    break;
			case TokenType::Greater:      ins.op = OpCode::Greater;      break;
			case TokenType::GreaterEqual: ins.op = OpCode::GreaterEqual; break;

			case TokenType::AndAnd:       ins.op = OpCode::And; break;
			case TokenType::OrOr:         ins.op = OpCode::Or;  break;

			default:
				ins.op = OpCode::Return;
				break;
			}
			out.push_back(std::move(ins));
			break;
		}
		}
	}

	// ---------------- Bytecode 执行（Stack-based VM） ----------------

	static Value ExecuteBytecode(const std::vector<Instruction>& code, RuntimeContext& ctx)
	{
		std::vector<Value> stack;
		stack.reserve(16);

		auto pop = [&]() -> Value
		{
			if (stack.empty()) return Value();
			Value v = stack.back();
			stack.pop_back();
			return v;
		};

		for (const auto& ins : code)
		{
			switch (ins.op)
			{
			case OpCode::PushInt:
				stack.push_back(Value::FromInt(ins.intOperand));
				break;

			case OpCode::PushFloat:
				stack.push_back(Value::FromFloat(ins.floatOperand));
				break;

			case OpCode::PushBool:
				stack.push_back(Value::FromBool(ins.intOperand != 0));
				break;

			case OpCode::PushString:
				stack.push_back(Value::FromString(ins.strOperand));
				break;

			case OpCode::LoadVar:
			{
				auto it = ctx.variables.find(ins.strOperand);
				if (it != ctx.variables.end())
					stack.push_back(it->second);
				else
					stack.push_back(Value());
				break;
			}

			case OpCode::Add:
			{
				Value rb = pop();
				Value lb = pop();
				// 字符串拼接
				if (lb.type == ValueType::String || rb.type == ValueType::String)
				{
					std::string a = lb.AsString();
					std::string b = rb.AsString();
					stack.push_back(Value::FromString(a + b));
				}
				else
				{
					double lf = lb.AsFloat();
					double rf = rb.AsFloat();
					stack.push_back(Value::FromFloat(static_cast<float>(lf + rf)));
				}
				break;
			}

			case OpCode::Sub:
			{
				Value rb = pop();
				Value lb = pop();
				double lf = lb.AsFloat();
				double rf = rb.AsFloat();
				stack.push_back(Value::FromFloat(static_cast<float>(lf - rf)));
				break;
			}

			case OpCode::Mul:
			{
				Value rb = pop();
				Value lb = pop();
				double lf = lb.AsFloat();
				double rf = rb.AsFloat();
				stack.push_back(Value::FromFloat(static_cast<float>(lf * rf)));
				break;
			}

			case OpCode::Div:
			{
				Value rb = pop();
				Value lb = pop();
				double lf = lb.AsFloat();
				double rf = rb.AsFloat();
				if (rf == 0.0)
					stack.push_back(Value::FromFloat(0.0f));
				else
					stack.push_back(Value::FromFloat(static_cast<float>(lf / rf)));
				break;
			}

			case OpCode::Equal:
			case OpCode::NotEqual:
			{
				Value rb = pop();
				Value lb = pop();
				bool eq = CompareEqual(lb, rb);
				if (ins.op == OpCode::NotEqual) eq = !eq;
				stack.push_back(Value::FromBool(eq));
				break;
			}

			case OpCode::Less:
			case OpCode::LessEqual:
			case OpCode::Greater:
			case OpCode::GreaterEqual:
			{
				Value rb = pop();
				Value lb = pop();
				TokenType op =
					(ins.op == OpCode::Less)         ? TokenType::Less :
					(ins.op == OpCode::LessEqual)    ? TokenType::LessEqual :
					(ins.op == OpCode::Greater)      ? TokenType::Greater :
					TokenType::GreaterEqual;
				bool res = CompareRelational(lb, rb, op);
				stack.push_back(Value::FromBool(res));
				break;
			}

			case OpCode::And:
			{
				Value rb = pop();
				Value lb = pop();
				bool res = lb.AsBool() && rb.AsBool();
				stack.push_back(Value::FromBool(res));
				break;
			}

			case OpCode::Or:
			{
				Value rb = pop();
				Value lb = pop();
				bool res = lb.AsBool() || rb.AsBool();
				stack.push_back(Value::FromBool(res));
				break;
			}

			case OpCode::Negate:
			{
				Value v = pop();
				if (v.type == ValueType::Int)
					stack.push_back(Value::FromInt(static_cast<int>(-v.AsInt())));
				else
					stack.push_back(Value::FromFloat(static_cast<float>(-v.AsFloat())));
				break;
			}

			case OpCode::Return:
				// Return：立即返回当前栈顶作为最终结果
				return stack.empty() ? Value() : stack.back();
			}
		}

		// 没有显式 Return 的情况：返回栈顶或 None
		return stack.empty() ? Value() : stack.back();
	}

	// ---------------- 对外接口实现 ----------------

	CompiledExpression CompileExpression(const std::string& expr)
	{
		CompiledExpression c;
		c.expr = expr;
		// 1) 词法分析：缓存 tokens（包含 End token）
		Lexer lex(expr);
		for (;;)
		{
			Token t = lex.Next();
			c.tokens.push_back(t);
			if (t.type == TokenType::End)
				break;
		}
		// 2) 语法分析：构建 AST
		ExpressionParserAST parser(c.tokens);
		c.root = parser.ParseExpressionAST();

		// 3) 从 AST 编译为字节码指令序列
		c.code.clear();
		if (c.root)
		{
			CompileBytecode(c.root, c.code);
			// 在结尾追加 Return，确保 VM 能正确结束
			Instruction ret;
			ret.op = OpCode::Return;
			c.code.push_back(ret);
		}
		return c;
	}

	Value ExecuteExpression(const CompiledExpression& compiled, RuntimeContext& ctx)
	{
		if (!compiled.code.empty())
		{
			// 优先使用字节码 VM 执行
			return ExecuteBytecode(compiled.code, ctx);
		}
		// 兼容：若尚未生成字节码，则回退到 AST 解释执行
		if (compiled.root)
			return EvalAST(compiled.root, ctx);
		return Value();
	}

	Value EvaluateExpression(const std::string& expr, RuntimeContext& ctx)
	{
		// 1) 查询全局缓存
		auto it = g_ExpressionCache.find(expr);
		if (it == g_ExpressionCache.end())
		{
			// 2) 若不存在，则预编译并放入缓存
			CompiledExpression compiled = CompileExpression(expr);
			it = g_ExpressionCache.emplace(expr, std::move(compiled)).first;
		}

		// 3) 执行缓存版本
		return ExecuteExpression(it->second, ctx);
	}
}

