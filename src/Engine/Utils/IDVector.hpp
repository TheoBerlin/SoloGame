#pragma once

#include <Engine/Utils/Assert.hpp>
#include <Engine/Utils/IDContainer.hpp>

/*
	Extends a vector to be able to:
	* Use IDs to index elements
	* Pop elements anywhere in the array without breaking ID-index relation

	IDD: ID Data
*/

template <typename T>
class IDDVector : public IDContainer
{
public:
	IDDVector() = default;
	~IDDVector() = default;

	// Index vector directly
	T operator[](uint32_t index) const
	{
		return m_Data[index];
	}

	T& operator[](uint32_t index)
	{
		return m_Data[index];
	}

	// Index vector using ID, assumes ID is linked to an element
	const T& IndexID(uint32_t ID) const
	{
		auto indexItr = m_IDToIndex.find(ID);
		ASSERT_MSG(indexItr != m_IDToIndex.end(), "Attempted to index using an unregistered ID: %d", ID);

		return m_Data[indexItr->second];
	}

	T& IndexID(uint32_t ID)
	{
		auto indexItr = m_IDToIndex.find(ID);
		ASSERT_MSG(indexItr != m_IDToIndex.end(), "Attempted to index using an unregistered ID: %d", ID);

		return m_Data[indexItr->second];
	}

	void push_back(const T& newElement, uint32_t ID)
	{
		m_Data.push_back(newElement);
		m_IDs.push_back(ID);
		m_IDToIndex[ID] = (uint32_t)m_Data.size() - 1;
	}

	void Pop(uint32_t ID) override final
	{
		auto popIndexItr = m_IDToIndex.find(ID);
		ASSERT_MSG(popIndexItr != m_IDToIndex.end(), "Attempted to pop a non-existing element, ID: %d", ID);

		m_Data[popIndexItr->second] = m_Data.back();
		m_IDs[popIndexItr->second] = m_IDs.back();

		m_IDToIndex[m_IDs.back()] = popIndexItr->second;

		m_Data.pop_back();
		m_IDs.pop_back();

		m_IDToIndex.erase(popIndexItr);
	}

	void Clear()
	{
		m_Data.clear();
		m_IDs.clear();
		m_IDToIndex.clear();
	}

	bool HasElement(uint32_t ID) const override final
	{
		return m_IDToIndex.contains(ID);
	}

	uint32_t Size() const override final
	{
		return (uint32_t)m_Data.size();
	}

	bool Empty() const
	{
		return m_Data.empty();
	}

	std::vector<T>& GetVec()
	{
		return m_Data;
	}

	const std::vector<T>& GetVec() const
	{
		return m_Data;
	}

	const std::vector<uint32_t>& GetIDs() const override final
	{
		return m_IDs;
	}

	T& Back()
	{
		return m_Data.back();
	}

	typename std::vector<T>::iterator begin() noexcept
	{
		return m_Data.begin();
	}

	typename std::vector<T>::iterator end() noexcept
	{
		return m_Data.end();
	}

	typename std::vector<T>::const_iterator begin() const noexcept
	{
		return m_Data.begin();
	}

	typename std::vector<T>::const_iterator end() const noexcept
	{
		return m_Data.end();
	}

private:
	std::vector<T> m_Data;
	// The ID for each data element. Stored separately from the main data for cache-friendliness.
	std::vector<uint32_t> m_IDs;

	// Maps IDs to indices to the data array
	std::unordered_map<uint32_t, uint32_t> m_IDToIndex;
};

class IDVector : public IDContainer
{
public:
	IDVector() = default;
	~IDVector() = default;

	uint32_t operator[](uint32_t index) const
	{
		return m_IDs[index];
	}

	void push_back(uint32_t ID)
	{
		m_IDs.push_back(ID);
		m_IDToIndex.insert({ ID, (uint32_t)m_IDs.size() - 1 });
	}

	void Pop(uint32_t ID) override final
	{
		auto popIndexItr = m_IDToIndex.find(ID);
		ASSERT_MSG(popIndexItr != m_IDToIndex.end(), "Attempted to pop a non-existing element, ID: %u", ID);

		m_IDs[popIndexItr->second] = m_IDs.back();
		m_IDToIndex[m_IDs.back()] = popIndexItr->second;
		m_IDs.pop_back();

		m_IDToIndex.erase(popIndexItr);
	}

	void Clear()
	{
		m_IDs.clear();
		m_IDToIndex.clear();
	}

	bool HasElement(uint32_t ID) const override final
	{
		return m_IDToIndex.contains(ID);
	}

	uint32_t Size() const override final
	{
		return (uint32_t)m_IDs.size();
	}

	bool Empty() const
	{
		return m_IDs.empty();
	}

	const std::vector<uint32_t>& GetIDs() const override final
	{
		return m_IDs;
	}

	uint32_t Front()
	{
		return m_IDs.front();
	}

	uint32_t Back()
	{
		return m_IDs.back();
	}

	typename std::vector<uint32_t>::iterator begin() noexcept
	{
		return m_IDs.begin();
	}

	typename std::vector<uint32_t>::iterator end() noexcept
	{
		return m_IDs.end();
	}

	typename std::vector<uint32_t>::const_iterator begin() const noexcept
	{
		return m_IDs.begin();
	}

	typename std::vector<uint32_t>::const_iterator end() const noexcept
	{
		return m_IDs.end();
	}

private:
	std::vector<uint32_t> m_IDs;
	// Maps IDs to indices to the ID array
	std::unordered_map<uint32_t, uint32_t> m_IDToIndex;
};
