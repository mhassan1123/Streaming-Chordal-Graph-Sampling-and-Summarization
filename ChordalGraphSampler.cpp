#include "ChordalGraphSampler.hpp"
#include <iostream>

ChordalGraphSampler::ChordalGraphSampler() : rng(std::random_device{}()) {}

bool ChordalGraphSampler::addRandomEdge(Graph& candidate) {
    std::vector<int> vertices = candidate.getVertices();
    std::size_t n = vertices.size();
    if (n < 2) return false;

    std::uniform_int_distribution<std::size_t> dist(0, n - 1);
    int u = vertices[dist(rng)];
    int v = vertices[dist(rng)];

    if (u != v && !candidate.hasEdge(u, v)) {
        candidate.addEdge(u, v);
        return true;
    }
    return false;
}

bool ChordalGraphSampler::removeRandomEdge(Graph& candidate) {
    std::vector<int> vertices = candidate.getVertices();
    std::size_t n = vertices.size();
    if (n < 2) return false;

    std::uniform_int_distribution<std::size_t> dist(0, n - 1);
    int u = vertices[dist(rng)];
    std::vector<int> neighbors = candidate.getNeighbors(u);

    if (!neighbors.empty()) {
        std::uniform_int_distribution<std::size_t> neighborDist(0, neighbors.size() - 1);
        int v = neighbors[neighborDist(rng)];
        candidate.removeEdge(u, v);
        return true;
    }
    return false;
}

bool ChordalGraphSampler::toggleEdge(Graph& candidate) {
    std::vector<int> vertices = candidate.getVertices();
    std::size_t n = vertices.size();
    if (n < 2) return false;

    std::uniform_int_distribution<std::size_t> dist(0, n - 1);
    int u = vertices[dist(rng)];
    int v = vertices[dist(rng)];

    if (u == v) return false;

    if (candidate.hasEdge(u, v)) {
        candidate.removeEdge(u, v);
    } else {
        candidate.addEdge(u, v);
    }
    return true;
}

std::vector<SamplingRecord> ChordalGraphSampler::runSampler(Graph currentGraph, int totalIterations, int logInterval) {
    std::vector<SamplingRecord> records;
    std::uniform_int_distribution<int> moveChoice(0, 2);

    for (int iter = 0; iter <= totalIterations; ++iter) {
        if (iter % logInterval == 0) {
            GraphSummary summary(currentGraph);
            ChordalAnalyzer analyzer(currentGraph);
            
            records.push_back({
                iter,
                summary.getEdgeCount(),
                summary.getDensity(),
                summary.getAverageDegree(),
                analyzer.getMaxCliqueSize()
            });
        }

        if (iter == totalIterations) break;

        Graph candidate = currentGraph;
        int move = moveChoice(rng);
        bool modified = false;

        if (move == 0) modified = addRandomEdge(candidate);
        else if (move == 1) modified = removeRandomEdge(candidate);
        else modified = toggleEdge(candidate);

        if (!modified) continue;

        ChordalAnalyzer candidateAnalyzer(candidate);
        if (candidateAnalyzer.isChordal()) {
            currentGraph = candidate;
        }
    }

    return records;
}