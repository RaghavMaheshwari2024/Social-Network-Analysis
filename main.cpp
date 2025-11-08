#include "data_loader.h" // Include your header file
#include <fstream>
#include <sstream>
#include <algorithm>
using namespace std;

// --- Data Loading Function ---
void load_graph_from_file(Graph& g, const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Assume two IDs per line, separated by a space/tab
        std::stringstream ss(line);
        NodeID u, v;

        // Extract two IDs from the line
        if (ss >> u >> v) {
            g.add_edge(u, v);
        }
    }
    std::cout << "Graph loaded successfully from " << filename << std::endl;
}

//custom comparator for sorting the scores_vec to get the top k seeds
bool Compare(const pair<int, double>& a, const pair<int, double>& b){
        return a.second > b.second;
}

// --- Main function for testing Week 1 ---
int main() {
    Graph my_network;
    const std::string filename = "0.edges"; 

    // Load the graph
    load_graph_from_file(my_network, filename);

    // Verify the load (optional: check a known node from your file)
    const auto& adj_list = my_network.get_adj_list();
    std::cout << "Total Nodes in Graph: " << adj_list.size() << std::endl;

    // Example check: Find the neighbors of Node ID 0
    if (adj_list.count(0)) {
        const auto& neighbors = my_network.get_neighbors(0);
        std::cout << "Node 0 has " << neighbors.size() << " neighbors." << std::endl;
    } else {
        std::cout << "Node 0 not found in the graph." << std::endl;
    }

    //computing the betweenness centrality for my_network
    unordered_map betweenness_centrality_scores = my_network.compute_betweenness_centrality();
    vector<pair<int, double>> scores_vec;
    cout << "Computing Betweenness centrality of the graph..." << endl;

    for(const auto& node_score: betweenness_centrality_scores){
        scores_vec.push_back(node_score);
    }

    sort(scores_vec.begin(), scores_vec.end(), Compare);

    cout << "==== Top 5 Influencers ===="<< endl;
    for(int i = 0; i < 5 && i < scores_vec.size(); ++i){
        cout << "Rank " << (i + 1) << ": Node " << scores_vec[i].first << " (Score: " << scores_vec[i].second << ")" << endl;
    }
    cout << "===========================" << endl;

    return 0;
}