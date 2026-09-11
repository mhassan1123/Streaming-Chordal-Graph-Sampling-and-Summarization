#ifndef DISJOINT_SET_UNION_HPP
#define DISJOINT_SET_UNION_HPP

#include <vector>
#include <unordered_map>
#include <cstddef>

class DisjointSetUnion {
private:
    int n;
    std::vector<int> par, rank;
    std::size_t componentCount;

public:
    DisjointSetUnion();
    explicit DisjointSetUnion(int n);

    void ensureSize(int node);
    int find(int a);
    void unionByRank(int a, int b);
    bool isConnected(int a, int b);
    std::size_t getComponentCount() const;
    std::unordered_map<int, std::vector<int>> getCommunities(const std::vector<int>& activeNodes);
};

#endif // DISJOINT_SET_UNION_HPP