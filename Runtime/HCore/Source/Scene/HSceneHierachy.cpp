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

#include "pch.h"
#include "Scene/HSceneHierachy.h"

namespace Horizon
{
	bool HHierarchy::DisconnectParent(HECS& registry, HEntityInterface& child, HRelationship& childInfo)
	{
		//assert(registry.valid(child));
		// 没有父类，为无效节点
		if (childInfo.Parent == nullptr)
		{
			assert(false);
			return true;
		}

		HRelationship* oldParentInfo = reinterpret_cast<HRelationship*>( childInfo.Parent->GetRelationShip() );
		assert(oldParentInfo);

		if (oldParentInfo == nullptr)
		{
			return false;
		}

		if (oldParentInfo->FirstChild == &child)                     // are we old parent's first child? not anymore so need to update its link
		{
			assert(childInfo.PrevSibling == nullptr);          // if we're a parent's FirstChild, we can't have prev siblings!
			oldParentInfo->FirstChild = childInfo.NextSibling;       // update old parent's FirstChild to the next sibling (if any, or null)
		}

		if (childInfo.PrevSibling != nullptr)              // is there a previous element in the list? make sure it no longer points at us
		{
			HRelationship* prevSiblingInfo = reinterpret_cast<HRelationship*>(childInfo.PrevSibling->GetRelationShip());
			assert(prevSiblingInfo->NextSibling == &child);         // it should have pointed at us

			prevSiblingInfo->NextSibling = childInfo.NextSibling;    // update prev sibling's link to next sibling (if any, or null)
		}

		if (childInfo.NextSibling != nullptr)                  // is there a next element in the list? make sure it no longer points at us
		{
			HRelationship* nextSiblingInfo = reinterpret_cast<HRelationship*>(childInfo.NextSibling->GetRelationShip());
			assert(nextSiblingInfo->PrevSibling == &child);        // it should have pointed at us

			nextSiblingInfo->PrevSibling = childInfo.PrevSibling;  // update prev sibling's link to next sibling (if any, or null)
		}

		oldParentInfo->ChildrenCount--;
		childInfo.Parent = nullptr;
		childInfo.PrevSibling = nullptr;
		childInfo.NextSibling = nullptr;

		return true;
	}

	bool HHierarchy::SetParent(HECS& registry, HEntityInterface& child, HEntityInterface* parent)
	{
		HRelationship* childInfo = reinterpret_cast<HRelationship*>(child.GetRelationShip());
		if (childInfo == nullptr)
		{
			return false;
		}

		assert(childInfo->IsValid(registry));

		// 我们肯定正在更改父级，所以我们必须断开当前的连接 we're definitely changing the parent so we have to disconnect current one
		if (childInfo->Parent != nullptr)
			DisconnectParent(registry, child, *childInfo);

		assert(childInfo->Parent == nullptr);

		if (parent == nullptr)  // 如果新父级为空，则只需断开当前节点，我们就完成了 if new parent is null then just disconnect current and we're done here
		{
			//SetTransformDirtyRecursiveUnsafe(registry, child);
			return true;
		}

		// 好的，我们有一个新的父级要设置，它不是空的。 ok, we've got a new parent to set, and it's not null
		//Components::Relationship& parentInfo = registry.get<Components::Relationship>(parent);
		HRelationship* parentInfo = reinterpret_cast<HRelationship*>(parent->GetRelationShip());
		assert(parentInfo != nullptr);
		assert(parentInfo->IsValid(registry));

		if (parentInfo == nullptr)
		{
			return false;
		}

		childInfo->Parent = parent;		// 更新新关系 update the new relationship

#ifndef HIERARCHY_INTER_FRIST_NODE
		// 如果父类没有子节点，我们就就是第一个子节点	If the parent class does not have a sub-node, we are the first sub-node.
		if (parentInfo->FirstChild == nullptr)
		{
			parentInfo->FirstChild = &child; // 我们现在是第一个孩子了！ we're the first child now!
		}
		// 如果有子节点，把我们自己也加到子节点链表末尾 If there is a sub-node, add ourselves to the end of the sub-node list
		else
		{
			HRelationship* firstSiblingInfo = reinterpret_cast<HRelationship*>(parentInfo->FirstChild->GetRelationShip());
			assert(firstSiblingInfo->IsValid(registry));

			// 获取子节点链表最后一个节点
			HRelationship* lastSiblingInfo = firstSiblingInfo;
			while (lastSiblingInfo->NextSibling != nullptr)
			{
				lastSiblingInfo = reinterpret_cast<HRelationship*>(lastSiblingInfo->NextSibling->GetRelationShip());
			}

			// 追加至链表结尾
			lastSiblingInfo->NextSibling = &child;

			// 设置末尾节点前一个节点
			if (lastSiblingInfo->PrevSibling == nullptr)	//如果前一个节点为空，代表当前节点是第一个子节点
			{
				childInfo->PrevSibling = parentInfo->FirstChild;
			}
			else
			{
				//获取之前末尾节点的前一个节点，用于获取之前的末尾节点实体
				auto* prevSiblingInfo = reinterpret_cast<HRelationship*>(lastSiblingInfo->PrevSibling->GetRelationShip());	
				childInfo->PrevSibling = prevSiblingInfo->NextSibling;
			}			
		}
#else
		// 如果有列表，把我们自己也加进去 if there's a list, insert ourselves into it
		if (parentInfo->FirstChild != nullptr)
		{
			HRelationship* firstSiblingInfo = reinterpret_cast<HRelationship*>(parentInfo->FirstChild->GetRelationShip());
			assert(firstSiblingInfo->IsValid(registry));
		
			firstSiblingInfo->PrevSibling = &child;
			childInfo->NextSibling = parentInfo->FirstChild;
		}
		
		parentInfo->FirstChild = &child; // we're the first child now!
#endif
		parentInfo->ChildrenCount++;

		// 如果深度改变了，我们必须确保整个树的深度是正确的 if the depth changed, we've got to make sure the depths are correct for the whole tree
		if (childInfo->Depth != parentInfo->Depth + 1)
		{
			childInfo->Depth = parentInfo->Depth + 1;
			//Scene::UpdateRelationshipDepthsRecursive(registry, child);
		}

		//SetTransformDirtyRecursiveUnsafe(registry, child);
		assert(parentInfo->ChildrenCount > 0);

		return true;
	}

	bool HHierarchy::SetNextSibling(HECS& registry, HEntityInterface* current, HEntityInterface* next)
	{
		HRelationship* currInfo = reinterpret_cast<HRelationship*>(current->GetRelationShip());
		if (currInfo == nullptr)
		{
			return false;
		}

		HRelationship* nextInfo = reinterpret_cast<HRelationship*>(next->GetRelationShip());
		if (nextInfo == nullptr)
		{
			return false;
		}

		assert(currInfo->IsValid(registry));
		assert(nextInfo->IsValid(registry));

		// 我们肯定正在更改父级，所以我们必须断开当前的连接 we're definitely changing the parent so we have to disconnect current one
		if (nextInfo->Parent != nullptr)
			DisconnectParent(registry, *next, *nextInfo);

		assert(nextInfo->Parent == nullptr);

		nextInfo->Parent = currInfo->Parent;		// 更新新关系 update the new relationship

		if (currInfo->NextSibling != nullptr)
		{
			auto* currNextInfo = reinterpret_cast<HRelationship*>(currInfo->NextSibling->GetRelationShip());
			currNextInfo->PrevSibling = next;

			//auto* tee = nextInfo->NextSibling->GetRelationShip();
			//std::cout << reinterpret_cast<HRelationship*>(nextInfo->NextSibling->GetRelationShip())->Label << std::endl;
	
			nextInfo->NextSibling = currInfo->NextSibling;
		}
	
		currInfo->NextSibling = next;
		nextInfo->PrevSibling = current;

		if (nextInfo->Parent != nullptr)
		{
			HRelationship* oldParentInfo = reinterpret_cast<HRelationship*>(nextInfo->Parent->GetRelationShip());
			oldParentInfo->ChildrenCount++;
		}
	
		return true;
	}
}