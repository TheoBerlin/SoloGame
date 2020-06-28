#pragma once

#include <set>

// Eliminate duplicates from a vector. Note that no memory is deallocated from the vector; its capacity remains the same.
template <typename T>
void eliminateDuplicates(std::vector<T>& vec)
{
    std::set<T> set;
    for (const T& element : vec) {
        set.insert(element);
    }

    vec.assign(set.begin(), set.end());
}
