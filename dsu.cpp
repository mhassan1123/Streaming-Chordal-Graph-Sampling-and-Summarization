#include "dsu.hpp"

DisjointSetUnion::DisjointSetUnion() : n(0), componentCount(0) {}

DisjointSetUnion::DisjointSetUnion(int n) : n(n), componentCount(n) {
    for (int i = 0; i < n; i++) {
        par.push_back(i);
        rank.push_back(0);
    }
}

void DisjointSetUnion::ensureSize(int node) {
    if (node >= n) {
        int oldN = n;
        n = node + 1;
        par.resize(n);
        rank.resize(n);
        for (int i = oldN; i < n; i++) {
            par[i] = i;
            rank[i] = 0;
            componentCount++;
        }
    }
}

int DisjointSetUnion::find(int a) {
    ensureSize(a);
    if (par[a] == a) return a;
    return par[a] = find(par[a]);
}

void DisjointSetUnion::unionByRank(int a, int b) {
    int parA = find(a);
    int parB = find(b);

    if (parA == parB) return;

    if (rank[parA] == rank[parB]) {
        par[parB] = parA;
        rank[parA]++;
    } else if (rank[parA] > rank[parB]) {
        par[parB] = parA;
    } else {
        par[parA] = parB;
    }
    componentCount--;
}

bool DisjointSetUnion::isConnected(int a, int b) {
    return find(a) == find(b);
}

std::size_t DisjointSetUnion::getComponentCount() const {
    return componentCount;
}

std::unordered_map<int, std::vector<int>> DisjointSetUnion::getCommunities(const std::vector<int>& activeNodes) {
    std::unordered_map<int, std::vector<int>> communities;
    for (int node : activeNodes) {
        int root = find(node);
        communities[root].push_back(node);
    }
    return communities;
}