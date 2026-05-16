/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "ReactiveCore/DerivedStateGraph.h"

#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace VisionGal::Editor::ReactiveCore
{
	void DerivedStateGraph::RebuildDependentsList()
	{
		for (Node& n : m_nodes)
			n.dependents.clear();
		for (Node& node : m_nodes)
		{
			for (const DerivedStateId dep : node.dependsOn)
			{
				for (Node& other : m_nodes)
				{
					if (other.id == dep)
					{
						other.dependents.push_back(node.id);
						break;
					}
				}
			}
		}
	}

	void DerivedStateGraph::Clear()
	{
		m_nodes.clear();
		m_topoOrder.clear();
	}

	int DerivedStateGraph::FindIndex(const DerivedStateId id) const
	{
		for (size_t i = 0; i < m_nodes.size(); ++i)
		{
			if (m_nodes[i].id == id)
				return static_cast<int>(i);
		}
		return -1;
	}

	void DerivedStateGraph::RegisterNode(
		const DerivedStateId id,
		std::vector<DerivedStateId> dependsOn,
		ComputeFn compute)
	{
		if (FindIndex(id) >= 0)
			return;
		Node n;
		n.id = id;
		n.dependsOn = std::move(dependsOn);
		n.compute = std::move(compute);
		m_nodes.push_back(std::move(n));
		RebuildDependentsList();
		RebuildTopo();
	}

	void DerivedStateGraph::Invalidate(const DerivedStateId id)
	{
		const int idx = FindIndex(id);
		if (idx < 0)
			return;
		m_nodes[static_cast<size_t>(idx)].dirty = true;
	}

	void DerivedStateGraph::InvalidateAll()
	{
		for (Node& n : m_nodes)
			n.dirty = true;
	}

	bool DerivedStateGraph::IsDirty(const DerivedStateId id) const
	{
		const int idx = FindIndex(id);
		if (idx < 0)
			return false;
		return m_nodes[static_cast<size_t>(idx)].dirty;
	}

	void DerivedStateGraph::RebuildTopo()
	{
		m_topoOrder.clear();
		if (m_nodes.empty())
			return;
		std::unordered_map<DerivedStateId, int> indeg;
		for (const Node& n : m_nodes)
			indeg[n.id] = 0;
		for (const Node& n : m_nodes)
		{
			for (const DerivedStateId d : n.dependsOn)
			{
				bool depExists = false;
				for (const Node& o : m_nodes)
				{
					if (o.id == d)
					{
						depExists = true;
						break;
					}
				}
				if (depExists)
					indeg[n.id]++;
			}
		}
		std::queue<DerivedStateId> q;
		for (const Node& n : m_nodes)
		{
			if (indeg[n.id] == 0)
				q.push(n.id);
		}
		while (!q.empty())
		{
			const DerivedStateId u = q.front();
			q.pop();
			m_topoOrder.push_back(u);
			const int ui = FindIndex(u);
			if (ui < 0)
				continue;
			for (const DerivedStateId v : m_nodes[static_cast<size_t>(ui)].dependents)
			{
				indeg[v]--;
				if (indeg[v] == 0)
					q.push(v);
			}
		}
		if (m_topoOrder.size() != m_nodes.size())
		{
			m_topoOrder.clear();
			for (const Node& n : m_nodes)
				m_topoOrder.push_back(n.id);
		}
	}

	void DerivedStateGraph::FlushDirty()
	{
		if (m_topoOrder.empty())
			RebuildTopo();
		std::unordered_set<DerivedStateId> dirtySet;
		for (const Node& n : m_nodes)
		{
			if (n.dirty)
				dirtySet.insert(n.id);
		}
		if (dirtySet.empty())
			return;
		bool changed = true;
		while (changed)
		{
			changed = false;
			for (const DerivedStateId tid : m_topoOrder)
			{
				const int ti = FindIndex(tid);
				if (ti < 0)
					continue;
				Node& node = m_nodes[static_cast<size_t>(ti)];
				if (node.dirty || dirtySet.count(node.id))
					continue;
				for (const DerivedStateId dep : node.dependsOn)
				{
					if (dirtySet.count(dep))
					{
						dirtySet.insert(node.id);
						changed = true;
						break;
					}
				}
			}
		}
		for (const DerivedStateId tid : m_topoOrder)
		{
			if (!dirtySet.count(tid))
				continue;
			const int ti = FindIndex(tid);
			if (ti < 0)
				continue;
			Node& node = m_nodes[static_cast<size_t>(ti)];
			if (node.compute)
				node.compute();
			node.dirty = false;
		}
	}
}
