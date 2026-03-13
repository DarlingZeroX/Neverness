///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016-2021, Intel Corporation
//
// SPDX-License-Identifier: MIT
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Author(s):  Filip Strugar (filip.strugar@intel.com)
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <vector>
#include <string>
#include "../Core/HCoreTypes.h"
#include "HEntityInterface.h"

namespace Horizon
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// System components
	//
	// Names are not unique and don't have to be part of an entity; registry.ctx<Scene::Name> is always there and specifies scene name
	// Name is the only component (other than Relationship) that gets serialized outside of standard serialization path to enable easier debugging.
	struct HEntityObjectData
	{
		HEntityID ID = 0;
		//std::string Label;

		HEntityInterface* Entity;

		std::vector<std::string> ComponentTypes;
		std::vector<uint64> ComponentIDs;
	};

	//
	// Automatic tag that exists on all entities with a Relationship and no Relationship::Parent
	// struct RelationshipRootTag          { };
	//
	// The parent/child relationship implementation based on "Unconstrained model" from https://skypjack.github.io/2019-06-25-ecs-baf-part-4/ / https://skypjack.github.io/2019-08-20-ecs-baf-part-4-insights/
	// This implementation uses doubly-linked list of siblings which is not circular. First element in the list will be parent's 'FirstChild' and have entt::null for the PrevSibling.
	// IMPORTANT: This component cannot be manipulated in any way other than indirectly through vaScene functions (Create..., SetParent, etc.). It is also manually serialized.
	struct HRelationship
	{
		static const int                c_MaxDepthLevels = 16;                   // Disallow tree depths higher than this - simplifies a lot of things on the implementation side
		static const int                c_MaxDepthValue = c_MaxDepthLevels - 1;   // (max value that Relationship::Depth can have)

		std::string Label;

		//Relationship( entt::entity parent, entt::entity firstChild, entt::entity prevSibling, entt::entity nextSibling, int childrenCount ) : Parent( parent ), FirstChild( firstChild ), PrevSibling( prevSibling ), NextSibling( nextSibling ), ChildrenCount( childrenCount ) { }
		HEntityInterface* Parent = nullptr;
		HEntityInterface* FirstChild = nullptr;
		HEntityInterface* PrevSibling = nullptr;
		HEntityInterface* NextSibling = nullptr;
		int32                           ChildrenCount = 0;
		int32                           Depth = 0;                    // tree depth

		HEntityID ParentID = 0;

		bool IsValid(HECS& registry)
		{
			if (Parent != nullptr && !Parent->IsValid())
				return false;
			if (FirstChild != nullptr && !FirstChild->IsValid())
				return false;
			if (PrevSibling != nullptr && !PrevSibling->IsValid())
				return false;
			if (NextSibling != nullptr && !NextSibling->IsValid())
				return false;
			if (Parent == nullptr && (PrevSibling != nullptr || NextSibling != nullptr))
				return false;
			if ((FirstChild != nullptr && ChildrenCount == 0) || (FirstChild == nullptr && ChildrenCount != 0))
				return false;
			if (Depth > c_MaxDepthValue)
				return false;
			return true;
		}

		bool operator == (const HRelationship& other) { return this->Parent == other.Parent && this->FirstChild == other.FirstChild && this->PrevSibling == other.PrevSibling && this->NextSibling == other.NextSibling && this->ChildrenCount == other.ChildrenCount && this->Depth == other.Depth; }
	};
}
