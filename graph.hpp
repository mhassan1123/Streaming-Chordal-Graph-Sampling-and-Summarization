#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <cstddef>
#include "dsu.hpp"

class Graph {
private:
    std::unordered_map<int, std::unordered_set<int>> adj;
    std::unordered_map<int, std::size_t> degrees;
    std::size_t edgeCount;
    DisjointSetUnion dsu;

public:
    Graph();

    void addVertex(int u);
    void addEdge(int u, int v);
    void removeEdge(int u, int v);

    bool hasEdge(int u, int v) const;
    std::vector<int> getNeighbors(int u) const;
    std::size_t getDegree(int u) const;
    std::vector<int> getVertices() const;

    std::size_t getEdgeCount() const;
    std::size_t getVertexCount() const;
    std::size_t getConnectedComponentsCount() const;

    std::vector<std::pair<int, std::size_t>> getTopKNodes(std::size_t k) const;
    std::unordered_map<int, double> computePageRank(double dampingFactor = 0.85, int maxIterations = 20, double tol = 1e-6) const;
    std::unordered_map<int, std::vector<int>> detectCommunities();

    const std::unordered_map<int, std::unordered_set<int>>& getAdjacencyMap() const;
    Graph getInducedSubgraph(const std::vector<int>& nodeSubset) const;
};

#endif // GRAPH_HPP