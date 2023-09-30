#pragma once

#include "engine/core/core.h"

/* ------------------------------------------------------------------------------------------------ 
* CLEANUP AND WRAPUP
* 1. Make sure that we can dump and load an entity with multiple parents with serializable data.
    . Remove T::Super, replace the mechanism of detecting if class inherits from Reflection base class in another way
    . Try making T::Self be automatically derived from decltype(*this) or some other trick.
    . Replace the code that used to check for fields in T::Super for the iterative templated version
* 5. Make sure we can pass a memory location for deserialization
* 
* INTEGRATION
* 1. Create Handle<T> types in engine so we can serialize handle metadata to deal with ptrs serialization
* ------------------------------------------------------------------------------------------------ */

#define STORE_TYPE_IN_HELPER(TypeName)	\
	template<> \
	struct Reflection::TypeNameWrapper<__COUNTER__> \
	{ \
		using T = TypeName; \
		static inline string type_name = #TypeName; \
	}

#define Reflected() \
    using Self = Reflection::TypeNameWrapper<__COUNTER__ - 1>::T; \
    using DumpFuncType = void(*)(Self& instance, string& Serialized); \
	using LoadFuncType = void(*)(Self& instance, string& field, string& value); \
	using GetterFuncPtrType = string(*)(Self&); \
	using SetterFuncPtrType = void (*)(Self*, string&); \
	inline static string Reflection_TypeName = Reflection::TypeNameWrapper<__COUNTER__ - 2>::type_name; \
    inline static DumpFuncType Reflection_DumpFunc = nullptr; \
    inline static LoadFuncType Reflection_LoadFunc = nullptr; \
    inline static vector<GetterFuncPtrType> Reflection_GetterFuncPtrs; \
    inline static map<string, SetterFuncPtrType> Reflection_SetterFuncPtrs; \
    string* Reflection_InstanceName = nullptr; \
    string Reflection_SetName(string& name_var, string&& default_name) \
    { \
        Reflection_InstanceName = &name_var; \
        return default_name; \
    }

#define Field(Type, Name, ...) ;  \
    inline static string Reflection_Getter_##Type_##Name(Self& instance) \
    { \
        string name; \
        sprintf(&name[0], "'%s' : %s = %s", #Name, #Type, Reflection::ToString(instance.Name)); \
        return name; \
    } \
    \
    inline static void Reflection_Setter_##Type_##Name(Self* instance, string& value) \
    { \
        instance->Name = Reflection::FromString<Type>(value); \
    } \
    \
    inline static struct Reflection_HelperType_##Type_##Name \
    { \
        Reflection_HelperType_##Type_##Name() \
        { \
            Reflection_GetterFuncPtrs.push_back(&Self::Reflection_Getter_##Type_##Name); \
            Reflection_SetterFuncPtrs[#Name] = &Self::Reflection_Setter_##Type_##Name; \
        } \
    } reflection_discard_##Type_##Name{}; \
    __VA_ARGS__ Type Name

#define Name(Name, Default); Field(string, Name) = Reflection_SetName(Name, Default);


namespace Reflection
{
	template<unsigned long long int N> struct TypeNameWrapper
	{ };

	template<typename T, typename Class, typename... Parents>
	void DumpIterative(T& Instance, string& Serialized)
	{
		for (auto* FieldGetter : Class::Reflection_GetterFuncPtrs)
		{
			Serialized.append(" ");
			Serialized.append(FieldGetter(Instance));
			Serialized.append(" ");
		}

		if constexpr (sizeof...(Parents) > 0)
		{
			DumpIterative<T, Parents...>(Instance, Serialized);
		}
	}

	template<typename T, typename Class, typename... Parents>
	void LoadIterative(T& Instance, string& Field, string& Value)
	{
		if (auto It = Class::Reflection_SetterFuncPtrs.find(Field); It != Class::Reflection_SetterFuncPtrs.end())
		{
			auto* SetterMethod = It->second;
			SetterMethod(&Instance, Value);
		}
		else
		{
			if constexpr (sizeof...(Parents) > 0)
			{
				LoadIterative<T, Parents...>(Instance, Field, Value);
			}
			else
				fatal_error("PROBLEM: We couldn't find the appropriate setter for the field '%s' with value '%s' anywhere.", Field.c_str(), Value.c_str());
		}
	}

	template<typename T, typename... Parents>
	struct ReflectionGetterSetter
	{
		ReflectionGetterSetter()
		{
			T::Reflection_DumpFunc = &DumpIterative<T, T, Parents...>;
			T::Reflection_LoadFunc = &LoadIterative<T, T, Parents...>;
		}
	};

	constexpr static uint SerializationSize = 1600;

	template<typename T>
	string Dump(T& Instance, bool bIncludeHeader = true)
	{
		string Serialized;
		Serialized.reserve(SerializationSize);

		string Header = "";
		string Body = "";
		if (bIncludeHeader)
		{
			string Name;
			Name.reserve(100);
			Name = Instance.Reflection_InstanceName != nullptr ? "NONAME" : *Instance.Reflection_InstanceName;
			sprintf(&Header[0], "{ \"%s\" : %s = {", Name.c_str(), T::Reflection_TypeName.c_str());
		}
		else
		{
			// hack to make deserialization of nested objects work with current parsing code
			Body = "{,";
		}

		Serialized = Header.append(Body);
		T::Reflection_DumpFunc(Instance, Serialized);

		Serialized.append(" } }\n");
		return Serialized;
	}

	// ---------------------------------------

	void ParseObject(string& Data, map<string, string>& FieldValueMap, bool IncludeHeader);

	template<typename T>
	T Load(string& Data, bool IncludeHeader = true)
	{
		map<string, string> Fields;
		ParseObject(Data, Fields, IncludeHeader);

		T Instance;
		for (auto& [field, value] : Fields)
		{
			T::Reflection_LoadFunc(Instance, static_cast<string&>(field), static_cast<string&>(value));
		}

		return Instance;
	}

	// ---------------------------------------

	// default template for serialization of fields
	template<typename TField>
	auto ToString(TField& Field) -> decltype(std::to_string(Field), string())
	{
		return std::to_string(Field);
	};

	// specialization for UserTypes (ReflectionTypes)
	template<typename TField, std::enable_if_t<std::is_class_v<TField>, bool>  = true>
	auto ToString(TField& Field) -> decltype(Dump<TField>(Field), string())
	{
		return Dump<TField>(Field, false);
	};
	string ToString(string& Field);
	string ToString(char& Field);
	string ToString(bool& Field);

	// ---------------------------------------

	template<typename T>
	T FromString(string& Value)
	{
		return Load<T>(Value, false);
	}

	// ---------------------------------------
}

inline string GetLine(string& Text, char DelimiterChar)
{
	uint Pos = Text.find(DelimiterChar);
	if (Pos == std::string::npos)
		return "";
	string Line = Text.substr(0, Pos);
	Text.erase(0, Pos + 1);
	return Line;
}


/*
 * Handle<T> - Stores type and UUID for the resource it points to.
 * Stores the actual pointer to the asset instance and a generative version number.
 * The pointer is private, we can access it by using Get(), which will compare the version number in the handle
 * with the version number the asset manager / world has for that "slot".
 */

struct AssetCatalogue
{
	uint AssetInstanceSize = 0;
	void* InstanceArray;
	uint* VersionList;
	uint InstanceCount = 0;

	AssetCatalogue(uint AssetInstanceSize, void* InstanceArray, uint ArraySize, uint* VersionList) :
		AssetInstanceSize(AssetInstanceSize),
		InstanceArray(InstanceArray),
		VersionList(VersionList),
		InstanceCount(ArraySize)
	{ }

	template<typename T>
	T* CheckVersion(T* Ptr, uint Version)
	{
		// do ptr arithmetic to get version list index and compare versions
		return nullptr;
	}
};

/*
struct Shader{};

struct ShaderCatalogue : public AssetCatalogue
{
    inline static constexpr u32 shader_list_size = 32;
    u32 version_list[shader_list_size];
    Shader shader_list[shader_list_size];

    ShaderCatalogue() : AssetCatalogue(sizeof(Shader), &shader_list[0], shader_list_size, &version_list[0])
    {
        // INIT lists
    }


    Shader* GetAssetInstance(Shader* ptr, u32 version)
    {
        auto* shader = CheckVersion<Shader>(ptr, version);
        if (shader)
            return shader;
        
        // load shader
    }

};


template<typename T>
struct Handle
{
    T* Get()
    {
        return get_t(ptr, version);
    };

    template<typename TCatalogue>
    explicit Handle(T* instance, unsigned int version, TCatalogue* catalogue) : ptr(instance), version(version)
    {
        get_t = catalogue->GetAssetInstance;
    }

private:
    T* ptr;
    unsigned int version;
    T*(*get_t)(T* ptr, u32 version);
};

*/
