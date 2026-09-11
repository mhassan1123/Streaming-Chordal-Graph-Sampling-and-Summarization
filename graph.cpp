#include "graph.hpp"
#include <queue>
#include <algorithm>
#include <cmath>

Graph::Graph() : edgeCount(0) {}

void Graph::addVertex(int u) {
    if (adj.find(u) == adj.end()) {
        adj[u] = std::unordered_set<int>();
        degrees[u] = 0;
        dsu.ensureSize(u);
    }
}

void Graph::addEdge(int u, int v) {
    if (u == v) return;

    addVertex(u);
    addVertex(v);

    if (adj[u].find(v) == adj[u].end()) {
        adj[u].insert(v);
        adj[v].insert(u);

        degrees[u]++;
        degrees[v]++;
        edgeCount++;

        dsu.unionByRank(u, v);
    }
}

void Graph::removeEdge(int u, int v) {
    if (adj.count(u) && adj[u].count(v)) {
        adj[u].erase(v);
        adj[v].erase(u);

        degrees[u]--;
        degrees[v]--;
        edgeCount--;

        dsu = DisjointSetUnion();
        for (const auto& pair : adj) {
            dsu.ensureSize(pair.first);
        }
        for (const auto& pair : adj) {
            int nodeU = pair.first;
            for (int nodeV : pair.second) {
                if (nodeU < nodeV) {
                    dsu.unionByRank(nodeU, nodeV);
                }
            }
        }
    }
}

bool Graph::hasEdge(int u, int v) const {
    auto it = adj.find(u);
    return (it != adj.end() && it->second.count(v));
}

std::vector<int> Graph::getNeighbors(int u) const {
    auto it = adj.find(u);
    if (it != adj.end()) {
        return std::vector<int>(it->second.begin(), it->second.end());
    }
    return {};
}

std::size_t Graph::getDegree(int u) const {
    auto it = degrees.find(u);
    if (it != degrees.end()) {
        return it->second;
    }
    return 0;
}

std::vector<int> Graph::getVertices() const {
    std::vector<int> vertices;
    vertices.reserve(adj.size());
    for (const auto& pair : adj) {
        vertices.push_back(pair.first);
    }
    return vertices;
}

std::size_t Graph::getEdgeCount() const { return edgeCount; }
std::size_t Graph::getVertexCount() const { return adj.size(); }
std::size_t Graph::getConnectedComponentsCount() const { return dsu.getComponentCount(); }

std::vector<std::pair<int, std::size_t>> Graph::getTopKNodes(std::size_t k) const {
    using DegreeNodePair = std::pair<std::size_t, int>;
    std::priority_queue<DegreeNodePair, std::vector<DegreeNodePair>, std::greater<DegreeNodePair>> minHeap;

    for (const auto& entry : degrees) {
        minHeap.push({entry.second, entry.first});
        if (minHeap.size() > k) minHeap.pop();
    }

    std::vector<std::pair<int, std::size_t>> topK;
    while (!minHeap.empty()) {
        auto top = minHeap.top();
        minHeap.pop();
        topK.push_back({top.second, top.first});
    }
    std::reverse(topK.begin(), topK.end());
    return topK;
}

std::unordered_map<int, double> Graph::computePageRank(double dampingFactor, int maxIterations, double tol) const {
    std::unordered_map<int, double> pageRank;
    std::size_t N = getVertexCount();
    if (N == 0) return pageRank;

    double initialRank = 1.0 / N;
    for (const auto& pair : adj) {
        pageRank[pair.first] = initialRank;
    }

    std::unordered_map<int, double> newPageRank = pageRank;

    for (int iter = 0; iter < maxIterations; ++iter) {
        double totalDiff = 0.0;

        for (const auto& pair : adj) {
            int u = pair.first;
            double incomingRankSum = 0.0;

            for (int v : pair.second) {
                std::size_t vDegree = degrees.at(v);
                if (vDegree > 0) {
                    incomingRankSum += pageRank[v] / static_cast<double>(vDegree);
                }
            }

            newPageRank[u] = ((1.0 - dampingFactor) / N) + (dampingFactor * incomingRankSum);
            totalDiff += std::abs(newPageRank[u] - pageRank[u]);
        }

        pageRank = newPageRank;
        if (totalDiff < tol) break;
    }

    return pageRank;
}

std::unordered_map<int, std::vector<int>> Graph::detectCommunities() {
    return dsu.getCommunities(getVertices());
}

const std::unordered_map<int, std::unordered_set<int>>& Graph::getAdjacencyMap() const {
    return adj;
}

Graph Graph::getInducedSubgraph(const std::vector<int>& nodeSubset) const {
    Graph sub;
    std::unordered_set<int> nodeSet(nodeSubset.begin(), nodeSubset.end());

    for (int u : nodeSubset) {
        sub.addVertex(u);
        auto it = adj.find(u);
        if (it != adj.end()) {
            for (int v : it->second) {
                if (nodeSet.count(v) && u < v) {
                    sub.addEdge(u, v);
                }
            }
        }
    }
    return sub;
}