#ifndef CHORDAL_ANALYZER_HPP
#define CHORDAL_ANALYZER_HPP

#include "graph.hpp"

class ChordalAnalyzer {
private:
    const Graph& graph;
    std::vector<int> getEliminationOrdering() const;

public:
    explicit ChordalAnalyzer(const Graph& g);

    std::vector<int> getPerfectEliminationOrdering() const;
    bool isChordal() const;
    std::size_t getMaxCliqueSize() const;
};

#endif // CHORDAL_ANALYZER_HPP