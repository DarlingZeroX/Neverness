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

#pragma once
#include "../Core/RuntimeCore.h"
#include "../Core/Events.h"
#include <set>
#include <unordered_map>
#include <HCore/Include/Event/HEventDelegate.h>

namespace Horizon::NodeGraph
{
	/*
	class H_NODE_GRAPH_API LinkManager
	{
	public:
		bool AddLink(const Link& link);

		bool RemoveLink(LINK_ID linkID);
		bool RemoveLink(const Link& link);
		void RemoveLinkByPinID(PIN_ID pinID);

		bool FinLink(LINK_ID linkID, Link& linkOUT) const;
		bool ExistLink(LINK_ID linkID) const;

		size_t LinkSize() const noexcept;

		bool IsLinkConnected(const Link& link) const;
		bool IsLinkConnected(PIN_ID start, PIN_ID end) const;

		size_t QueryConnectedLinks(PIN_ID pinID) const;
		bool PinHadAnyLink(PIN_ID pinID) const;
		void GetConnectedLinks(PIN_ID pinID, std::vector<Link>& links);

		HEventDelegate<const LinkEvent&> OnLinkEvent;
	protected:
		// 只删除连接，不发送事件，不调用源删除API
		bool RemoveLinkOnly(LINK_ID linkID, Link& link);
		// 带事件删除，无调用源删除API
		bool RemoveLinkWithEvent(LINK_ID linkID);

		LINK_ID GetNextLinkUUID() const;

		void InitializePinInfo(PIN_ID pinID);

		struct PinLinkInfo
		{
			std::set<LINK_ID> LinkIDs;
			PinDirection Direction;
		};

		std::unordered_map<LINK_ID, Link> m_LinkMap;
		std::unordered_map<PIN_ID, PinLinkInfo> m_PinMap;
	};
	*/
}
