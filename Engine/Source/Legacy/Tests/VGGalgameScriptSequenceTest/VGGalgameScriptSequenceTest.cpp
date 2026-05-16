/*
 * VGGalgameSequenceRuntime 内核测试：Save → Tick → Load → 状态对齐
 */

#include <gtest/gtest.h>

#include <NNCore/Include/File/nlohmann/json.hpp>

#include "VGGalgameSequenceRuntime/Include/Runtime/SequenceExecutionInstance.h"
#include "VGGalgameSequenceRuntime/Include/Runtime/SequenceStateSerializer.h"
#include "VGGalgameSequenceRuntime/Include/SequenceExecutionContext.h"
#include "VGGalgameSequenceRuntime/Include/Sequence/DataContainer.h"
#include "VGGalgameSequenceRuntime/Include/ExecutorResourceManager.h"

using namespace VisionGal;
using namespace VisionGal::GalGame;

TEST(SequenceStateSerializer, SaveTickLoadPreservesCursorAndTick)
{
	SSSequenceExecutionContext ctx;
	ctx.SequenceData = MakeRef<VGSSequenceDataContainer>();
	ctx.ResourceManager = MakeRef<SSExecutorResourceManager>();
	/// 空剪辑条目：无 RuntimeSystem 匹配，单 Tick 内跳过并前进一条。
	for (int i = 0; i < 5; ++i)
		ctx.SequenceData->m_Sequence.push_back(nullptr);

	SequenceExecutionInstance a;
	a.SetExecutionContext(&ctx);
	a.Play();
	a.Tick(0.016f, nullptr);

	const nlohmann::json snapshot = SequenceStateSerializer::Save(a);
	ASSERT_TRUE(snapshot.is_object());
	ASSERT_EQ(snapshot.value("schemaVersion", 0), 1);

	SequenceExecutionInstance b;
	b.SetExecutionContext(&ctx);
	SequenceStateSerializer::Load(b, snapshot);

	EXPECT_EQ(b.GetCurrentSequenceIndex(), a.GetCurrentSequenceIndex());
	EXPECT_EQ(b.GetGlobalTickCounter(), a.GetGlobalTickCounter());
	EXPECT_EQ(b.GetState(), a.GetState());
}

TEST(SequenceStateSerializer, LoadThenContinueClearsSerializedWait)
{
	SSSequenceExecutionContext ctx;
	ctx.SequenceData = MakeRef<VGSSequenceDataContainer>();
	ctx.ResourceManager = MakeRef<SSExecutorResourceManager>();
	ctx.SequenceData->m_Sequence.push_back(nullptr);

	nlohmann::json j;
	j["schemaVersion"] = 1;
	j["state"] = static_cast<int>(ESSSequenceExecutorState::Playing);
	j["globalTick"] = 10;
	j["frames"] = nlohmann::json::array();
	{
		nlohmann::json fr;
		fr["cursor"] = nlohmann::json::object({{"sequenceIndex", 0}});
		fr["hasDispatched"] = true;
		fr["activeWaitTokens"] = nlohmann::json::array({1});
		fr["parallel"] = nullptr;
		j["frames"].push_back(fr);
	}
	j["variables"] = nlohmann::json::object();
	j["userBlob"] = nlohmann::json::object();
	j["waitRegistryNext"] = 2;
	j["waitRegistryActive"] = nlohmann::json::array({1});
	j["objectIdNext"] = ctx.ResourceManager->GetIdGenerator().GetNextRawIdForSave();

	SequenceExecutionInstance inst;
	inst.SetExecutionContext(&ctx);
	SequenceStateSerializer::Load(inst, j);
	EXPECT_TRUE(inst.IsWaiting());

	inst.Continue(nullptr);
	EXPECT_FALSE(inst.IsWaiting());
}
