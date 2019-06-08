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
    IDVector();
    ~IDVector();

    // Index vector directly
    inline T operator[](size_t index);

    // Index vector using ID, assumes ID is linked to an element
    inline T indexID(size_t ID);

    // Returns ID of added element
    inline void push_back(T& newElement, size_t ID);

    void pop(size_t ID);

    inline bool hasElement(size_t ID) const;

private:
    std::vector<T> vec;

    /*
        Stores indexed using an ID to retrieve and index to vec
        Example: T myElement = vec[indices[ID]];
    */
    std::vector<size_t> indices;

    // Stores index for each ID
    std::vector<size_t> ids;

    // IDs available to recycle
    std::queue<size_t> freeIDS;
};
