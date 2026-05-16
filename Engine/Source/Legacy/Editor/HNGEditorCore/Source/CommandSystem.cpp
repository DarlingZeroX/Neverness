/*
* 命令系统实现（Undo/Redo）
*/

#include "CommandSystem.h"

#include <algorithm>

namespace Horizon::NodeGraphEditor
{
	// ---------------- CommandManager ----------------

	void CommandManager::PushToUndoStack(std::unique_ptr<ICommand> cmd)
	{
		if (!cmd) return;
		m_UndoStack.push_back(std::move(cmd));
	}

	void CommandManager::ExecuteCommand(std::unique_ptr<ICommand> cmd)
	{
		if (!cmd) return;

		// Batch 模式：执行但不立即入栈，暂存到 batch 列表，最终合并为一次 Undo/Redo。
		if (m_BatchDepth > 0)
		{
			cmd->Execute();

			// 第一次在 batch 中执行新命令时，清空 redo（等价于“发生了新的用户操作”）
			if (!m_BatchClearedRedo)
			{
				m_RedoStack.clear();
				m_BatchClearedRedo = true;
			}

			m_BatchCommands.push_back(std::move(cmd));
			return;
		}

		// 普通模式
		cmd->Execute();

		// 一旦执行了新命令，redo 历史就失效（与绝大多数编辑器一致）
		m_RedoStack.clear();
		PushToUndoStack(std::move(cmd));
	}

	void CommandManager::BeginBatch()
	{
		++m_BatchDepth;
	}

	void CommandManager::EndBatch()
	{
		if (m_BatchDepth <= 0)
			return;

		--m_BatchDepth;
		if (m_BatchDepth > 0)
			return; // 仍在外层 batch 内

		// batch 结束：把暂存命令合并推入 undo 栈
		if (m_BatchCommands.empty())
		{
			m_BatchClearedRedo = false;
			return;
		}

		if (m_BatchCommands.size() == 1)
		{
			PushToUndoStack(std::move(m_BatchCommands.front()));
			m_BatchCommands.clear();
			m_BatchClearedRedo = false;
			return;
		}

		PushToUndoStack(std::make_unique<CompositeCommand>(std::move(m_BatchCommands)));
		m_BatchCommands.clear();
		m_BatchClearedRedo = false;
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
		m_BatchCommands.clear();
		m_BatchDepth = 0;
		m_BatchClearedRedo = false;
	}
}

