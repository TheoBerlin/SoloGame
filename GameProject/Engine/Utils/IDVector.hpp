#pragma once

#include <Engine/Utils/IDContainer.hpp>
#include <queue>
#include <vector>

/*
    Extends a vector to be able to:
    * Use IDs to index elements
    * Pop elements in the middle of the array without breaking ID-index relation

    Stores elements, index for each ID and ID for each element

    IDD: ID Data
*/

template <typename T>
class IDDVector : public IDContainer
{
public:
    IDDVector() {}
    ~IDDVector() {}

    // Index vector directly
    inline T operator[](size_t index) const
    {
        return m_Data[index];
    }

    inline T& operator[](size_t index)
    {
        return m_Data[index];
    }

    // Index vector using ID, assumes ID is linked to an element
    inline T indexID(size_t ID) const
    {
        return m_Data[m_Indices[ID]];
    }

    inline T& indexID(size_t ID)
    {
        return m_Data[m_Indices[ID]];
    }

    void push_back(const T& newElement, size_t ID)
    {
        m_Data.push_back(newElement);
        m_IDs.push_back(ID);

        // Link element ID to index, resize ID vector in case ID is larger than the vector's size
        if (m_Indices.size() < ID + 1) {
            m_Indices.resize(ID + 1);
        }

        m_Indices[ID] = m_Data.size()-1;
    }

    void pop(size_t ID)
    {
        size_t popIndex = m_Indices[ID];

        if (popIndex < m_Data.size()-1) {
            // Replace to be popped element with the rear element
            m_Data[popIndex] = m_Data.back();
            m_IDs[popIndex] = m_IDs.back();

            m_Indices[m_IDs.back()] = popIndex;
        }

        m_Data.pop_back();
        m_IDs.pop_back();

        if (m_Data.empty()) {
            m_Indices.clear();
        }

        // The rear elements in the indices vector might point at deleted elements, clean them up
        while (m_Indices.size() > m_Data.size() && m_Indices.back() >= m_Data.size()) {
            m_Indices.pop_back();
        }
    }

    void clear()
    {
        m_Data.clear();
        m_IDs.clear();
        m_Indices.clear();
    }

    inline bool hasElement(size_t ID) const
    {
        // Check if the ID pointed at by indices[ID] is the same as the parameter ID
        return ID < m_Indices.size() && ID == m_IDs[m_Indices[ID]];
    }

    inline size_t size() const
    {
        return m_Data.size();
    }

    std::vector<T>& getVec()
    {
        return this->m_Data;
    }

    const std::vector<T>& getVec() const
    {
        return this->m_Data;
    }

    const std::vector<size_t>& getIDs() const
    {
        return this->m_IDs;
    }

    inline T& back()
    {
        return this->m_Data.back();
    }

private:
    std::vector<T> m_Data;

    /*
        Stores indices to the main vector, use entity IDs to index it, eg:
        T myElement = vec[indices[ID]];
        or
        size_t entityIndex = indices[entityID];
    */
    std::vector<size_t> m_Indices;

    // Stores index for each ID, eg. ids[5] == vec[5].id (had vec's elements contained IDs)
    std::vector<size_t> m_IDs;
};

class IDVector : public IDContainer
{
public:
    IDVector() {}
    ~IDVector() {}

    inline size_t operator[](size_t index) const
    {
        return m_IDs[index];
    }

    void push_back(size_t ID)
    {
        m_IDs.push_back(ID);

        // Link element ID to index, resize ID vector in case ID is larger than the vector's size
        if (m_Indices.size() < ID + 1) {
            m_Indices.resize(ID + 1);
        }

        m_Indices[ID] = m_Indices.size() - 1;
    }

    void pop(size_t ID)
    {
        size_t popIndex = m_Indices[ID];

        if (popIndex < m_IDs.size() - 1) {
            // Replace to be popped element with the rear element
            m_IDs[popIndex] = m_IDs.back();

            m_Indices[m_IDs.back()] = popIndex;
        }

        m_IDs.pop_back();

        if (m_IDs.empty()) {
            m_Indices.clear();
        }

        // The rear elements in the indices vector might point at deleted elements, clean them up
        while (m_Indices.size() > m_IDs.size() && m_Indices.back() >= m_IDs.size()) {
            m_Indices.pop_back();
        }
    }

    void clear()
    {
        m_IDs.clear();
        m_Indices.clear();
    }

    inline bool hasElement(size_t ID) const
    {
        // Check if the ID pointed at by indices[ID] is the same as the parameter ID
        return ID < m_Indices.size() && ID == m_IDs[m_Indices[ID]];
    }

    inline size_t size() const
    {
        return m_IDs.size();
    }

    const std::vector<size_t>& getIDs() const
    {
        return m_IDs;
    }

    inline size_t back()
    {
        return m_IDs.back();
    }

private:
    /*
        Stores indices to the main vector, use entity IDs to index it, eg:
        T myElement = vec[indices[ID]];
        or
        size_t entityIndex = indices[entityID];
    */
    std::vector<size_t> m_Indices;

    // Stores index for each ID, eg. ids[5] == vec[5].id (had vec's elements contained IDs)
    std::vector<size_t> m_IDs;
};

