//cpp Runtime\VGEngine\Include\Lua\LuaDataBridge.h
#pragma once
#include "sol2/sol.hpp"
#include <unordered_map>

namespace VisionGal
{
	// 在不同的 lua_State 之间拷贝 Lua 数据（支持常见原语与表的递归拷贝）
	// 说明：
	// - 支持类型：nil, boolean, number, string, table（递归拷贝）
	// - 对于 function、userdata、thread 等不可安全跨 state 传递的类型，返回 nil
	// - 不处理循环引用（简单实现），若需要支持循环引用，可引入地址缓存映射
	class LuaDataBridge
	{
	public:
		// 从 srcState 中的 srcObj 拷贝到 dstState，返回在 dstState 中的 sol::object
		// 使用示例：
		// sol::object dstObj = LuaDataBridge::Copy(srcState, srcObj, dstState);
		static sol::object Copy(sol::state_view srcState, const sol::object& srcObj, sol::state_view dstState)
		{
			// 简单缓存：避免重复创建同一表（可选，当前实现不追踪循环）
			std::unordered_map<const void*, sol::table> table_cache;
			return CopyInternal(srcObj, dstState, table_cache);
		}

	private:
		using TableCache = std::unordered_map<const void*, sol::table>;

		static sol::object CopyInternal(const sol::object& srcObj, sol::state_view dstState, TableCache& cache)
		{
			// nil
			if (!srcObj.valid() || srcObj.is<sol::nil_t>())
			{
				return sol::make_object(dstState, sol::nil);
			}

			// boolean
			if (srcObj.is<bool>())
			{
				return sol::make_object(dstState, srcObj.as<bool>());
			}

			// number (double)
			if (srcObj.is<double>() || srcObj.is<float>() || srcObj.is<int>() || srcObj.is<long long>())
			{
				// 以 double 传递所有数值
				double v = srcObj.as<double>();
				return sol::make_object(dstState, v);
			}

			// string
			if (srcObj.is<std::string>() || srcObj.is<const char*>())
			{
				std::string s = srcObj.as<std::string>();
				return sol::make_object(dstState, s);
			}

			// table - 递归拷贝
			if (srcObj.get_type() == sol::type::table)
			{
				sol::table srcTable = srcObj.as<sol::table>();

				// 尝试简单去重：使用 srcTable 的地址作为 key（注意：此地址在不同 Lua 状态间不可比较）
				// 这里我们仍然使用 srcTable.pointer() 作为缓存键（若 sol 版本不支持 pointer()，则缓存失效）
				const void* src_ptr = nullptr;
				#if SOL_SAFE_GETTER
				// SOL_SAFE_GETTER 在不同 sol 版本间可能存在差异，尝试获取底层指针
				src_ptr = srcTable.pointer();
				#else
				src_ptr = srcTable.pointer();
				#endif

				// 如果缓存中已有对应 dst 表，直接返回（避免重复构造）
				if (src_ptr != nullptr)
				{
					auto it = cache.find(src_ptr);
					if (it != cache.end())
					{
						return sol::make_object(dstState, it->second);
					}
				}

				// 在目标 state 中创建新表
				sol::table dstTable = dstState.create_table();

				// 存入缓存（先占位，帮助处理递归表）
				if (src_ptr != nullptr)
					cache.emplace(src_ptr, dstTable);

				// 迭代表字段
				srcTable.for_each([&](sol::object key, sol::object value) {
					// 仅支持可转换的键类型（string / number / boolean / table（递归））
					sol::object k = CopyInternal(key, dstState, cache);
					sol::object v = CopyInternal(value, dstState, cache);

					// 将 k, v 写入 dstTable。sol2 支持以 sol::object 作为 key/value 写入。
					// 若某些 sol 版本不支持直接使用 sol::object 作为索引，可按类型分支再写入。
					try
					{
						dstTable[k] = v;
					}
					catch (...)
					{
						// 兼容性降级：只接受常用键类型
						if (k.is<std::string>())
						{
							dstTable.set(k.as<std::string>(), v);
						}
						else if (k.is<double>())
						{
							dstTable.set(k.as<double>(), v);
						}
						else if (k.is<bool>())
						{
							dstTable.set(k.as<bool>(), v);
						}
						// 其它键类型暂不支持
					}
				});

				return sol::make_object(dstState, dstTable);
			}

			// function / userdata / thread 等不可跨 state 复制：返回 nil（可以根据需求改为抛出错误或自定义代理）
			return sol::make_object(dstState, sol::nil);
		}
	};
}