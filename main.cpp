#include "data_loader.h" 
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <queue>
#include <stack>
#include <random>
#include <cmath> 

// Using namespace std for simplicity as requested
using namespace std; 

// --- Helper Functions and Influence Model Definitions ---

// Function to convert common neighbors to probability (Dynamic p)
double calculate_influence_probability(int common_neighbors) {
    // Simple linear scaling: 10 common friends = 100% influence chance.
    // Adjust SCALING_FACTOR to control how quickly influence spreads.
    const double SCALING_FACTOR = 0.1; 
    
    double p = common_neighbors * SCALING_FACTOR;
    return min(1.0, p);
}


// --- 1. COMMON NEIGHBOR COUNTER ---
int count_common_neighbors(const Graph& g, NodeID A, NodeID B) {
    // Uses a set for O(1) average lookup time.
    set<NodeID> neighbors_A;
    for (const auto& edge : g.get_neighbors(A)) {
        neighbors_A.insert(edge.target_node);
    }
    
    int common_count = 0;
    
    for (const auto& edge : g.get_neighbors(B)) {
        NodeID v = edge.target_node;
        
        if (v == A || v == B) continue; 
        
        if (neighbors_A.count(v)) {
            common_count++;
        }
    }
    return common_count;
}


// --- 2. BETWEENNESS CENTRALITY (Brandes' Algorithm) ---
unordered_map<NodeID, double> calculate_betweenness_centrality(const Graph& g) {
    unordered_map<NodeID, double> BC;
    vector<NodeID> all_nodes;
    
    // Get all unique nodes and initialize BC scores
    for (const auto& pair : g.get_adj_list()) {
        all_nodes.push_back(pair.first);
        BC[pair.first] = 0.0; 
    }
    
    // Iterate over all nodes 's' (source)
    for (NodeID s : all_nodes) {
        stack<NodeID> S; 
        queue<NodeID> Q; 
        
        // P[w] stores predecessors on shortest paths from s to w
        unordered_map<NodeID, vector<NodeID>> P; 
        unordered_map<NodeID, int> sigma; // Number of shortest paths from s to w
        unordered_map<NodeID, int> dist;  // Shortest distance from s to w
        
        // --- FORWARD PASS (BFS) Initialization ---
        sigma[s] = 1;
        dist[s] = 0;
        Q.push(s);

        // --- FORWARD PASS ---
        while (!Q.empty()) {
            NodeID v = Q.front();
            Q.pop();
            S.push(v);
            
            if (P.find(v) == P.end()) { P[v] = vector<NodeID>(); }

            // Iterate through neighbors 'w'
            for (const auto& edge : g.get_neighbors(v)) {
                NodeID w = edge.target_node;
                
                // 1. Node w discovered for the first time
                if (dist.find(w) == dist.end()) {
                    dist[w] = dist[v] + 1;
                    sigma[w] = 0;
                    Q.push(w);
                }
                
                // 2. Node w found on a shortest path from s to v
                if (dist[w] == dist[v] + 1) {
                    sigma[w] += sigma[v]; 
                    
                    if (P.find(w) == P.end()) { P[w] = vector<NodeID>(); }
                    P[w].push_back(v); 
                }
            }
        }
        
        // --- BACKWARD PASS (Accumulation) ---
        unordered_map<NodeID, double> delta; // Dependency score
        for (NodeID node : all_nodes) { delta[node] = 0.0; }

        while (!S.empty()) {
            NodeID w = S.top();
            S.pop();
            
            // Calculate dependency delta_s(w) for its predecessors 'v'
            for (NodeID v : P[w]) {
                double fraction = (double)sigma[v] / sigma[w];
                delta[v] += fraction * (1.0 + delta[w]);
            }
            
            // Add final dependency to BC score (if w is not the source s)
            if (w != s) {
                BC[w] += delta[w];
            }
        }
    }
    return BC;
}


// --- 3. DYNAMIC ICM SIMULATION ---
int simulate_ICM(const Graph& g, const set<NodeID>& S, int num_simulations) {
    long long total_spread = 0;
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0.0, 1.0);

    for (int i = 0; i < num_simulations; ++i) {
        set<NodeID> active = S;
        queue<NodeID> q;
        for (NodeID node : S) q.push(node);
        
        while (!q.empty()) {
            NodeID u = q.front(); q.pop();

            for (const auto& edge : g.get_neighbors(u)) {
                NodeID v = edge.target_node;

                if (active.find(v) == active.end()) {
                    int common_count = count_common_neighbors(g, u, v);
                    double p_uv = calculate_influence_probability(common_count); 

                    if (dis(gen) < p_uv) { 
                        active.insert(v);
                        q.push(v);
                    }
                }
            }
        }
        total_spread += active.size();
    }
    return (int)(total_spread / num_simulations); 
}


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


// --- Main function: Driver for Hybrid Influence Maximization ---
int main() {
    Graph my_network;
    const string filename = "0.edges"; 
    const int K_SEEDS = 5;
    const int NUM_SIMULATIONS = 10000;

    // 1. Load the graph
    load_graph_from_file(my_network, filename);
    const auto& adj_list = my_network.get_adj_list();
    cout << "Total Nodes in Graph: " << adj_list.size() << endl;


    // 2. Calculate Betweenness Centrality (BC) and select K seeds
    cout << "\n--- Phase 1: Calculating Betweenness Centrality for Seed Selection ---" << endl;
    unordered_map<NodeID, double> bc_scores = calculate_betweenness_centrality(my_network);

    // Prepare for sorting
    vector<pair<double, NodeID>> sorted_nodes;
    for (const auto& pair : bc_scores) {
        sorted_nodes.push_back({pair.second, pair.first});
    }
    // Sort descending by BC score
    sort(sorted_nodes.begin(), sorted_nodes.end(),
              [](const pair<double, NodeID>& a, const pair<double, NodeID>& b){
                  return a.first > b.first;
              });

    // Select the top K nodes
    set<NodeID> initial_seed_set;
    cout << "Top " << K_SEEDS << " Seeds (via BC Score):" << endl;
    for (int i = 0; i < min((int)sorted_nodes.size(), K_SEEDS); ++i) {
        initial_seed_set.insert(sorted_nodes[i].second);
        cout << "  Seed " << i+1 << ": Node " << sorted_nodes[i].second 
                  << " (Score: " << sorted_nodes[i].first << ")" << endl;
    }


    // 3. Run Dynamic ICM Spread
    cout << "\n--- Phase 2: Running Dynamic ICM Spread (Sims: " << NUM_SIMULATIONS << ") ---" << endl;
    int final_spread = simulate_ICM(my_network, initial_seed_set, NUM_SIMULATIONS);

    cout << "--------------------------------------------------------" << endl;
    cout << "FINAL RESULT: BC seeds achieved an average influence spread of " 
              << final_spread << " total nodes." << endl;
    cout << "--------------------------------------------------------" << endl;
    
    return 0;
}