#include "data_loader.h" // Include your header file
#include <fstream>
#include <sstream>
#include <algorithm>
#include <set>
using namespace std;

// --- Data Loading Function ---
void load_graph_from_file(Graph& g, const string& filename) {
    // This value is still required by the add_edge function signature in data_loader.h
    const double DEFAULT_PROBABILITY = 0.01; 
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        NodeID u, v;

        if (ss >> u >> v) {
            // Pass the required probability argument
            g.add_edge(u, v, DEFAULT_PROBABILITY);
        }
    }
    cout << "Graph loaded successfully from " << filename << endl;
}


//custom comparator for sorting the scores_vec to get the top k seeds
bool Compare(const pair<int, double>& a, const pair<int, double>& b){
        return a.second > b.second;
}

// --- Main function for testing Week 1 ---
int main() {
    Graph my_network;
    const string filename = "0.edges"; 
    const int K_SEEDS = 5;
    const int NUM_SIMULATIONS = 10000;

    // 1. Load the graph
    load_graph_from_file(my_network, filename);
    const auto& adj_list = my_network.get_adj_list();
    cout << "Total Nodes in Graph: " << adj_list.size() << endl;

    // --- THIS IS THE CRITICAL CHANGE ---
    // We are now calling your member function from data_loader.h
    unordered_map<NodeID, double> bc_scores = my_network.compute_betweenness_centrality();
    // ------------------------------------

    // Prepare for sorting
    vector<pair<double, NodeID>> sorted_nodes;
    for (const auto& pair : bc_scores) {
        sorted_nodes.push_back({pair.first, pair.second});
    }
    // Sort descending by BC score
    sort(sorted_nodes.begin(), sorted_nodes.end(), Compare);

    // Select the top K nodes
    set<NodeID> initial_seed_set;
    cout << "Top " << K_SEEDS << " Seeds (via BC Score):" << endl;
    for (int i = 0; i < min((int)sorted_nodes.size(), K_SEEDS); ++i) {
        initial_seed_set.insert(sorted_nodes[i].second);
        cout << "  Seed " << i+1 << ": Node " << sorted_nodes[i].second 
                  << " (Score: " << sorted_nodes[i].first << ")" << endl;
    }
}