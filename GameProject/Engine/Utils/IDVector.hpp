#pragma once

#include <queue>
#include <vector>

/*
    Extends a vector to be able to:
    * Use IDs to index elements
    * Pop elements in the middle of the array without breaking ID-index relation
    
    Stores elements, index for each ID and ID for each element
*/

template <typename T>
class IDVector
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
    void push_back(T& newElement, size_t ID)
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

        // The indices vector might point at deleted elements, clean up the rear elements
        while (!indices.empty() && (ids.empty() || ids[indices.back()] != indices.size() - 1)) {
            indices.pop_back();
        }
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

    const std::vector<size_t>& getIDs() const
    {
        return this->ids;
    }

private:
    std::vector<T> vec;

    /*
        Stores indexed using an ID to retrieve and index to vec
        Example: T myElement = vec[indices[ID]];
    */
    std::vector<size_t> indices;

    // Stores index for each ID, eg. ids[5] == vec[5].id had vec's elements contained IDs
    std::vector<size_t> ids;
};
