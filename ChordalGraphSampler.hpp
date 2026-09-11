#ifndef CHORDAL_GRAPH_SAMPLER_HPP
#define CHORDAL_GRAPH_SAMPLER_HPP

#include "graph.hpp"
#include "graph_summary.hpp"
#include "ChordalAnalyzer.hpp"
#include <random>

struct SamplingRecord {
    int iteration;
    std::size_t edges;
    double density;
    double avgDegree;
    std::size_t maxCliqueSize;
};

class ChordalGraphSampler {
private:
    std::mt19937 rng;

public:
    ChordalGraphSampler();

    bool addRandomEdge(Graph& candidate);
    bool removeRandomEdge(Graph& candidate);
    bool toggleEdge(Graph& candidate);

    std::vector<SamplingRecord> runSampler(Graph currentGraph, int totalIterations, int logInterval = 10);
};

#endif // CHORDAL_GRAPH_SAMPLER_HPP