#ifndef CHORDAL_GRAPH_SAMPLER_HPP
#define CHORDAL_GRAPH_SAMPLER_HPP

#include "graph.hpp"
#include "graph_summary.hpp"
#include "ChordalAnalyzer.hpp"
#include <random>
#include <utility>
#include <vector>

struct SamplingRecord {
    int iteration;
    std::size_t edges;
    double density;
    double avgDegree;
    std::size_t maxCliqueSize;
    // Filled only when runSampler(..., captureSnapshots = true). Lets a UI replay the chain.
    std::vector<std::pair<int, int>> edgeList;
};

// Bookkeeping for the Metropolis-style accept/reject step.
struct SamplerStats {
    std::size_t proposals = 0;   // total iterations
    std::size_t noOps = 0;       // move could not be applied (e.g. picked u == v)
    std::size_t accepted = 0;    // candidate was chordal -> chain moved
    std::size_t rejected = 0;    // candidate was NOT chordal -> chain stayed
};

class ChordalGraphSampler {
private:
    std::mt19937 rng;
    SamplerStats stats;

public:
    // Pass a seed for reproducible runs; default stays non-deterministic.
    explicit ChordalGraphSampler(unsigned int seed = std::random_device{}());

    bool addRandomEdge(Graph& candidate);
    bool removeRandomEdge(Graph& candidate);
    bool toggleEdge(Graph& candidate);

    std::vector<SamplingRecord> runSampler(Graph currentGraph, int totalIterations,
                                           int logInterval = 10, bool captureSnapshots = false);

    const SamplerStats& getStats() const { return stats; }
    static std::vector<std::pair<int, int>> extractEdges(const Graph& g);
};

#endif // CHORDAL_GRAPH_SAMPLER_HPP
