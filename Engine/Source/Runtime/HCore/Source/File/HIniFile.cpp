#include "pch.h"
#include "File/HIniFile.h"
#include <string>
#include "File/include/SimpleIni.h"
#include "HCoreTypes.h"

namespace Horizon
{
	using HSimpleIniA = CSimpleIniA;

	//class HIniFile;



	struct HIniTypeHandler
	{
		virtual  int SetValue(HSimpleIniA& file, const char* a_pSection,
			const char* a_pKey,
			const META_NAMESPACE::Variant& a_bValue,
			const char* a_pComment = nullptr,
			bool            a_bForceReplace = false
		) = 0;

		virtual bool GetValue(HSimpleIniA& file, const char* a_pSection,
			const char* a_pKey,
			const META_NAMESPACE::FieldInfo& a_bValue,
			Meta::Variant& instance,
			bool* a_pHasMultiple = nullptr
		) = 0;
	};

	class CORE_MODULE_API HIniTypeManager
	{
	public:
		HIniTypeManager();

		static HIniTypeManager& Instance();

		HIniTypeHandler* TryGetHandler(const type_info* info);
	private:
		unordered_type_map<Ref<HIniTypeHandler>> m_TypeHandler;
	};

	struct HIniBooleanHandler : public HIniTypeHandler
	{
		int SetValue(HSimpleIniA& file, const char* a_pSection, const char* a_pKey, const Horizon::Meta::Variant& a_bValue,
			const char* a_pComment, bool a_bForceReplace) override
		{
			const auto value = a_bValue.ToBool();

			//std::cout << a_pKey << ": " << value << std::endl;

			return file.SetBoolValue(a_pSection, a_pKey, value, a_pComment, a_bForceReplace);
		}

		bool GetValue(HSimpleIniA& file, const char* a_pSection, const char* a_pKey, const Meta::FieldInfo& a_bValue,
			Meta::Variant& instance, bool* a_pHasMultiple) override
		{
			bool value = file.GetBoolValue(a_pSection, a_pKey, 0, a_pHasMultiple);

			a_bValue.SetVariantValue(instance, value);

			return true;
		}
	};

	struct HIniStringHandler : public HIniTypeHandler
	{
		int SetValue(HSimpleIniA& file, const char* a_pSection, const char* a_pKey, const Horizon::Meta::Variant& a_bValue,
			const char* a_pComment, bool a_bForceReplace) override
		{
			const auto value = a_bValue.ToString();

			//std::cout << a_pKey << ": " << value << std::endl;

			return file.SetValue(a_pSection, a_pKey, value.c_str(), a_pComment, a_bForceReplace);
		}

		bool GetValue(HSimpleIniA& file, const char* a_pSection, const char* a_pKey, const Meta::FieldInfo& a_bValue,
			Meta::Variant& instance, bool* a_pHasMultiple) override
		{
			std::string value = file.GetValue(a_pSection, a_pKey, "", a_pHasMultiple);

			a_bValue.SetVariantValue(instance, value);

			return true;
		}
	};

	struct HIniDoubleHandler : public HIniTypeHandler
	{
		int SetValue(HSimpleIniA& file, const char* a_pSection, const char* a_pKey, const Horizon::Meta::Variant& a_bValue,
			const char* a_pComment, bool a_bForceReplace) override
		{
			const auto value = a_bValue.ToDouble();

			//std::cout << a_pKey << ": " << value << std::endl;

			return file.SetDoubleValue(a_pSection, a_pKey, value, a_pComment, a_bForceReplace);
		}

		bool GetValue(HSimpleIniA& file, const char* a_pSection, const char* a_pKey, const Meta::FieldInfo& a_bValue,
			Meta::Variant& instance, bool* a_pHasMultiple) override
		{
			double value = file.GetDoubleValue(a_pSection, a_pKey, .0, a_pHasMultiple);

			a_bValue.SetVariantValue(instance, value);

			return true;
		}
	};

	struct HIniLongHandler : public HIniTypeHandler
	{
		int SetValue(HSimpleIniA& file, const char* a_pSection, const char* a_pKey, const Horizon::Meta::Variant& a_bValue,
			const char* a_pComment, bool a_bForceReplace) override
		{
			const auto value = a_bValue.ToLong();

			//std::cout << a_pKey << ": " << value << std::endl;

			return file.SetLongValue(a_pSection, a_pKey, value, a_pComment, a_bForceReplace);
		}

		bool GetValue(HSimpleIniA& file, const char* a_pSection, const char* a_pKey, const Meta::FieldInfo& a_bValue,
			Meta::Variant& instance, bool* a_pHasMultiple) override
		{
			long value = file.GetLongValue(a_pSection, a_pKey, 0, a_pHasMultiple);

			a_bValue.SetVariantValue(instance, value);

			return true;
		}
	};

	class HIniFile::Internal : public HSimpleIniA
	{
	public:
		Internal(void)
		{
			
		}

		int SaveFileInPath(const std::filesystem::path& path)
		{
			return SaveFile(path.c_str());
		}

		static Ref<Internal> LoadFromFile(const std::filesystem::path& path)
		{
			auto file = CreateRef<Internal>();

			file->LoadFile(path.c_str());

			return file;
		}

		static Ref<HIniFile::Internal> SerializeType(const META_NAMESPACE::Type& type, const META_NAMESPACE::Variant& variant)
		{
			auto file = CreateRef<HIniFile::Internal>();

			auto typeName = type.GetName();
			auto* sectionName = typeName.data();

			std::string temp = "[";
			temp += typeName;
			temp += "]";

			file->LoadData(temp);

			for (const auto& field : type.GetFields())
			{
				auto* fieldName = field.GetName().data();
				auto filedValue = field.GetValue(variant);

				auto* handler = HIniTypeManager::Instance().TryGetHandler(field.GetType().GetTypeInfo());

				if (handler != nullptr)
				{
					handler->SetValue(*file, sectionName, fieldName, filedValue);
				}
				else if (field.GetType().IsEnum())
				{
					file->SetLongValue(sectionName, fieldName, filedValue.ToLong());
				}
				else
				{
					throw "Unsupported type";
				}
			}

			return file;
		}

		int DeserializeType(const META_NAMESPACE::Type& type, META_NAMESPACE::Variant& variant)
		{
			auto typeName = type.GetName();
			auto* sectionName = typeName.data();

			for (const auto& field : type.GetFields())
			{
				auto* fieldName = field.GetName().data();
				auto filedValue = field.GetValue(variant);

				auto* handler = HIniTypeManager::Instance().TryGetHandler(field.GetType().GetTypeInfo());

				if (handler != nullptr)
				{
					handler->GetValue(*this, sectionName, fieldName, field, variant);
				}
				else if (field.GetType().IsEnum())
				{
					long value = 0;
					this->GetLongValue(sectionName, fieldName, value);
					field.SetVariantValue(variant, value);
				}
				else
				{
					throw "Unsupported type";
				}
			}

			return 0;
		}

	};

	HIniTypeManager::HIniTypeManager()
	{
		auto boolHandler = CreateRef<HIniBooleanHandler>();
		auto stringHandler = CreateRef<HIniStringHandler>();
		auto doubleHandler = CreateRef<HIniDoubleHandler>();
		auto longHandler = CreateRef<HIniLongHandler>();

		m_TypeHandler[&typeid(bool)] = boolHandler;
		m_TypeHandler[&typeid(std::string)] = stringHandler;
		m_TypeHandler[&typeid(float)] = doubleHandler;
		m_TypeHandler[&typeid(double)] = doubleHandler;
		m_TypeHandler[&typeid(uint32)] = longHandler;
		m_TypeHandler[&typeid(int32)] = longHandler;
		m_TypeHandler[&typeid(int)] = longHandler;
	}

	HIniTypeManager& HIniTypeManager::Instance()
	{
		static HIniTypeManager manager;
		 
		return manager;
	}

	HIniTypeHandler* HIniTypeManager::TryGetHandler(const type_info* info)
	{
		auto result = m_TypeHandler.find(info);

		if (result != m_TypeHandler.end())
			return result->second.get();

		return nullptr;
	}

	HIniFile::HIniFile()
	{
		m_Impl = CreateScope<HIniFile::Internal>();

		m_Impl->SetUnicode();
	}

	HIniFile::HIniFile(Ref<Internal> impl)
		:m_Impl(impl)
	{
	}

	int HIniFile::SaveFileInPath(const std::filesystem::path& path)
	{
		return m_Impl->SaveFile(path.c_str());
	}

	Ref<HIniFile> HIniFile::LoadFromFile(const std::filesystem::path& path)
	{
		auto file = CreateRef<HIniFile>();

		file->m_Impl->LoadFile(path.c_str());

		return file;
	}

	Ref<HIniFile> HIniFile::SerializeType(const Horizon::Meta::Type& type, const META_NAMESPACE::Variant& variant)
	{
		return CreateRef<HIniFile>(Internal::SerializeType(type, variant)) ;
	}

	int HIniFile::DeserializeType(const Meta::Type& type, Meta::Variant& variant)
	{
		return m_Impl->DeserializeType(type, variant);
	}

}
