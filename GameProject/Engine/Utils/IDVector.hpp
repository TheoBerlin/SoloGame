#pragma once

#include <Engine/Utils/IDContainer.hpp>
#include <queue>
#include <vector>

/*
    Extends a vector to be able to:
    * Use IDs to index elements
    * Pop elements in the middle of the array without breaking ID-index relation
    
    Stores elements, index for each ID and ID for each element
*/

template <typename T>
class IDVector : public IDContainer
{
public:
    IDVector() {}
    ~IDVector() {}

    // Index vector directly
    inline T operator[](size_t index) const
    {
        return vec[index];
    }

    inline T& operator[](size_t index)
    {
        return vec[index];
    }

    // Index vector using ID, assumes ID is linked to an element
    inline T indexID(size_t ID) const
    {
        return vec[indices[ID]];
    }

    inline T& indexID(size_t ID)
    {
        return vec[indices[ID]];
    }

    // Returns ID of added element
    void push_back(const T& newElement, size_t ID)
    {
        vec.push_back(newElement);
        ids.push_back(ID);

        // Link element ID to index, resize ID vector in case ID is larger than the vector's size
        if (indices.size() < ID+1) {
            indices.resize(ID+1);
        }

        indices[ID] = vec.size()-1;
    }

    void pop(size_t ID)
    {
        size_t popIndex = indices[ID];

        if (popIndex < vec.size()-1) {
            // Replace to be popped element with the rear element
            vec[popIndex] = vec.back();
            ids[popIndex] = ids.back();

            indices[ids.back()] = popIndex;
        }

        vec.pop_back();
        ids.pop_back();

        if (vec.empty()) {
            indices.clear();
        }

        // The rear elements in the indices vector might point at deleted elements, clean them up
        while (indices.size() > vec.size() && indices.back() >= vec.size()) {
            indices.pop_back();
        }
    }

    void clear()
    {
        vec.clear();
        ids.clear();
        indices.clear();
    }

    inline bool hasElement(size_t ID) const
    {
        // Check if the ID pointed at by indices[ID] is the same as the parameter ID
        return ID < indices.size() && ID == ids[indices[ID]];
    }

    inline size_t size() const
    {
        return vec.size();
    }

    std::vector<T>& getVec()
    {
        return this->vec;
    }

    const std::vector<size_t>& getIDs() const
    {
        return this->ids;
    }

    inline T& back()
    {
        return this->vec.back();
    }

private:
    std::vector<T> vec;

    /*
        Stores indices to the main vector, use entity IDs to index it, eg:
        T myElement = vec[indices[ID]];
        or
        size_t entityIndex = indices[entityID];
    */
    std::vector<size_t> indices;

    // Stores index for each ID, eg. ids[5] == vec[5].id had vec's elements contained IDs
    std::vector<size_t> ids;
};
