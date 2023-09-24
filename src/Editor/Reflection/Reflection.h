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
    using DumpFuncType = void(*)(Self& instance, string& serialized); \
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
	template<unsigned long long int N> struct TypeNameWrapper{ };

	template<typename T, typename Class, typename... Parents>
	void DumpIterative(T& instance, string& serialized)
	{
		for (auto* field_getter : Class::Reflection_GetterFuncPtrs)
		{
			serialized.append(" ");
			serialized.append(field_getter(instance));
			serialized.append(" ");
		}

		if constexpr (sizeof...(Parents) > 0)
		{
			DumpIterative<T, Parents...>(instance, serialized);
		}
	}

	template<typename T, typename Class, typename... Parents>
	void LoadIterative(T& instance, string& field, string& value)
	{
		if (auto it = Class::Reflection_SetterFuncPtrs.find(field); it != Class::Reflection_SetterFuncPtrs.end())  
		{ 
			auto* setter_method = it->second;
			setter_method(&instance, value);
		}
		else
		{
			if constexpr (sizeof...(Parents) > 0)
			{
				LoadIterative<T, Parents...>(instance, field, value);
			}
			else
				fatal_error("PROBLEM: We couldn't find the appropriate setter for the field '%s' with value '%s' anywhere.", field.c_str(), value.c_str());
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
	string Dump(T& instance, bool include_header = true)
	{
		string serialized;
		serialized.reserve(SerializationSize);
		
		string header = "";
		string body = "";
	    if (include_header)
	    {
			string name; 
    		name.reserve(100);
	    	name = instance.Reflection_InstanceName != nullptr ? "NONAME" : *instance.Reflection_InstanceName;
    		sprintf(&header[0], "{ \"%s\" : %s = {", name.c_str(), T::Reflection_TypeName.c_str());
	    }
	    else
	    {
	        // hack to make deserialization of nested objects work with current parsing code
	        body = "{,";
	    }

		serialized = header.append(body);
	    T::Reflection_DumpFunc(instance, serialized);

		serialized.append(" } }\n");
	    return serialized;
	}

	// ---------------------------------------

	void ParseObject(string& data, map<string, string>& field_value_map, bool include_header);

	template<typename T>
	T Load(string& data, bool include_header = true)
	{
		map<string, string> fields;
		ParseObject(data, fields, include_header);
	    
		T instance;
		for (auto& [field, value] : fields)
		{
			T::Reflection_LoadFunc(instance, static_cast<string&>(field), static_cast<string&>(value));
		}

		return instance;
	}
	
	// ---------------------------------------

	// default template for serialization of fields
	template<typename TField>
	auto ToString(TField& field) -> decltype(std::to_string(field), string())
	{
	    return std::to_string(field);
	};

	// specialization for UserTypes (ReflectionTypes)
	template<typename TField, std::enable_if_t<std::is_class_v<TField>, bool> = true>
	auto ToString(TField& field) -> decltype(Dump<TField>(field), string())
	{
	    return Dump<TField>(field, false);
	};
	string ToString(string& field);
	string ToString(char& field);
	string ToString(bool& field);
	
	// ---------------------------------------

	template<typename T>
	T FromString(string& value)
	{
	    return Load<T>(value, false);
	}

	// ---------------------------------------
}

inline string GetLine(string& text, char delimiter)
{
	uint pos = text.find(delimiter);
	if (pos == std::string::npos)
		return "";
	string line = text.substr(0, pos);
	text.erase(0, pos + 1);
	return line;
}


/*
 * Handle<T> - Stores type and UUID for the resource it points to.
 * Stores the actual pointer to the asset instance and a generative version number.
 * The pointer is private, we can access it by using Get(), which will compare the version number in the handle
 * with the version number the asset manager / world has for that "slot".
 */

struct AssetCatalogue
{
    uint asset_instance_size = 0;
    void* instance_array;
    uint* version_list;
    uint instance_count = 0;

    AssetCatalogue(uint asset_instance_size,  void* instance_array, uint array_size, uint* version_list) : 
        asset_instance_size(asset_instance_size),
        instance_array(instance_array),
        version_list(version_list),
        instance_count(array_size)
    {

    }

    template<typename T>
    T* CheckVersion(T* ptr, uint version)
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