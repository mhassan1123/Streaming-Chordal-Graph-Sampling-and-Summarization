#ifndef GRAPH_SUMMARY_HPP
#define GRAPH_SUMMARY_HPP

#include "graph.hpp"

class GraphSummary {
private:
    const Graph& graph;

public:
    explicit GraphSummary(const Graph& g);

    std::size_t getNodeCount() const;
    std::size_t getEdgeCount() const;
    double getAverageDegree() const;
    double getDensity() const;
    std::size_t getComponents() const;

    std::vector<std::pair<int, std::size_t>> getTopKNodes(std::size_t k) const;
    std::unordered_map<int, double> getPageRank() const;
    std::unordered_map<int, std::vector<int>> getCommunities() const;
};

#endif // GRAPH_SUMMARY_HPP