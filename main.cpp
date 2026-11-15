#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "graph.hpp"
#include "graph_summary.hpp"
#include "ChordalAnalyzer.hpp"
#include "ChordalGraphSampler.hpp"

using namespace std;

void createSampleCSV(const string& filename) {
    ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << "1,2\n2,3\n3,1\n3,4\n4,5\n5,3\n";
        outFile << "5,6\n6,7\n7,5\n7,8\n8,9\n9,7\n";
        outFile << "2,6\n4,8\n1,9\n3,7\n";
        outFile.close();
    }
}

int main() {
    string csvPath = "graph_stream.csv";
    createSampleCSV(csvPath);

    ifstream stream(csvPath);
    if (!stream.is_open()) {
        cerr << "Error: Could not open stream file: " << csvPath << endl;
        return 1;
    }

    Graph streamingGraph;
    string line;

    cout << "==========================================================" << endl;
    cout << "  STAGE 1: STREAMING ENGINE INGESTION & SUMMARY          " << endl;
    cout << "==========================================================" << endl;

    while (getline(stream, line)) {
        if (line.empty()) continue;
        // \r for carraige return 
        if (!line.empty() && line.back() == '\r') line.pop_back();

        stringstream ss(line);
        string uStr, vStr;

        if (getline(ss, uStr, ',') && getline(ss, vStr, ',')) {
            try {
                streamingGraph.addEdge(stoi(uStr), stoi(vStr));
            } catch (...) {}
        }
    }

    GraphSummary rawSummary(streamingGraph);
    ChordalAnalyzer rawAnalyzer(streamingGraph);

    cout << "[Stream Summary]" << endl;
    cout << "  Total Ingested Nodes     : " << rawSummary.getNodeCount() << endl;
    cout << "  Total Ingested Edges     : " << rawSummary.getEdgeCount() << endl;
    cout << "  Stream Average Degree    : " << fixed << setprecision(3) << rawSummary.getAverageDegree() << endl;
    cout << "  Stream Graph Density     : " << rawSummary.getDensity() << endl;
    cout << "  Connected Components     : " << rawSummary.getComponents() << endl;

    cout << "\n==========================================================" << endl;
    cout << "  STAGE 2: STREAMING FEATURE & COMMUNITY EXTRACTION        " << endl;
    cout << "==========================================================" << endl;

    auto topDegreeNodes = streamingGraph.getTopKNodes(5);
    cout << "Top Nodes by Degree:" << endl;
    vector<int> targetVertices;
    for (const auto& p : topDegreeNodes) {
        cout << "  Node " << p.first << " (Degree: " << p.second << ")" << endl;
        targetVertices.push_back(p.first);
    }

    unordered_map<int, vector<int>> communities = rawSummary.getCommunities();
    cout << "Detected " << communities.size() << " connected community clusters." << endl;

    cout << "\n==========================================================" << endl;
    cout << "  STAGE 3: SUBGRAPH EXTRACTION FOR CHORDAL ANALYSIS       " << endl;
    cout << "==========================================================" << endl;

    Graph inducedSubgraph = streamingGraph.getInducedSubgraph(targetVertices);
    GraphSummary subSummary(inducedSubgraph);

    cout << "[Extracted Subgraph Properties]" << endl;
    cout << "  Nodes in Subgraph        : " << subSummary.getNodeCount() << endl;
    cout << "  Edges in Subgraph        : " << subSummary.getEdgeCount() << endl;
    cout << "  Subgraph Density         : " << subSummary.getDensity() << endl;

    ChordalAnalyzer subAnalyzer(inducedSubgraph);
    cout << "  Is Subgraph Chordal?     : " << (subAnalyzer.isChordal() ? "YES" : "NO") << endl;

    cout << "\n==========================================================" << endl;
    cout << "  STAGE 4: MARKOV CHAIN MONTE CARLO SAMPLING              " << endl;
    cout << "==========================================================" << endl;

    ChordalGraphSampler sampler;
    vector<SamplingRecord> history = sampler.runSampler(inducedSubgraph, 50, 10);

    cout << left << setw(12) << "Iteration" 
         << setw(10) << "Edges" 
         << setw(12) << "Density" 
         << setw(14) << "Avg Degree" 
         << setw(16) << "Max Clique" << endl;
    cout << string(64, '-') << endl;

    for (const auto& rec : history) {
        cout << left << setw(12) << rec.iteration 
             << setw(10) << rec.edges 
             << setw(12) << fixed << setprecision(4) << rec.density 
             << setw(14) << fixed << setprecision(4) << rec.avgDegree 
             << setw(16) << rec.maxCliqueSize << endl;
    }

    cout << "\n==========================================================" << endl;
    cout << "  STAGE 5: INTEGRATED STRUCTURAL COMPARISON ANALYSIS     " << endl;
    cout << "==========================================================" << endl;

    cout << "PROPERTY COMPARISON:" << endl;
    cout << "  Metric          | Raw Stream | Extracted Subgraph | Final MCMC State" << endl;
    cout << "  ----------------|------------|--------------------|------------------" << endl;
    cout << "  Edges           | " << setw(10) << rawSummary.getEdgeCount() 
         << " | " << setw(18) << subSummary.getEdgeCount() 
         << " | " << history.back().edges << endl;
    cout << "  Density         | " << setw(10) << setprecision(3) << rawSummary.getDensity() 
         << " | " << setw(18) << setprecision(3) << subSummary.getDensity() 
         << " | " << setprecision(3) << history.back().density << endl;
    cout << "  Avg Degree      | " << setw(10) << setprecision(3) << rawSummary.getAverageDegree() 
         << " | " << setw(18) << setprecision(3) << subSummary.getAverageDegree() 
         << " | " << setprecision(3) << history.back().avgDegree << endl;
    cout << "  Max Clique Size | " << setw(10) << rawAnalyzer.getMaxCliqueSize() 
         << " | " << setw(18) << subAnalyzer.getMaxCliqueSize() 
         << " | " << history.back().maxCliqueSize << endl;

    cout << "\n==========================================================\n" << endl;
    return 0;
}