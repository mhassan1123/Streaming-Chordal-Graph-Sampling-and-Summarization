#include "ChordalGraphSampler.hpp"
#include <algorithm>

ChordalGraphSampler::ChordalGraphSampler(unsigned int seed) : rng(seed) {}

// Sorted so that a fixed seed gives identical runs (unordered_map order is unspecified).
static std::vector<int> sortedVertices(const Graph& g) {
    std::vector<int> v = g.getVertices();
    std::sort(v.begin(), v.end());
    return v;
}

std::vector<std::pair<int, int>> ChordalGraphSampler::extractEdges(const Graph& g) {
    std::vector<std::pair<int, int>> edges;
    for (const auto& kv : g.getAdjacencyMap())
        for (int v : kv.second)
            if (kv.first < v) edges.emplace_back(kv.first, v);
    std::sort(edges.begin(), edges.end());
    return edges;
}

bool ChordalGraphSampler::addRandomEdge(Graph& candidate) {
    std::vector<int> vertices = sortedVertices(candidate);
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
    std::vector<int> vertices = sortedVertices(candidate);
    std::size_t n = vertices.size();
    if (n < 2) return false;

    std::uniform_int_distribution<std::size_t> dist(0, n - 1);
    int u = vertices[dist(rng)];
    std::vector<int> neighbors = candidate.getNeighbors(u);
    std::sort(neighbors.begin(), neighbors.end());

    if (!neighbors.empty()) {
        std::uniform_int_distribution<std::size_t> neighborDist(0, neighbors.size() - 1);
        int v = neighbors[neighborDist(rng)];
        candidate.removeEdge(u, v);
        return true;
    }
    return false;
}

bool ChordalGraphSampler::toggleEdge(Graph& candidate) {
    std::vector<int> vertices = sortedVertices(candidate);
    std::size_t n = vertices.size();
    if (n < 2) return false;

    std::uniform_int_distribution<std::size_t> dist(0, n - 1);
    int u = vertices[dist(rng)];
    int v = vertices[dist(rng)];

    if (u == v) return false;

    if (candidate.hasEdge(u, v)) candidate.removeEdge(u, v);
    else candidate.addEdge(u, v);
    return true;
}

std::vector<SamplingRecord> ChordalGraphSampler::runSampler(Graph currentGraph, int totalIterations,
                                                            int logInterval, bool captureSnapshots) {
    std::vector<SamplingRecord> records;
    std::uniform_int_distribution<int> moveChoice(0, 2);
    stats = SamplerStats();
    if (logInterval <= 0) logInterval = 1;

    for (int iter = 0; iter <= totalIterations; ++iter) {
        if (iter % logInterval == 0) {
            GraphSummary summary(currentGraph);
            ChordalAnalyzer analyzer(currentGraph);

            SamplingRecord rec{iter, summary.getEdgeCount(), summary.getDensity(),
                               summary.getAverageDegree(), analyzer.getMaxCliqueSize(), {}};
            if (captureSnapshots) rec.edgeList = extractEdges(currentGraph);
            records.push_back(std::move(rec));
        }

        if (iter == totalIterations) break;

        stats.proposals++;
        Graph candidate = currentGraph;
        int move = moveChoice(rng);
        bool modified = false;

        if (move == 0) modified = addRandomEdge(candidate);
        else if (move == 1) modified = removeRandomEdge(candidate);
        else modified = toggleEdge(candidate);

        if (!modified) { stats.noOps++; continue; }

        ChordalAnalyzer candidateAnalyzer(candidate);
        if (candidateAnalyzer.isChordal()) {
            currentGraph = candidate;
            stats.accepted++;
        } else {
            stats.rejected++;
        }
    }

    return records;
}