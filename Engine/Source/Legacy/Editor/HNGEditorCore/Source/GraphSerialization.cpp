/*
* EditorGraph JSON 序列化 / 反序列化实现
*/

#include "GraphSerialization.h"

#include <fstream>
#include <utility>

// 项目内置的 nlohmann::json 单头文件
#include <HCore/Include/File/nlohmann/json.hpp>

namespace Horizon::NodeGraphEditor
{
	using json = nlohmann::json;

	static int ToInt(ax::NodeEditor::NodeId id) { return id.Get(); }
	static int ToInt(ax::NodeEditor::PinId id) { return id.Get(); }
	static int ToInt(ax::NodeEditor::LinkId id) { return id.Get(); }

	static ax::NodeEditor::NodeId ToNodeId(int v) { return ax::NodeEditor::NodeId(v); }
	static ax::NodeEditor::PinId ToPinId(int v) { return ax::NodeEditor::PinId(v); }
	static ax::NodeEditor::LinkId ToLinkId(int v) { return ax::NodeEditor::LinkId(v); }

	static json SerializeImVec2(const ImVec2& v)
	{
		return json{ {"x", v.x}, {"y", v.y} };
	}

	static ImVec2 DeserializeImVec2(const json& j)
	{
		ImVec2 v{};
		v.x = j.value("x", 0.0f);
		v.y = j.value("y", 0.0f);
		return v;
	}

	static json SerializeImColor(const ImColor& c)
	{
		// ImColor 内部可隐式转 ImVec4（RGBA float）
		ImVec4 v = c.Value;
		return json{ {"r", v.x}, {"g", v.y}, {"b", v.z}, {"a", v.w} };
	}

	static ImColor DeserializeImColor(const json& j)
	{
		ImVec4 v{};
		v.x = j.value("r", 1.0f);
		v.y = j.value("g", 1.0f);
		v.z = j.value("b", 1.0f);
		v.w = j.value("a", 1.0f);
		return ImColor(v);
	}

	static json SerializeValue(const NodeGraphRuntime::Value& v)
	{
		using namespace NodeGraphRuntime;
		switch (v.type)
		{
		case ValueType::Int:
			return json{ {"type", "Int"}, {"value", v.AsInt()} };
		case ValueType::Float:
			return json{ {"type", "Float"}, {"value", v.AsFloat()} };
		case ValueType::Bool:
			return json{ {"type", "Bool"}, {"value", v.AsBool()} };
		case ValueType::String:
			return json{ {"type", "String"}, {"value", v.AsString()} };
		default:
			return json{ {"type", "None"} };
		}
	}

	static NodeGraphRuntime::Value DeserializeValue(const json& j)
	{
		using namespace NodeGraphRuntime;
		if (j.is_object())
		{
			const std::string type = j.value("type", "None");
			if (type == "Int") return Value::FromInt(static_cast<int>(j.value("value", 0)));
			if (type == "Float") return Value::FromFloat(j.value("value", 0.0f));
			if (type == "Bool") return Value::FromBool(j.value("value", false));
			if (type == "String") return Value::FromString(j.value("value", std::string{}));
			return Value{};
		}

		// 兼容旧版：properties 中原先只支持 string
		if (j.is_string()) return Value::FromString(j.get<std::string>());
		if (j.is_boolean()) return Value::FromBool(j.get<bool>());
		if (j.is_number_integer()) return Value::FromInt(static_cast<int>(j.get<int64_t>()));
		if (j.is_number_float()) return Value::FromFloat(j.get<float>());

		return Value{};
	}

	bool SaveGraph(const EditorGraph& graph, const std::string& path)
	{
		try
		{
			json root;
			root["version"] = 2;
			// 保存下一次生成用的 id state，避免重载后 ID 复用
			{
				const GraphIdState st = graph.idGen.GetState();
				root["id_state"] = json{
					{"nextNodeId", st.nextNodeId},
					{"nextPinId", st.nextPinId},
					{"nextLinkId", st.nextLinkId}
				};
			}

			// nodes
			root["nodes"] = json::array();
			for (const auto& n : graph.nodes)
			{
				json jn;
				jn["id"] = ToInt(n.id);
				jn["type"] = static_cast<int>(n.typeId);
				jn["name"] = n.name;
				jn["position"] = SerializeImVec2(n.position);

				// properties
				jn["properties"] = json::object();
				for (const auto& kv : n.properties)
				{
					jn["properties"][kv.first] = SerializeValue(kv.second);
				}

				// pins
				auto serializePins = [](const std::vector<EditorPin>& pins)
				{
					json arr = json::array();
					for (const auto& p : pins)
					{
						json jp;
						jp["id"] = ToInt(p.id);
						jp["name"] = p.name;
						jp["slotType"] = static_cast<int>(p.type);
						jp["isInput"] = p.isInput;
						jp["color"] = SerializeImColor(p.color);
						jp["runtimeIndex"] = p.runtimeIndex;
						arr.push_back(std::move(jp));
					}
					return arr;
				};

				jn["inputs"] = serializePins(n.inputs);
				jn["outputs"] = serializePins(n.outputs);

				root["nodes"].push_back(std::move(jn));
			}

			// links
			root["links"] = json::array();
			for (const auto& l : graph.links)
			{
				json jl;
				jl["id"] = ToInt(l.id);
				jl["startPinId"] = ToInt(l.startPinId);
				jl["endPinId"] = ToInt(l.endPinId);
				root["links"].push_back(std::move(jl));
			}

			std::ofstream ofs(path, std::ios::out | std::ios::trunc);
			if (!ofs.is_open())
				return false;

			ofs << root.dump(2); // pretty print
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	EditorGraph LoadGraph(const std::string& path)
	{
		EditorGraph graph;
		graph.nodes.clear();
		graph.links.clear();
		graph.context = nullptr;
		graph.registry = nullptr;
		graph.dirty = true;

		try
		{
			std::ifstream ifs(path, std::ios::in);
			if (!ifs.is_open())
				return graph;

			json root = json::parse(ifs, nullptr, true, true);

			// version（目前仅保留字段，后续可按版本迁移）
			(void)root.value("version", 2);

			// id_state：优先加载生成器状态
			if (root.contains("id_state") && root["id_state"].is_object())
			{
				const auto& js = root["id_state"];
				GraphIdState loaded{};
				loaded.nextNodeId = js.value("nextNodeId", loaded.nextNodeId);
				loaded.nextPinId = js.value("nextPinId", loaded.nextPinId);
				loaded.nextLinkId = js.value("nextLinkId", loaded.nextLinkId);
				graph.idGen.Reset(loaded);
			}

			// nodes
			if (root.contains("nodes") && root["nodes"].is_array())
			{
				for (const auto& jn : root["nodes"])
				{
					EditorNode n;
					n.id = ToNodeId(jn.value("id", 0));
					n.typeId = static_cast<NodeGraphRuntime::NodeTypeId>(jn.value("type", 0));
					n.name = jn.value("name", std::string{});
					if (jn.contains("position"))
						n.position = DeserializeImVec2(jn["position"]);

					// properties
					n.properties.clear();
					if (jn.contains("properties") && jn["properties"].is_object())
					{
						for (auto it = jn["properties"].begin(); it != jn["properties"].end(); ++it)
						{
							n.properties[it.key()] = DeserializeValue(it.value());
						}
					}

					// pins
					auto deserializePins = [](const json& arr, bool isInput)
					{
						std::vector<EditorPin> pins;
						if (!arr.is_array()) return pins;
						pins.reserve(arr.size());
						for (const auto& jp : arr)
						{
							EditorPin p;
							p.id = ToPinId(jp.value("id", 0));
							p.name = jp.value("name", std::string{});
							p.type = static_cast<NodeGraphRuntime::SlotType>(jp.value("slotType", 0));
							p.isInput = jp.value("isInput", isInput);
							if (jp.contains("color"))
								p.color = DeserializeImColor(jp["color"]);
							p.runtimeIndex = jp.value("runtimeIndex", 0u);
							pins.push_back(std::move(p));
						}
						return pins;
					};

					if (jn.contains("inputs"))  n.inputs = deserializePins(jn["inputs"], true);
					if (jn.contains("outputs")) n.outputs = deserializePins(jn["outputs"], false);

					graph.nodes.push_back(std::move(n));
				}
			}

			// links
			if (root.contains("links") && root["links"].is_array())
			{
				for (const auto& jl : root["links"])
				{
					EditorLink l;
					l.id = ToLinkId(jl.value("id", 0));
					l.startPinId = ToPinId(jl.value("startPinId", 0));
					l.endPinId = ToPinId(jl.value("endPinId", 0));
					graph.links.push_back(std::move(l));
				}
			}

			// 冲突修复：确保 idGen.next* 至少大于图中最大已存在 id
			graph.FixupIdStateAfterLoad();

			return graph;
		}
		catch (...)
		{
			// 解析失败：返回空图
			return graph;
		}
	}
}

