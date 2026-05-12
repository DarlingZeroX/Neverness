/*
 * SequenceStateSerializer — JSON 与 SequenceExecutionInstance 互转实现
 */

#include "Runtime/SequenceStateSerializer.h"

#include "Runtime/SequenceExecutionInstance.h"
#include "Runtime/SequenceValue.h"
#include "ExecutorResourceManager.h"
#include "SequenceExecutionContext.h"

namespace VisionGal::GalGame
{
	namespace
	{
		constexpr int kSnapshotSchemaVersion = 1;

		const char* PolicyToString(const SequenceBlockingPolicy p) noexcept
		{
			switch (p)
			{
			case SequenceBlockingPolicy::None: return "None";
			case SequenceBlockingPolicy::WaitAll: return "WaitAll";
			case SequenceBlockingPolicy::WaitAny: return "WaitAny";
			}
			return "WaitAll";
		}

		bool PolicyFromString(const std::string& s, SequenceBlockingPolicy& out) noexcept
		{
			if (s == "None") { out = SequenceBlockingPolicy::None; return true; }
			if (s == "WaitAll") { out = SequenceBlockingPolicy::WaitAll; return true; }
			if (s == "WaitAny") { out = SequenceBlockingPolicy::WaitAny; return true; }
			return false;
		}

		nlohmann::json SerializeParallelGroup(const SequenceParallelGroup& pg)
		{
			nlohmann::json j;
			j["indices"] = pg.ActiveIndices;
			j["policy"] = PolicyToString(pg.Policy);
			j["resumeIndex"] = pg.ResumeSequenceIndex;
			j["slotDispatched"] = pg.SlotHasDispatched;
			nlohmann::json waits = nlohmann::json::array();
			for (const auto& row : pg.SlotWaitTokens)
			{
				nlohmann::json arr = nlohmann::json::array();
				for (const std::uint64_t id : row)
					arr.push_back(id);
				waits.push_back(std::move(arr));
			}
			j["slotWaits"] = std::move(waits);
			return j;
		}

		bool DeserializeParallelGroup(const nlohmann::json& j, SequenceParallelGroup& out)
		{
			if (!j.is_object() || !j.contains("indices") || !j["indices"].is_array())
				return false;
			out.ActiveIndices.clear();
			for (const auto& el : j["indices"])
				out.ActiveIndices.push_back(el.get<std::size_t>());
			if (out.ActiveIndices.empty())
				return false;
			std::string pol = j.value("policy", std::string("WaitAll"));
			if (!PolicyFromString(pol, out.Policy))
				out.Policy = SequenceBlockingPolicy::WaitAll;
			out.ResumeSequenceIndex = j.value("resumeIndex", static_cast<std::size_t>(0));
			out.SlotHasDispatched = j.value("slotDispatched", std::vector<bool>(out.ActiveIndices.size(), false));
			if (out.SlotHasDispatched.size() != out.ActiveIndices.size())
				out.SlotHasDispatched.assign(out.ActiveIndices.size(), false);
			out.SlotWaitTokens.assign(out.ActiveIndices.size(), {});
			if (j.contains("slotWaits") && j["slotWaits"].is_array())
			{
				std::size_t i = 0;
				for (const auto& row : j["slotWaits"])
				{
					if (i >= out.SlotWaitTokens.size())
						break;
					if (row.is_array())
					{
						for (const auto& idj : row)
							out.SlotWaitTokens[i].push_back(idj.get<std::uint64_t>());
					}
					++i;
				}
			}
			return true;
		}
	}

	nlohmann::json SequenceStateSerializer::Save(const SequenceExecutionInstance& inst)
	{
		nlohmann::json root;
		root["schemaVersion"] = kSnapshotSchemaVersion;
		root["state"] = static_cast<int>(inst.m_State);
		root["globalTick"] = inst.m_GlobalTickCounter;

		nlohmann::json frames = nlohmann::json::array();
		for (const SequenceExecutionFrame& fr : inst.m_Frames)
		{
			nlohmann::json jf;
			jf["cursor"] = nlohmann::json{{"sequenceIndex", fr.Cursor.SequenceIndex}};
			jf["hasDispatched"] = fr.HasDispatchedCurrentClip;
			nlohmann::json waits = nlohmann::json::array();
			for (const std::uint64_t id : fr.ActiveWaitTokenIds)
				waits.push_back(id);
			jf["activeWaitTokens"] = std::move(waits);
			if (fr.ParallelGroup.has_value())
				jf["parallel"] = SerializeParallelGroup(*fr.ParallelGroup);
			else
				jf["parallel"] = nullptr;
			frames.push_back(std::move(jf));
		}
		root["frames"] = std::move(frames);

		nlohmann::json vars = nlohmann::json::object();
		for (const auto& kv : inst.m_VariableTable.RawMap())
			vars[kv.first] = kv.second.ToJson();
		root["variables"] = std::move(vars);

		nlohmann::json blob = nlohmann::json::object();
		for (const auto& kv : inst.m_UserStateBlob)
			blob[kv.first] = kv.second;
		root["userBlob"] = std::move(blob);

		root["waitRegistryNext"] = inst.m_WaitRegistry.GetNextTokenIdForSave();
		root["waitRegistryActive"] = inst.m_WaitRegistry.GetActiveTokensForSave();

		if (inst.m_ExecutionContext != nullptr && inst.m_ExecutionContext->ResourceManager != nullptr)
			root["objectIdNext"] = inst.m_ExecutionContext->ResourceManager->GetIdGenerator().GetNextRawIdForSave();
		else
			root["objectIdNext"] = 1u;

		return root;
	}

	void SequenceStateSerializer::Load(SequenceExecutionInstance& inst, const nlohmann::json& data)
	{
		if (!data.is_object())
			return;
		const int ver = data.value("schemaVersion", 0);
		if (ver != kSnapshotSchemaVersion)
			return;

		inst.m_State = static_cast<ESSSequenceExecutorState>(data.value("state", static_cast<int>(ESSSequenceExecutorState::Stopped)));
		inst.m_GlobalTickCounter = data.value("globalTick", static_cast<std::uint64_t>(0));

		inst.m_Frames.clear();
		if (data.contains("frames") && data["frames"].is_array())
		{
			for (const auto& jf : data["frames"])
			{
				SequenceExecutionFrame fr;
				if (jf.contains("cursor") && jf["cursor"].is_object())
					fr.Cursor.SequenceIndex = jf["cursor"].value("sequenceIndex", static_cast<std::size_t>(0));
				fr.HasDispatchedCurrentClip = jf.value("hasDispatched", false);
				if (jf.contains("activeWaitTokens") && jf["activeWaitTokens"].is_array())
				{
					for (const auto& idj : jf["activeWaitTokens"])
						fr.ActiveWaitTokenIds.push_back(idj.get<std::uint64_t>());
				}
				if (jf.contains("parallel") && !jf["parallel"].is_null())
				{
					SequenceParallelGroup pg;
					if (DeserializeParallelGroup(jf["parallel"], pg))
						fr.ParallelGroup = std::move(pg);
				}
				inst.m_Frames.push_back(std::move(fr));
			}
		}
		if (inst.m_Frames.empty())
			inst.m_Frames.push_back(SequenceExecutionFrame{});

		inst.m_VariableTable.Clear();
		if (data.contains("variables") && data["variables"].is_object())
		{
			for (auto it = data["variables"].begin(); it != data["variables"].end(); ++it)
			{
				SequenceValue val;
				if (SequenceValue::TryFromJson(it.value(), val))
					inst.m_VariableTable.Set(it.key(), std::move(val));
			}
		}

		inst.m_UserStateBlob.clear();
		if (data.contains("userBlob") && data["userBlob"].is_object())
		{
			for (auto it = data["userBlob"].begin(); it != data["userBlob"].end(); ++it)
			{
				if (it.value().is_string())
					inst.m_UserStateBlob[it.key()] = it.value().get<std::string>();
			}
		}

		const std::uint64_t nextWait = data.value("waitRegistryNext", static_cast<std::uint64_t>(1));
		std::vector<std::uint64_t> activeWait;
		if (data.contains("waitRegistryActive") && data["waitRegistryActive"].is_array())
		{
			for (const auto& el : data["waitRegistryActive"])
				activeWait.push_back(el.get<std::uint64_t>());
		}
		std::uint64_t nextWaitAdj = nextWait;
		for (const std::uint64_t id : activeWait)
			if (id >= nextWaitAdj)
				nextWaitAdj = id + 1ull;
		inst.m_WaitRegistry.RestoreFromSnapshot(nextWaitAdj, activeWait);

		const std::uint32_t nextOid = data.value("objectIdNext", static_cast<std::uint32_t>(1));
		if (inst.m_ExecutionContext != nullptr && inst.m_ExecutionContext->ResourceManager != nullptr)
			inst.m_ExecutionContext->ResourceManager->GetIdGenerator().RestoreNextRawIdFromSave(nextOid);
	}
}
