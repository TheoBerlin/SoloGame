#pragma once

#include <Engine/ECS/Entity.hpp>
#include <Engine/Utils/StringUtils.hpp>

class ComponentType
{
public:
	constexpr ComponentType(const char* pTypeName)
		:
			m_pTypeName(pTypeName)
		,	m_Hash(HashString(pTypeName))
	{}

	~ComponentType() = default;

	constexpr const char* Name() const { return m_pTypeName; }
	constexpr size_t Hash() const { return m_Hash; }

private:
	const char* m_pTypeName;
	size_t m_Hash;
};

/*	ComponentTypeHash allows one to use an already hashed uint32_t as a key in an unordered associative container.
	This is used to for a <uint32, ComponentType> mapping, which is used when deserializing entities. When sending entity data over the internet,
	only the hash of each component type is sent, not its name string. Hence the mapping is needed. */
class ComponentTypeHash
{
public:
	ComponentTypeHash(uint32_t hash) :
		Hash(hash)
	{}

	bool operator==(const ComponentTypeHash& other) const
	{
		return Hash == other.Hash;
	}

	const uint32_t Hash;
};

namespace std
{
	template<>
	struct hash<const ComponentType*>
	{
		size_t operator()(const ComponentType* pComponentType) const
		{
			return pComponentType->Hash();
		}
	};

	template<>
	struct hash<ComponentTypeHash>
	{
		size_t operator()(const ComponentTypeHash& typeHash) const
		{
			return typeHash.Hash;
		}
	};
}

#define DECL_COMPONENT(Component) \
	private: \
		inline static constexpr const ComponentType s_Type = ComponentType(#Component); \
	public: \
		FORCEINLINE static constexpr const ComponentType* Type() \
		{ \
			return &s_Type; \
		} \
		static constexpr bool HasDirtyFlag() \
		{ \
			return false; \
		} \

#define DECL_COMPONENT_WITH_DIRTY_FLAG(Component) \
	protected: \
		inline static constexpr const ComponentType s_Type = ComponentType(#Component); \
	public: \
		FORCEINLINE static constexpr const ComponentType* Type() \
		{ \
			return &s_Type; \
		} \
		static constexpr bool HasDirtyFlag() \
		{ \
			return true; \
		} \
		bool Dirty = true \

	enum ComponentPermissions
	{
		NDA = 0,	// No Data Access
		R   = 1,	// Read
		RW  = 2		// Read & Write
	};

template <typename Comp>
class GroupedComponent
{
public:
	ComponentPermissions Permissions = NDA; // The default permissions for any component type in a component group
};

struct ComponentAccess
{
	template <typename Comp>
	ComponentAccess(GroupedComponent<Comp> groupedComponent)
		:
		Permissions(groupedComponent.Permissions),
		pTID(Comp::Type())
	{}

	ComponentAccess(ComponentPermissions permissions, const ComponentType* pType)
		:
		Permissions(permissions),
		pTID(pType)
	{}

	ComponentPermissions Permissions;
	const ComponentType* pTID;
};

class IComponentGroup
{
public:
	virtual std::vector<ComponentAccess> ToArray() const = 0;
};

template <typename Comp>
struct ComponentOwnership
{
	// Called just after creating a component
	std::function<void(Comp& component, Entity entity)> Constructor;
	// Called just before deleting a component
	std::function<void(Comp& component, Entity entity)> Destructor;
	/**
	 * Serialize the component into the buffer, excluding the type hash.
	 * \return The required size of the serialization in bytes.
	 * Does not write to the buffer if said size is greater than the provided bufferSize.
	*/
	std::function<uint32_t(const Comp& component, uint8_t* pBuffer, uint32_t bufferSize)> Serialize;
	/**
	 * Deserialize the buffer into the referenced component. Does not add or register the component.
	 * The passed component could be referencing an already existing component. One should be aware of this to
	 * to avoid allocating memory for members of the component without deleting previous allocations.
	 * \return Success or failure.
	*/
	std::function<bool(Comp& component, uint32_t serializationSize, const uint8_t* pBuffer)> Deserialize;
};
