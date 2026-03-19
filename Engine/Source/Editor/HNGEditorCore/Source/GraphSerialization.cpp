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

	bool SaveGraph(const EditorGraph& graph, const std::string& path)
	{
		try
		{
			json root;
			root["version"] = 1;

			// nodes
			root["nodes"] = json::array();
			for (const auto& n : graph.nodes)
			{
				json jn;
				jn["id"] = ToInt(n.id);
				jn["type"] = static_cast<int>(n.type);
				jn["name"] = n.name;
				jn["position"] = SerializeImVec2(n.position);

				// properties
				jn["properties"] = json::object();
				for (const auto& kv : n.properties)
				{
					jn["properties"][kv.first] = kv.second;
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
			(void)root.value("version", 1);

			// nodes
			if (root.contains("nodes") && root["nodes"].is_array())
			{
				for (const auto& jn : root["nodes"])
				{
					EditorNode n;
					n.id = ToNodeId(jn.value("id", 0));
					n.type = static_cast<NodeGraphRuntime::NodeType>(jn.value("type", 0));
					n.name = jn.value("name", std::string{});
					if (jn.contains("position"))
						n.position = DeserializeImVec2(jn["position"]);

					// properties
					n.properties.clear();
					if (jn.contains("properties") && jn["properties"].is_object())
					{
						for (auto it = jn["properties"].begin(); it != jn["properties"].end(); ++it)
						{
							if (it.value().is_string())
								n.properties[it.key()] = it.value().get<std::string>();
							else
								n.properties[it.key()] = it.value().dump();
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

			return graph;
		}
		catch (...)
		{
			// 解析失败：返回空图
			return graph;
		}
	}
}

