#include "Core/ExpressionEvaluator.h"
#include <cctype>
#include <vector>

namespace Horizon::NodeGraphRuntime
{
	// ---------------- Token 定义 ----------------

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

	// ---------------- Parser + Evaluator ----------------

	class ExpressionParser
	{
	public:
		ExpressionParser(const std::string& src, RuntimeContext& ctx)
			: m_Lexer(src), m_Ctx(ctx)
		{
			m_Current = m_Lexer.Next();
		}

		Value Parse()
		{
			Value v = ParseOr();
			return v;
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

		Value ParseOr()
		{
			Value left = ParseAnd();
			while (m_Current.type == TokenType::OrOr)
			{
				Advance();
				Value right = ParseAnd();
				bool lb = left.AsBool();
				bool rb = right.AsBool();
				left = Value::FromBool(lb || rb);
			}
			return left;
		}

		Value ParseAnd()
		{
			Value left = ParseEquality();
			while (m_Current.type == TokenType::AndAnd)
			{
				Advance();
				Value right = ParseEquality();
				bool lb = left.AsBool();
				bool rb = right.AsBool();
				left = Value::FromBool(lb && rb);
			}
			return left;
		}

		Value ParseEquality()
		{
			Value left = ParseRelational();
			while (m_Current.type == TokenType::EqualEqual || m_Current.type == TokenType::NotEqual)
			{
				TokenType op = m_Current.type;
				Advance();
				Value right = ParseRelational();
				bool result = CompareEqual(left, right);
				if (op == TokenType::NotEqual) result = !result;
				left = Value::FromBool(result);
			}
			return left;
		}

		Value ParseRelational()
		{
			Value left = ParseAdditive();
			while (m_Current.type == TokenType::Less || m_Current.type == TokenType::LessEqual ||
				   m_Current.type == TokenType::Greater || m_Current.type == TokenType::GreaterEqual)
			{
				TokenType op = m_Current.type;
				Advance();
				Value right = ParseAdditive();
				bool result = CompareRelational(left, right, op);
				left = Value::FromBool(result);
			}
			return left;
		}

		Value ParseAdditive()
		{
			Value left = ParseTerm();
			while (m_Current.type == TokenType::Plus || m_Current.type == TokenType::Minus)
			{
				TokenType op = m_Current.type;
				Advance();
				Value right = ParseTerm();

				// 字符串相加：做拼接
				if (op == TokenType::Plus &&
					(left.type == ValueType::String || right.type == ValueType::String))
				{
					std::string a = left.AsString();
					std::string b = right.AsString();
					left = Value::FromString(a + b);
					continue;
				}

				double lf = left.AsFloat();
				double rf = right.AsFloat();
				double res = (op == TokenType::Plus) ? (lf + rf) : (lf - rf);
				left = Value::FromFloat(static_cast<float>(res));
			}
			return left;
		}

		Value ParseTerm()
		{
			Value left = ParseUnary();
			while (m_Current.type == TokenType::Star || m_Current.type == TokenType::Slash)
			{
				TokenType op = m_Current.type;
				Advance();
				Value right = ParseUnary();
				double lf = left.AsFloat();
				double rf = right.AsFloat();
				// 简单防护：除零直接返回 0
				if (op == TokenType::Slash && rf == 0.0)
				{
					left = Value::FromFloat(0.0f);
				}
				else
				{
					double res = (op == TokenType::Star) ? (lf * rf) : (lf / rf);
					left = Value::FromFloat(static_cast<float>(res));
				}
			}
			return left;
		}

		Value ParseUnary()
		{
			if (m_Current.type == TokenType::Minus)
			{
				Advance();
				Value v = ParseUnary();
				if (v.type == ValueType::Int)
					return Value::FromInt(static_cast<int>(-v.AsInt()));
				return Value::FromFloat(static_cast<float>(-v.AsFloat()));
			}
			return ParsePrimary();
		}

		Value ParsePrimary()
		{
			switch (m_Current.type)
			{
			case TokenType::IntLiteral:
			{
				int v = std::stoi(m_Current.text);
				Advance();
				return Value::FromInt(v);
			}
			case TokenType::FloatLiteral:
			{
				float v = std::stof(m_Current.text);
				Advance();
				return Value::FromFloat(v);
			}
			case TokenType::StringLiteral:
			{
				std::string s = m_Current.text;
				Advance();
				return Value::FromString(s);
			}
			case TokenType::BoolLiteral:
			{
				bool b = (m_Current.text == "true");
				Advance();
				return Value::FromBool(b);
			}
			case TokenType::Identifier:
			{
				std::string name = m_Current.text;
				Advance();
				// 从 RuntimeContext::variables 中读取变量值
				auto it = m_Ctx.variables.find(name);
				if (it != m_Ctx.variables.end())
					return it->second;
				// 未找到变量：返回 None
				return Value();
			}
			case TokenType::LParen:
			{
				Advance();
				Value v = ParseOr();
				if (m_Current.type == TokenType::RParen)
					Advance();
				return v;
			}
			default:
				break;
			}
			// 语法错误：返回 None
			return Value();
		}

		void Advance()
		{
			m_Current = m_Lexer.Next();
		}

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

	private:
		Lexer m_Lexer;
		Token m_Current;
		RuntimeContext& m_Ctx;
	};

	// ---------------- 对外接口实现 ----------------

	Value EvaluateExpression(const std::string& expr, RuntimeContext& ctx)
	{
		ExpressionParser parser(expr, ctx);
		return parser.Parse();
	}
}

