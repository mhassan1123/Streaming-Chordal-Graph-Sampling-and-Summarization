// chordal_api: stdin -> JSON on stdout. Called by server/server.js.
// stdin line 1 : CONFIG <seed> <iterations> <logInterval> <topK>
// stdin rest   : one edge per line, "u,v" (same format as graph_stream.csv)
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <string>
#include "graph.hpp"
#include "graph_summary.hpp"
#include "ChordalAnalyzer.hpp"
#include "ChordalGraphSampler.hpp"

using Edges = std::vector<std::pair<int, int>>;

static std::string num(double x) {
    if (!std::isfinite(x)) return "0";
    std::ostringstream o; o << std::setprecision(9) << x; return o.str();
}
static std::string edgesJson(const Edges& e) {
    std::string s = "[";
    for (std::size_t i = 0; i < e.size(); ++i) {
        if (i) s += ",";
        s += "[" + std::to_string(e[i].first) + "," + std::to_string(e[i].second) + "]";
    }
    return s + "]";
}
static std::string intsJson(std::vector<int> v) {
    std::sort(v.begin(), v.end());
    std::string s = "[";
    for (std::size_t i = 0; i < v.size(); ++i) { if (i) s += ","; s += std::to_string(v[i]); }
    return s + "]";
}
static const char* b(bool x) { return x ? "true" : "false"; }

int main() {
    std::string line;
    unsigned seed = 1; int iterations = 200, logInterval = 10; std::size_t topK = 5;
    Graph g;

    while (std::getline(std::cin, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue;
        if (line.rfind("CONFIG", 0) == 0) {
            std::istringstream c(line.substr(6));
            c >> seed >> iterations >> logInterval >> topK;
            continue;
        }
        for (char& ch : line) if (ch == ',' || ch == ';' || ch == '\t') ch = ' ';
        std::istringstream ss(line);
        int u, v;
        if (ss >> u >> v) g.addEdge(u, v);
    }

    GraphSummary sum(g);
    ChordalAnalyzer an(g);
    auto pr = sum.getPageRank();
    auto comms = sum.getCommunities();

    std::vector<std::vector<int>> groups;
    for (auto& kv : comms) groups.push_back(kv.second);
    for (auto& grp : groups) std::sort(grp.begin(), grp.end());
    std::sort(groups.begin(), groups.end(), [](auto& a, auto& c) {
        return a.size() != c.size() ? a.size() > c.size() : a[0] < c[0]; });
    std::unordered_map<int, int> commOf;
    for (std::size_t i = 0; i < groups.size(); ++i) for (int v : groups[i]) commOf[v] = (int)i;

    std::ostringstream o;
    o << "{\"stream\":{\"nodes\":" << sum.getNodeCount() << ",\"edges\":" << sum.getEdgeCount()
      << ",\"avgDegree\":" << num(sum.getAverageDegree()) << ",\"density\":" << num(sum.getDensity())
      << ",\"components\":" << sum.getComponents() << ",\"maxClique\":" << an.getMaxCliqueSize()
      << ",\"chordal\":" << b(an.isChordal())
      << ",\"edgeList\":" << edgesJson(ChordalGraphSampler::extractEdges(g)) << "}";

    std::vector<int> verts = g.getVertices(); std::sort(verts.begin(), verts.end());
    o << ",\"nodes\":[";
    for (std::size_t i = 0; i < verts.size(); ++i) {
        int v = verts[i];
        o << (i ? "," : "") << "{\"id\":" << v << ",\"degree\":" << g.getDegree(v)
          << ",\"pageRank\":" << num(pr[v]) << ",\"community\":" << commOf[v] << "}";
    }
    o << "]";

    auto top = g.getTopKNodes(topK);
    std::vector<int> targets;
    o << ",\"topK\":[";
    for (std::size_t i = 0; i < top.size(); ++i) {
        targets.push_back(top[i].first);
        o << (i ? "," : "") << "{\"node\":" << top[i].first << ",\"degree\":" << top[i].second << "}";
    }
    o << "],\"communities\":[";
    for (std::size_t i = 0; i < groups.size(); ++i) o << (i ? "," : "") << intsJson(groups[i]);
    o << "]";

    Graph sub = g.getInducedSubgraph(targets);
    GraphSummary subSum(sub); ChordalAnalyzer subAn(sub);
    o << ",\"subgraph\":{\"nodes\":" << intsJson(sub.getVertices()) << ",\"edges\":" << subSum.getEdgeCount()
      << ",\"density\":" << num(subSum.getDensity()) << ",\"avgDegree\":" << num(subSum.getAverageDegree())
      << ",\"maxClique\":" << subAn.getMaxCliqueSize() << ",\"chordal\":" << b(subAn.isChordal())
      << ",\"edgeList\":" << edgesJson(ChordalGraphSampler::extractEdges(sub)) << "}";

    ChordalGraphSampler sampler(seed);
    auto hist = sampler.runSampler(sub, iterations, logInterval, true);
    const SamplerStats& st = sampler.getStats();
    o << ",\"mcmc\":{\"seed\":" << seed << ",\"iterations\":" << iterations << ",\"logInterval\":" << logInterval
      << ",\"stats\":{\"proposals\":" << st.proposals << ",\"noOps\":" << st.noOps
      << ",\"accepted\":" << st.accepted << ",\"rejected\":" << st.rejected << "},\"records\":[";
    for (std::size_t i = 0; i < hist.size(); ++i) {
        const auto& r = hist[i];
        o << (i ? "," : "") << "{\"iteration\":" << r.iteration << ",\"edges\":" << r.edges
          << ",\"density\":" << num(r.density) << ",\"avgDegree\":" << num(r.avgDegree)
          << ",\"maxClique\":" << r.maxCliqueSize << ",\"edgeList\":" << edgesJson(r.edgeList) << "}";
    }
    o << "]}}";
    std::cout << o.str() << std::endl;
    return 0;
}