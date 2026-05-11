/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Events/SequenceEditorEvent.h"

#include <memory>
#include <string>

namespace VisionGal::Editor
{
	class SequenceDocument;
	class SequenceDocumentViewModel;
	class SequenceExecutionController;
	class SequenceSelectionModel;
	class SequenceUndoStack;
	class SequenceClipboard;
	class SequenceInspectorRegistry;
	class SequenceValidationRegistry;
	class ISequenceEditorCommand;
	struct SequenceRuntimeSnapshot;
	struct SequenceRuntimeOverlayState;
	class SequenceEditorEventBus;
	struct SequenceEditorServiceLocator;
	class SequenceValidationCacheService;

	using SequenceDocumentMutationSink = void (*)(void* userData, const SequenceDocumentMutationSummary& summary);

	struct SequenceEditorContext
	{
		SequenceDocument* document = nullptr;
		SequenceExecutionController* execution = nullptr;
		SequenceSelectionModel* selection = nullptr;
		SequenceUndoStack* undo = nullptr;
		SequenceClipboard* clipboard = nullptr;
		SequenceInspectorRegistry* inspectorRegistry = nullptr;

		/// Rebuilt by host presentation tick; list / timeline / outliner read rows from here.
		SequenceDocumentViewModel* documentViewModel = nullptr;

		SequenceValidationRegistry* validationRegistry = nullptr;
		SequenceValidationCacheService* validationCache = nullptr;

		const SequenceRuntimeOverlayState* runtimeOverlay = nullptr;

		const std::string* searchFilter = nullptr;

		SequenceRuntimeSnapshot* lastExecutionSnapshot = nullptr;

		bool (*executeToEntry)(void* userData, unsigned index) = nullptr;
		void* executeToUserData = nullptr;

		SequenceEditorEventBus* eventBus = nullptr;
		SequenceEditorServiceLocator* services = nullptr;

		SequenceDocumentMutationSink onDocumentMutationAccumulate = nullptr;
		void* onDocumentMutationAccumulateUserData = nullptr;

		void (*requestPresentationRefresh)(void* userData) = nullptr;
		void* requestPresentationRefreshUserData = nullptr;

		void ExecuteCommand(std::unique_ptr<ISequenceEditorCommand> command);
		void UndoDocument();
		void RedoDocument();

		void NotifyDocumentChanged(const SequenceDocumentMutationSummary& summary);
		void RequestPresentationRefresh();
	};
}
