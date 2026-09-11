#include "graph_summary.hpp"

GraphSummary::GraphSummary(const Graph& g) : graph(g) {}

std::size_t GraphSummary::getNodeCount() const { return graph.getVertexCount(); }
std::size_t GraphSummary::getEdgeCount() const { return graph.getEdgeCount(); }

double GraphSummary::getAverageDegree() const {
    std::size_t V = getNodeCount();
    return (V > 0) ? (2.0 * getEdgeCount()) / V : 0.0;
}

double GraphSummary::getDensity() const {
    std::size_t V = getNodeCount();
    return (V > 1) ? (2.0 * getEdgeCount()) / (V * (V - 1.0)) : 0.0;
}

std::size_t GraphSummary::getComponents() const { return graph.getConnectedComponentsCount(); }
std::vector<std::pair<int, std::size_t>> GraphSummary::getTopKNodes(std::size_t k) const { return graph.getTopKNodes(k); }
std::unordered_map<int, double> GraphSummary::getPageRank() const { return graph.computePageRank(); }

std::unordered_map<int, std::vector<int>> GraphSummary::getCommunities() const {
    Graph tempGraph = graph; 
    return tempGraph.detectCommunities();
}