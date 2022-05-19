#include "utility.h"
#include <algorithm>
#include <set>
#include <vector>

using namespace std;

namespace uair {

    void print(const std::vector<int> &v) {
        if (v.size() > 0) {
            for (int i = 0; i < v.size() - 1; ++i)
                std::cout << v[i] << " ";
            std::cout << v[v.size() - 1];
            std::cout << std::endl;
        }
    }

    void print(const std::vector<std::vector<int>> &v) {
        for (const auto &elem : v) {
            for (const auto &elem2 : elem) {
                std::cout << elem2 << " ";
            }
            std::cout << std::endl;
        }
    }

    void print(const hash_set<int> &s) {
        for (hash_set<int>::const_iterator it = s.begin(); it != s.end(); it++)
            std::cout << *it << " ";
        std::cout << std::endl;
    }

    void print(const hash_set<unsigned> &s) {
        for (hash_set<unsigned>::const_iterator it = s.begin(); it != s.end();
             it++)
            std::cout << *it << " ";
        std::cout << std::endl;
    }

    void print(const hash_map<int, int> &m) {
        for (hash_map<int, int>::const_iterator it = m.begin(); it != m.end();
             it++)
            std::cout << it->first << " -> " << it->second << std::endl;
    }

    void print(const hash_map<int, std::vector<int>> &m) {
        for (hash_map<int, std::vector<int>>::const_iterator it = m.begin();
             it != m.end(); it++) {
            std::cout << it->first << " -> {";
            vector<int> v = it->second;
            for (int i = 0; i < v.size(); i++)
                cout << v[i] << " ";
            cout << "}" << endl;
        }
    }

    bool comp(int i, int j) { return abs(i) < abs(j); }

    // elements in v1, v2 are in order
    // check whether v2 is contained in v1
    bool imply(std::vector<int> &v1, std::vector<int> &v2) {

        if (v1.size() < v2.size())
            return false;

        auto first1 = v1.begin(), first2 = v2.begin(), last1 = v1.end(),
             last2 = v2.end();
        while (first2 != last2) {
            if ((first1 == last1) || comp(*first2, *first1))
                return false;
            if ((*first1) == (*first2))
                ++first2;
            ++first1;
        }
        return true;
    }

    std::vector<int> vec_intersect(const std::vector<int> &vec1,
                                   const std::vector<int> &vec2) {
        std::vector<int> res;
        auto first1 = vec1.begin();
        auto first2 = vec2.begin();
        auto last1 = vec1.end();
        auto last2 = vec2.end();
        while (first2 != last2) {
            if (first1 == last1)
                break;
            // abs(*first1)<abs(*first2)
            // sort by abs(value), increasing
            if (comp(*first1, *first2))
                first1++;
            else if ((*first1) == (*first2)) {
                res.push_back(*first1);
                first1++;
                first2++;
            } else
                first2++;
        }
        return res;
    }

    // union of vectors
    std::vector<int> vec_unite(const std::vector<int> &vec1,
                               const std::vector<int> &vec2) {
        std::vector<int> res;
        std::set<int> tmp;
        for (const auto &elem : vec1)
            tmp.insert(elem);
        for (const auto &elem : vec2)
            tmp.insert(elem);
        for (const auto &elem : tmp)
            res.push_back(elem);
        sort(res.begin(), res.end(),
             [](int x, int y) { return abs(x) < abs(y); });
        return res;
    }
} // namespace uair