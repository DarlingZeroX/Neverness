/*
* 命令系统实现（Undo/Redo）
*/

#include "CommandSystem.h"

#include <algorithm>

namespace Horizon::NodeGraphEditor
{
	// ---------------- CommandManager ----------------

	void CommandManager::ExecuteCommand(std::unique_ptr<ICommand> cmd)
	{
		if (!cmd) return;
		cmd->Execute();

		// 一旦执行了新命令，redo 历史就失效（与绝大多数编辑器一致）
		m_RedoStack.clear();
		m_UndoStack.push_back(std::move(cmd));
	}

	void CommandManager::Undo()
	{
		if (m_UndoStack.empty()) return;
		auto cmd = std::move(m_UndoStack.back());
		m_UndoStack.pop_back();
		cmd->Undo();
		m_RedoStack.push_back(std::move(cmd));
	}

	void CommandManager::Redo()
	{
		if (m_RedoStack.empty()) return;
		auto cmd = std::move(m_RedoStack.back());
		m_RedoStack.pop_back();
		cmd->Execute();
		m_UndoStack.push_back(std::move(cmd));
	}

	void CommandManager::Clear()
	{
		m_UndoStack.clear();
		m_RedoStack.clear();
	}
}

