#include "IDVector.hpp"

template <typename T>
IDVector<T>::IDVector()
{}

template <typename T>
IDVector<T>::~IDVector()
{}

template <typename T>
T IDVector<T>::operator[](size_t index)
{
    return vec[index];
}

template <typename T>
T IDVector<T>::indexID(size_t ID)
{
    return vec[indices[ID]];
}

template <typename T>
void IDVector<T>::push_back(T& newElement, size_t ID)
{
    vec.push_back(newElement);
    ids.push_back(ID);

    // Link element ID to index, resize ID vectors in case ID is larger than their sizes
    indices.reserve(ID);
    usedIds.reserve(ID);

    indices[ID] = vec.size();
    usedIds[ID] = true;
}

template <typename T>
void IDVector<T>::pop(size_t ID)
{
    size_t popIndex = indices[ID];

    if (popIndex < vec.size()) {
        // Replace to be popped element with the rear element
        vec[popIndex] = vec.back();
        ids[popIndex] = ids.back();

        indices[ids.back()] = popIndex;
    }

    vec.pop_back();
    ids.pop_back();

    // The indices vector might point at deleted elements, clean up the rear elements
    while (indices.back() != ids.[indices.back()]) {
        indices.pop_back();
    }
}

template <typename T>
bool IDVector<T>::hasElement(size_t ID) const
{
    // Check if the ID pointed at by indices[ID] is the same as the parameter ID
    return ID == ids[indices[ID]];
}
