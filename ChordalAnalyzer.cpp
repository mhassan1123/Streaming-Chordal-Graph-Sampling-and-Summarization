#include "ChordalAnalyzer.hpp"
#include <algorithm>

ChordalAnalyzer::ChordalAnalyzer(const Graph& g) : graph(g) {}

std::vector<int> ChordalAnalyzer::getEliminationOrdering() const {
    std::vector<int> vertices = graph.getVertices();
    std::size_t n = vertices.size();
    if (n == 0) return {};

    std::unordered_map<int, int> weight;
    std::unordered_set<int> unvisited(vertices.begin(), vertices.end());
    std::vector<int> peo;
    peo.reserve(n);

    for (int v : vertices) weight[v] = 0;

    for (std::size_t i = 0; i < n; ++i) {
        int maxNode = -1, maxWeight = -1;

        for (int v : unvisited) {
            if (weight[v] > maxWeight) {
                maxWeight = weight[v];
                maxNode = v;
            }
        }

        if (maxNode == -1) break;

        peo.push_back(maxNode);
        unvisited.erase(maxNode);

        for (int neighbor : graph.getNeighbors(maxNode)) {
            if (unvisited.count(neighbor)) {
                weight[neighbor]++;
            }
        }
    }
    return peo;
}

std::vector<int> ChordalAnalyzer::getPerfectEliminationOrdering() const {
    return getEliminationOrdering();
}

bool ChordalAnalyzer::isChordal() const {
    std::vector<int> order = getEliminationOrdering();
    std::size_t n = order.size();
    if (n <= 3) return true;

    std::unordered_map<int, int> position;
    for (std::size_t i = 0; i < n; ++i) {
        position[order[i]] = static_cast<int>(i);
    }

    const auto& adj = graph.getAdjacencyMap();

    for (std::size_t i = 0; i < n; ++i) {
        int u = order[i];
        std::vector<int> higherNeighbors;

        for (int v : adj.at(u)) {
            if (position[v] > position[u]) {
                higherNeighbors.push_back(v);
            }
        }

        for (std::size_t j = 0; j < higherNeighbors.size(); ++j) {
            for (std::size_t k = j + 1; k < higherNeighbors.size(); ++k) {
                int neighborA = higherNeighbors[j];
                int neighborB = higherNeighbors[k];

                if (adj.at(neighborA).find(neighborB) == adj.at(neighborA).end()) {
                    return false; 
                }
            }
        }
    }
    return true;
}

std::size_t ChordalAnalyzer::getMaxCliqueSize() const {
    std::vector<int> order = getEliminationOrdering();
    std::size_t maxCliqueSize = 0;
    std::unordered_map<int, int> position;
    for (std::size_t i = 0; i < order.size(); ++i) {
        position[order[i]] = static_cast<int>(i);
    }

    const auto& adj = graph.getAdjacencyMap();
    for (int u : order) {
        std::size_t higherCount = 0;
        if (adj.count(u)) {
            for (int v : adj.at(u)) {
                if (position[v] > position[u]) {
                    higherCount++;
                }
            }
        }
        maxCliqueSize = std::max(maxCliqueSize, higherCount + 1);
    }
    return maxCliqueSize;
}