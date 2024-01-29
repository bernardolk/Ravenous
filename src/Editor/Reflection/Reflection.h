#pragma once

#include "Serialization.h"
#include "Engine/Core/Core.h"
#include "Engine/Serialization/Parsing/Parser.h"
#include "Engine/World/World.h"

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

#define Reflected_Common(Type) \
	using Self = Type; \
	using DumpFuncType = void(*)(Self& instance, string& OutSerialized); \
	using LoadFuncType = void(*)(Self& instance, const string& field, const string& value); \
	using GetterFuncPtrType = string(*)(Self&); \
	using SetterFuncPtrType = void (*)(Self*, const string&); \
	struct SetterData { string Field; SetterFuncPtrType SetterFunc; }; \
	inline static string Reflection_TypeName = #Type; \
	inline static DumpFuncType Reflection_DumpFunc = nullptr; \
	inline static LoadFuncType Reflection_LoadFunc = nullptr; \
	inline static vector<GetterFuncPtrType> Reflection_GetterFuncPtrs; \
	inline static vector<SetterData> Reflection_SetterFuncPtrs;

#define Reflected(Type) \
	Reflected_Common(Type) \
	struct Reflection_TypeInitialization \
	{ \
		Reflection_TypeInitialization() \
		{ \
			auto InitFunc = [](const string& SerializedEntity) -> EEntity* { EHandle<Self> NewEntityHandle = SpawnEntity<Self>(); Reflection::Load(SerializedEntity, *(*NewEntityHandle)); return static_cast<EEntity*>(*NewEntityHandle); }; \
			auto CastAndDumpFunc = [](EEntity& Instance) -> string { return Reflection::Dump<Self>(*reinterpret_cast<Self*>(&Instance)); }; \
			auto NewFunc = []() -> EEntity* { EHandle<Self> NewEntityHandle = SpawnEntity<Self>(); return static_cast<EEntity*>(*NewEntityHandle); }; \
			Reflection::TypeMetadata Meta; \
			Meta.TypeName = Reflection_TypeName; \
			Meta.TypeID = Self::GetTypeID(); \
			Meta.TypeInitFunction = InitFunc; \
			Meta.Size = sizeof(Self); \
			Meta.CastAndDumpFunction = CastAndDumpFunc; \
			Meta.NewFunction = NewFunc; \
			Reflection::TypeMetadataManager::Get()->TypeMetadataCollection.push_back(Meta); \
		} \
	} inline static __Reflection_TypeInitializer{};

#define Reflected_BaseEEntity(Type) \
	Reflected_Common(Type)


/* =============================================================================================
 * Field (Macro):
 *	Responsible for setting the Setter and Getter funcs that enable reflection to work with the
 *	member declared through it.
 *	Defines a discarded static member that will run special code in its constructor at static
 *	initialization time to fill metadata about the field.
 * ============================================================================================== */
//@TODO @speed string copy in getters
#define Field(Type, Name, ...) ;  \
    static string Reflection_Getter_ ##Name(Self& instance) \
    { \
		string SerializedField; \
		SerializedField.reserve(Reflection::ReflectionGetterSerializationBufferSize); \
		int BytesWritten = sprintf_s(&SerializedField[0], Reflection::ReflectionGetterSerializationBufferSize, "%s: %s = %s", #Name, #Type, Reflection::ToString(instance.Name).c_str()); \
		if (BytesWritten == -1) { \
			FatalError("ReflectionGetter failed"); \
		} \
		return SerializedField; \
    } \
    \
    static void Reflection_Setter_ ##Name(Self* instance, const string& Value) \
    { \
        instance->Name = Reflection::FromString<Type>(Value); \
    } \
    \
    struct Reflection_HelperType_ ##Name \
    { \
        Reflection_HelperType_ ##Name() \
        { \
            Reflection_GetterFuncPtrs.push_back(&Self::Reflection_Getter_ ##Name); \
			Reflection_SetterFuncPtrs.push_back({#Name, &Self::Reflection_Setter_ ##Name}); \
        } \
    } inline static Reflection__discard_ ##Name{}; \
    __VA_ARGS__ Type Name

//@TODO @speed string copy in getters
#define Handle(Type, Name, ...) ;  \
    static string Reflection_Getter_ ##Name(Self& instance) \
    { \
		string SerializedField; \
		SerializedField.reserve(Reflection::ReflectionGetterSerializationBufferSize); \
		int BytesWritten = sprintf_s(&SerializedField[0], Reflection::ReflectionGetterSerializationBufferSize, "%s: %s = %s", #Name, #Type, Reflection::ToString(instance.Name).c_str()); \
		if (BytesWritten == -1) { \
			FatalError("ReflectionGetter failed"); \
		} \
		return SerializedField; \
    } \
    \
    static void Reflection_Setter_ ##Name(Self* instance, const string& Value) \
    { \
        instance->Name = Reflection::FromStringHandle<Type>(Value); \
    } \
    \
    struct Reflection_HelperType_ ##Name \
    { \
        Reflection_HelperType_ ##Name() \
        { \
            Reflection_GetterFuncPtrs.push_back(&Self::Reflection_Getter_ ##Name); \
			Reflection_SetterFuncPtrs.push_back({#Name, &Self::Reflection_Setter_ ##Name}); \
        } \
    } inline static Reflection__discard_ ##Name{}; \
    __VA_ARGS__ Type Name

/* ===================================
 * Reflection namespace
 * ==================================== */
namespace Reflection
{
	template<unsigned long long int N> struct TypeNameWrapper { };

	constexpr uint ReflectionGetterSerializationBufferSize = 100;

	void ParseFieldsFromSerializedObject(const string& Data, map<string, string>& OutFieldValueMap);

	struct TypeMetadata
	{
		using TypeInitFPtr = EEntity*(*)(const string& SerializedEntity);
		using CastAndDumpFPtr = string(*)(EEntity& Instance);
		using NewFunc = EEntity*(*)(void);
		
		string TypeName;
		RTypeID TypeID = 0;
		int64 Size = 0;
		TypeInitFPtr TypeInitFunction = nullptr;
		CastAndDumpFPtr CastAndDumpFunction = nullptr;
		NewFunc NewFunction = nullptr;
	};
	
	struct TypeMetadataManager
	{
		vector<TypeMetadata> TypeMetadataCollection;

		static TypeMetadataManager* Get()
		{
			static TypeMetadataManager Instance{};
			return &Instance;
		}

		TypeMetadata* FindTypeMetadata(RTypeID TypeID)
		{
			for (auto& Data : TypeMetadataCollection)
			{
				if (Data.TypeID == TypeID) {
					return &Data;
				}	
			}

			return nullptr;
		}

		TypeMetadata* FindTypeMetadataByName(const string& TypeName)
		{
			for (auto& Data : TypeMetadataCollection)
			{
				if (Data.TypeName == TypeName) {
					return &Data;
				}	
			}

			return nullptr;
		}
	};
	
/* =====================================================================================================================
 * Reflection::Tracer
 *	Dummy type used as a parent in reflected types, used by template type-trait checks to decide if Type is reflected
 * ===================================================================================================================== */
	struct Tracer{};

/* ============================================================================================
 * Reflection::FromString
 *	Templated function to convert string values into actual valid instances of that type
 * ============================================================================================ */
	template<typename TField>
	TField FromString(const string& Value);

	// template<typename T>
	// EHandle<T> FromString(const string& Value);

	template<typename T>
	EHandle<T> FromStringHandle(const string& Value)
	{
		RUUID ID = FromString<RUUID>(Value);
		EHandle<T> Handle = MakeHandleFromID<T>(ID);
		if(!Handle.IsValid())
		{
			auto* RestoredEntity = Serialization::LoadEntityFromFile(ID);
			if (!RestoredEntity) {
				Break("Error: Couldn't load entity from file while deserializing handle for entity with ID: %s", Value.c_str())
				return {};
			}
			return MakeHandle<T>(RestoredEntity);
		}
		return Handle;
	}

/* ====================================
 * Dump Iterative
 * ==================================== */
	template<typename T, typename Class, typename... Parents>
	void DumpIterative(T& Instance, string& OutSerialized)
	{
		for (auto* FieldGetter : Class::Reflection_GetterFuncPtrs)
		{
			string Value = FieldGetter(Instance);
			OutSerialized.append(Value.c_str());
			OutSerialized.push_back('\n');
		}

		if constexpr (sizeof...(Parents) > 0)
		{
			DumpIterative<T, Parents...>(Instance, OutSerialized);
		}
	}

/* ====================================
 * Load Iterative
 * ==================================== */
	template<typename T, typename Class, typename... Parents>
	void LoadIterative(T& Instance, const string& Field, const string& Value)
	{
		for (auto& SetterData : Class::Reflection_SetterFuncPtrs)
		{		
			if (SetterData.Field == Field) {
				SetterData.SetterFunc(&Instance, Value);
				return;
			}
		}

		// If SetterData is not found, try in parents
		if constexpr (sizeof...(Parents) > 0) {
			LoadIterative<T, Parents...>(Instance, Field, Value);
		}
		else {
			FatalError("PROBLEM: We couldn't find the appropriate setter for the field '%s' with value '%s' anywhere.", Field.c_str(), Value.c_str());
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
	constexpr static uint SerializationBufferSize = 1600;

	template<typename T>
	string Dump(T& Instance)
	{
		string Serialized;
		Serialized.reserve(SerializationBufferSize);

		// Append Header (Name : Type)
		Serialized.append(Instance.Name);
		Serialized.append(" : ");
		Serialized.append(T::Reflection_TypeName);
		Serialized.push_back('\n');
		
		// Where actual work happens
		T::Reflection_DumpFunc(Instance, Serialized);

		return Serialized;
	}

/* ====================================
 * Load
 * ==================================== */
	EEntity* LoadFromString(const string& SeralizedEntity);
	
	template<typename T>
	void Load(const string& Data, T& OutInstance)
	{
		Parser p(Data, Data.size());

		// Set name and discard Type string
		p.ParseNewLine();
		p.ParseToken();
		if (p.HasToken()) {
			OutInstance.Name = GetParsed<string>(p);
		}
		p.ParseLine();

		// Parse other fields
		string FieldsData = p.P.String;		
		std::map<string, string> Fields;
		ParseFieldsFromSerializedObject(FieldsData, Fields);

		for (auto& [Field, Value] : Fields)
		{
			T::Reflection_LoadFunc(OutInstance, Field, Value);
		}
	}

	/* =======================================
	/* EEntity Specialization
	/* ======================================= */
	// EEntity is an abstract type, it does not contain budget or traits info and shouldn't
	// be directly spanwed. EStaticMesh is the correct basic entity type.
	template<>
	inline void Load(const string& Data, EEntity& OutInstance)
	{
		// Do nothing.	
	}

/* ==============================================
 * FromString specialization for reflected types
 * ============================================== */
	template<typename TField, std::enable_if_t<std::is_base_of_v<Reflection::Tracer, TField>, bool> = false>
	TField FromString(const string& StringValue)
	{
		return Load<TField>(StringValue, false);
	}

/* ====================================
 * ToString specializations
 * ==================================== */
	
	string ToString(string& Field);
	string ToString(char& Field);
	string ToString(bool& Field);
	string ToString(vec3& Field);
	string ToString(RUUID& Field);
	string ToString(RShader* Field);
	string ToString(RMesh* Field);
	string ToString(RCollisionMesh* Field);
	string ToString(RTexture Field);

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

	template<typename T>
	string ToString(EHandle<T>& Field)
	{	
		return ToString(Field.EntityID);	
	}

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
	
} // END Reflection namespace


/* ===================================================================================================
 * GetLine
 * ===================================================================================================*/
inline string GetLine(string& Text, char DelimiterChar)
{
	uint Pos = Text.find(DelimiterChar);
	if (Pos == std::string::npos)
		return "";
	string Line = Text.substr(0, Pos);
	Text.erase(0, Pos + 1);
	return Line;
}