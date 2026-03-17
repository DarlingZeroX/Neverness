/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*/

#include "Graph/LinkManager.h"

namespace Horizon::NodeGraph
{
	/*
	void LinkManager::InitializePinInfo(PIN_ID pinID)
	{
		const auto pinRes = m_PinMap.find(pinID);
		if (pinRes != m_PinMap.end())
			return;

		m_PinMap[pinID] = PinLinkInfo{};
	}

	bool LinkManager::AddLink(const Link& inLink)
	{
		if (IsLinkConnected(inLink.StartPinID, inLink.EndPinID))
			return false;

		LINK_ID uid = GetNextLinkUUID();

		Link link = inLink;
		link.LinkID = uid;

		const PIN_ID startPinID = link.StartPinID;
		const PIN_ID endPinID = link.EndPinID;

		InitializePinInfo(startPinID);
		InitializePinInfo(endPinID);

		m_PinMap[startPinID].Direction = PinDirection::Output;
		m_PinMap[startPinID].LinkIDs.emplace(uid);

		m_PinMap[endPinID].Direction = PinDirection::Input;
		m_PinMap[endPinID].LinkIDs.emplace(uid);

		m_LinkMap[uid] = link;

		// Link 创建事件
		LinkEvent evt;
		evt.LinkData = link;
		evt.EventType = LinkEventType::Created;
		OnLinkEvent.Invoke(evt);

		return true;
	}

	bool LinkManager::RemoveLinkOnly(LINK_ID linkID, Link& link)
	{
		const auto result = m_LinkMap.find(linkID);
		if (result == m_LinkMap.end())
			return false;

		const PIN_ID pinStart = result->second.StartPinID;
		const PIN_ID pinEnd = result->second.EndPinID;

		// 移除Link
		link = result->second;
		m_LinkMap.erase(linkID);

		// 在m_PinMap中移除Pin输出
		{
			const auto pinRes = m_PinMap.find(pinStart);
			if (pinRes != m_PinMap.end())
			{
				pinRes->second.LinkIDs.erase(linkID);
			}
		}

		// 在m_PinMap中移除Pin输入
		{
			const auto pinRes = m_PinMap.find(pinEnd);
			if (pinRes != m_PinMap.end())
			{
				pinRes->second.LinkIDs.erase(linkID);
			}
		}

		return true;
	}

	bool LinkManager::RemoveLinkWithEvent(LINK_ID id)
	{
		Link link;

		if (RemoveLinkOnly(id, link))
		{
			// Link 删除事件
			LinkEvent evt;
			evt.LinkData = link;
			evt.EventType = LinkEventType::Deleted;
			OnLinkEvent.Invoke(evt);

			return true;
		}

		return false;
	}

	LINK_ID LinkManager::GetNextLinkUUID() const
	{
		LINK_ID linkID = 1;
		while (ExistLink(linkID))
		{
			linkID++;
		}

		return linkID;
	}

	bool LinkManager::RemoveLink(LINK_ID id)
	{
		return RemoveLinkWithEvent(id);
	}

	bool LinkManager::RemoveLink(const Link& link)
	{
		return RemoveLink(link.LinkID);
	}

	void LinkManager::RemoveLinkByPinID(PIN_ID pinID)
	{
		std::set<LINK_ID> linkIDs;

		auto pinRes = m_PinMap.find(pinID);
		if (pinRes != m_PinMap.end())
		{
			linkIDs = pinRes->second.LinkIDs;
		}

		if (linkIDs.size() > 0)
		{
			std::cout << "==================Remove link by pin id: " << pinID << std::endl;
		}

		for (auto& linkID : linkIDs)
		{
			RemoveLink(linkID);
		}
	}

	bool LinkManager::FinLink(LINK_ID linkID, Link& link) const
	{
		auto result = m_LinkMap.find(linkID);
		if (result != m_LinkMap.end())
		{
			link = result->second;
			return true;
		}

		return false;
	}

	bool LinkManager::ExistLink(LINK_ID linkID) const
	{
		return m_LinkMap.find(linkID) != m_LinkMap.end();
	}

	size_t LinkManager::LinkSize() const noexcept
	{
		return m_LinkMap.size();
	}

	bool LinkManager::IsLinkConnected(const Link& link) const
	{
		return IsLinkConnected(link.StartPinID, link.EndPinID);
	}

	bool LinkManager::IsLinkConnected(PIN_ID start, PIN_ID end) const
	{
		auto startPinInfo = m_PinMap.find(start);
		auto endPinInfo = m_PinMap.find(end);

		if (startPinInfo != m_PinMap.end() && endPinInfo != m_PinMap.end())
		{
			auto& startLinkSet = startPinInfo->second.LinkIDs;
			auto& endLinkSet = endPinInfo->second.LinkIDs;

			for (auto& item : startLinkSet)
			{
				if (endLinkSet.find(item) != endLinkSet.end())
				{
					return true;
				}
			}
		}

		return false;
	}

	size_t LinkManager::QueryConnectedLinks(PIN_ID pinID) const
	{
		auto res = m_PinMap.find(pinID);
		if (res != m_PinMap.end())
		{
			return res->second.LinkIDs.size();
		}

		return 0;
	}

	bool LinkManager::PinHadAnyLink(PIN_ID pinID) const
	{
		return QueryConnectedLinks(pinID) > 0;
	}

	void LinkManager::GetConnectedLinks(PIN_ID pinID, std::vector<Link>& links)
	{
		auto res = m_PinMap.find(pinID);
		if (res != m_PinMap.end())
		{
			for (auto& linkID : res->second.LinkIDs)
			{
				Link link;
				if (FinLink(linkID, link))
				{
					links.push_back(link);
				}
				else
					__debugbreak();
			}
		}
	}
	*/
}