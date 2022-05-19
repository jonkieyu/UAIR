#ifndef UTILITY_H
#define UTILITY_H

#include "hash_map.h"
#include "hash_set.h"
#include <cstdlib>
#include <iostream>
#include <vector>

namespace uair {

    void print(const std::vector<int> &v);

    void print(const std::vector<std::vector<int>> &v);

    void print(const hash_set<int> &s);

    void print(const hash_set<unsigned> &s);

    void print(const hash_map<int, int> &m);

    void print(const hash_map<int, std::vector<int>> &m);

    // elements in v1, v2 are in order
    // check whether v2 is contained in v1
    bool imply(std::vector<int> &v1, std::vector<int> &v2);

    std::vector<int> vec_intersect(const std::vector<int> &v1,
                                   const std::vector<int> &v2);
    std::vector<int> vec_unite(const std::vector<int> &v1,
                               const std::vector<int> &v2);

    inline std::vector<int> cube_intersect(const std::vector<int> &v1,
                                           const std::vector<int> &v2) {
        return vec_intersect(v1, v2);
    }

    inline std::vector<int> cube_unite(const std::vector<int> &v1,
                                       const std::vector<int> &v2) {
        return vec_unite(v1, v2);
    }

    bool comp(int i, int j);

} // namespace uair

#endif
