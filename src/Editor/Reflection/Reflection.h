#pragma once

#include "engine/core/core.h"

/* =============================================================================================
* Reflection.h:
*	A (not so) simple native C++, header only reflection library. Not standalone, is tightly
*	knit with the Entity types to achieve reflection metadata to be used by the Ravenous Editor.
* ============================================================================================== */


/* =============================================================================================
 * Reflected (Macro):
 *	Responsible for setting the Setter and Getter funcs that enable reflection to work with the
 *	member declared through it.
 *	Defines a discarded static member that will run special code in its constructor at static
 *	initialization time to fill metadata about the field.
 * ============================================================================================== */
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

/* =============================================================================================
 * Field (Macro):
 *	Responsible for setting the Setter and Getter funcs that enable reflection to work with the
 *	member declared through it.
 *	Defines a discarded static member that will run special code in its constructor at static
 *	initialization time to fill metadata about the field.
 * ============================================================================================== */
#define Field(Type, Name, ...) ;  \
    inline static string Reflection_Getter_ ##Type ##_ ##Name(Self& instance) \
    { \
		static constexpr uint ReflectionGetterSerializationBufferSize = 100; \
		string SerializedField; \
		SerializedField.reserve(ReflectionGetterSerializationBufferSize); \
		int BytesWritten = sprintf_s(&SerializedField[0], ReflectionGetterSerializationBufferSize, "'%s' : %s = %s", #Name, #Type, Reflection::ToString(instance.Name).c_str()); \
		if (BytesWritten == -1) { \
			fatal_error("ReflectionGetter failed"); \
		} \
		return SerializedField; \
    } \
    \
    inline static void Reflection_Setter_ ##Type ##_ ##Name(Self* instance, string& Value) \
    { \
        instance->Name = Reflection::FromString<Type>(Value); \
    } \
    \
    struct Reflection_HelperType_ ##Type ##_ ##Name \
    { \
        Reflection_HelperType_ ##Type ##_ ##Name() \
        { \
            Reflection_GetterFuncPtrs.push_back(&Self::Reflection_Getter_ ##Type ##_ ##Name); \
            Reflection_SetterFuncPtrs[#Name] = &Self::Reflection_Setter_ ##Type ##_ ##Name; \
        } \
    } inline static reflection_discard_ ##Type ##_ ##Name{}; \
    __VA_ARGS__ Type Name


/* =============================================================================================
 * STORE_TYPE_IN_HELPER (Macro):
 *	Stores the name of the type in a helper struct that specialized a template using COUNTER so
 *	that every reflected type has access to its own name through a 'Self' type alias without
 *	requiring users to type the name of the type they are defining twice.
 *	This works well and safely because COUNTER is used A) after fwd declaring the new type,
 *	inside the EntityType() macro, B) right inside the type declaration in the Reflected() macro.
 *	That way, we can expect COUNTER - 1 to give back the original int used in the specialization
 *	from within the type definition (thus Self is always available to use).
 *	NOTE however, that reflection is enabled only in WITH_EDITOR builds, which means that gameplay
 *	code shouldn't rely on Self:: for anything.
 * ============================================================================================== */
#define STORE_TYPE_IN_HELPER(TypeName)	\
template<> \
struct Reflection::TypeNameWrapper<__COUNTER__> \
{ \
using T = TypeName; \
static inline string type_name = #TypeName; \
}

/* ===================================
 * Reflection namespace
 * ==================================== */
namespace Reflection
{
	template<unsigned long long int N> struct TypeNameWrapper { };

/* ====================================
 * Reflection::Tracer
 *	Dummy parent that is used in template type-trait checks to decide if Type is reflected
 * ==================================== */
	struct Tracer{};

/* ====================================
 * Dump Iterative
 * ==================================== */
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

/* ====================================
 * Load Iterative
 * ==================================== */
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


/* ============================================================================
 * ReflectionGetterSetter (Type):
 *	Enables introspecting into the class parents by assigning the Dump/Load
 *  func ptrs to a specialized version that includes the reflected type parents.
 * ============================================================================= */
	template<typename T, typename... Parents>
	struct ReflectionGetterSetter
	{
		ReflectionGetterSetter()
		{
			static char Helper = []
			{
				T::Reflection_DumpFunc = &DumpIterative<T, T, Parents...>;
				T::Reflection_LoadFunc = &LoadIterative<T, T, Parents...>;
				return 1;
			}();

			static_assert(&Helper);
		}
	};

/* ====================================
 * Dump
 * ==================================== */
	constexpr static uint SerializationSize = 1600;

	template<typename T>
	string Dump(T& Instance, bool bIncludeHeader = true)
	{
		string Serialized;
		Serialized.reserve(SerializationSize);

		string Header = "";
		if (bIncludeHeader)
		{
			Header.reserve(100);
			uint BytesWritten = sprintf_s(&Header[0], 100, "{ \"%s\" : %s = {", Instance.Name.c_str(), T::Reflection_TypeName.c_str());
			if (BytesWritten == -1) { 
				fatal_error("Writing Header on Dump<T> failed."); 
			}
		}
		else
		{
			// hack to make deserialization of nested objects work with current parsing code
			Serialized = Header.append("{,");
		}

		T::Reflection_DumpFunc(Instance, Serialized);

		Serialized.append(" } }\n");
		return Serialized;
	}

/* ====================================
 * Parse
 * ==================================== */
	void ParseObject(string& Data, map<string, string>& FieldValueMap, bool IncludeHeader);

	template<typename T>
	T Load(string& Data, bool IncludeHeader = true)
	{
		map<string, string> Fields;
		ParseObject(Data, Fields, IncludeHeader);

		T Instance;
		for (auto& [field, value] : Fields)
		{
			T::Reflection_LoadFunc(Instance, field, value);
		}

		return Instance;
	}

/* ====================================
 * ToString specializations
 * ==================================== */
	
	//string ToString(const string& Field);
	string ToString(string& Field);
	
	//string ToString(const char& Field);
	string ToString(char& Field);
	
	//string ToString(const bool& Field);
	string ToString(bool& Field);
	
	//string ToString(const vec3& Field);
	string ToString(vec3& Field);
	
	// default template for serialization of fields (check note on decltype usage below)
	template<typename TField>
	auto ToString(TField& Field) -> decltype(std::to_string(Field), string())
	{
		return std::to_string(Field);
	};

	// specialization for UserTypes (ReflectionTypes) (check note on decltype usage below)
	template<typename TField, std::enable_if_t<std::is_base_of_v<Reflection::Tracer, TField>, bool> = false>
	auto ToString(TField& Field) -> decltype(Dump<TField>(Field), string())
	{
		return Dump<TField>(Field, false);
	};


	/* ===================================================================================================
	 * Note on usage of decltype in templated function declarations:
	 * ===================================================================================================
	 *	decltype lets you evaluate the return type of an expression, and you can use it in conjunction
	 *  with the auto keyword to evaluate an expression that involves the template parameter.
	 *  What makes it useful, though, is that if the expression evaluation *fails* that templated
	 *  function is rulled out from the function call name resolution list. In that way, using the comma
	 *  operator (which evaluates a list of expressions and return the last one) we can use it to
	 *  check if a type has a member function or field, (or even another template specialization!) that
	 *  qualifies that type to be used with our templated function.
	 */
	
/* ====================================
 * FromString specializations
 * ==================================== */
	template<typename TField>
	TField FromString(string& Value)
	{
		//static_assert(false, "Not Implemented for Type");
		return TField();
	}

	template<typename TField, std::enable_if_t<std::is_base_of_v<Reflection::Tracer, TField>, bool> = false>
	TField FromString(string& Value)
	{
		return Load<TField>(Value, false);
	}
	
} // END Reflection namespace



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