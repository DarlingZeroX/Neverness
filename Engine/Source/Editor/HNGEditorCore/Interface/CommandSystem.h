/*
* 命令系统（Command System）
*
* 🎯 目标：
* - 为蓝图编辑器提供可撤销/重做（Undo/Redo）的基础设施
* - 将“对图的修改”封装成命令对象，统一管理执行与回滚
*
* 为什么需要命令系统？
* - 编辑器操作（AddNode/DeleteNode/Link/MoveNode）都需要可撤销
* - 如果直接在 UI 回调里修改数据，很难可靠地恢复到历史状态
* - 命令对象天然包含“做/撤销”两套逻辑，更适合编辑器
*
* 设计原则：
* 1) 命令必须是“可逆的”
*    - Execute() 改变 EditorGraph 状态
*    - Undo() 必须能把状态恢复到 Execute() 之前
*
* 2) 命令要保存足够的快照数据
*    - DeleteNode 需要保存被删节点以及相关连线
*    - MoveNode 需要保存移动前/后的位置
*    - AddNode 需要保存创建出来的节点（含 id/pins），否则重做会生成不同 id 导致 link 断裂
*
* 3) CommandManager 负责：
*    - 执行命令并压入 undo 栈
*    - Undo：弹出 undo 栈并调用 Undo()，压入 redo 栈
*    - Redo：弹出 redo 栈并调用 Execute()，压回 undo 栈
*/

#pragma once

#include <memory>
#include <vector>
#include <optional>
#include "../HNGEditorCoreConfig.h"

namespace Horizon::NodeGraphEditor
{
	// ----------------------------
	// ICommand：命令接口
	// ----------------------------
	class HNG_EDITOR_CORE_API ICommand
	{
	public:
		virtual ~ICommand() = default;
		virtual void Execute() = 0;
		virtual void Undo() = 0;
	};

	// ----------------------------
	// CommandManager：命令管理器（Undo/Redo）
	// ----------------------------
	class HNG_EDITOR_CORE_API CommandManager
	{
	public:
		CommandManager() = default;
		~CommandManager() = default;

		// 命令历史不允许拷贝（unique_ptr 不可拷贝，且语义上也不应复制 Undo/Redo 栈）
		CommandManager(const CommandManager&) = delete;
		CommandManager& operator=(const CommandManager&) = delete;

		// 允许移动（例如容器搬运/对象移动）
		CommandManager(CommandManager&&) noexcept = default;
		CommandManager& operator=(CommandManager&&) noexcept = default;

		// 执行一个命令（会清空 redo 栈）
		void ExecuteCommand(std::unique_ptr<ICommand> cmd);

		// 撤销/重做
		bool CanUndo() const { return !m_UndoStack.empty(); }
		bool CanRedo() const { return !m_RedoStack.empty(); }
		void Undo();
		void Redo();

		// 清空历史（例如加载新图时）
		void Clear();

	private:
		std::vector<std::unique_ptr<ICommand>> m_UndoStack;
		std::vector<std::unique_ptr<ICommand>> m_RedoStack;
	};
}

