#include "data_loader.h"
#include "integrated_social_network.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <chrono>

using namespace std;

// Load graph from file
void load_graph_from_file(Graph& g, const string& filename) {
    const double DEFAULT_PROBABILITY = 0.01;
    ifstream file(filename);
    
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return;
    }

    string line;
    int edge_count = 0;
    
    while (getline(file, line)) {
        stringstream ss(line);
        NodeID u, v;
        if (ss >> u >> v) {
            g.add_edge(u, v, DEFAULT_PROBABILITY);
            edge_count++;
        }
    }
    
    cout << "✓ Graph loaded: " << edge_count << " edges" << endl;
}

void print_header(const string& title) {
    cout << "\n" << string(80, '=') << endl;
    cout << "  " << title << endl;
    cout << string(80, '=') << endl;
}

void show_menu() {
    cout << "\n" << string(80, '-') << endl;
    cout << "INTEGRATED SOCIAL NETWORK ANALYSIS SYSTEM" << endl;
    cout << string(80, '-') << endl;
    cout << "INFLUENCE MAXIMIZATION:" << endl;
    cout << "  1. Find influential seeds (Betweenness Centrality)" << endl;
    cout << "  2. Run ICM influence spread simulation" << endl;
    cout << "  3. Compare BC vs Greedy seed selection" << endl;
    cout << "\nFRIEND RECOMMENDATION:" << endl;
    cout << "  4. Get friend recommendations for a user" << endl;
    cout << "  5. Find influential friend candidates (HYBRID)" << endl;
    cout << "  6. Analyze recommendation impact on influence spread" << endl;
    cout << "\nGENERAL ANALYSIS:" << endl;
    cout << "  7. Show graph statistics" << endl;
    cout << "  8. Run complete demo (all features)" << endl;
    cout << "  0. Exit" << endl;
    cout << string(80, '-') << endl;
    cout << "Enter choice: ";
}

void show_graph_stats(const Graph& g) {
    print_header("GRAPH STATISTICS");
    const auto& adj = g.get_adj_list();
    cout << "Total Nodes: " << adj.size() << endl;
    
    int total_edges = 0;
    int max_degree = 0;
    double sum_degree = 0;
    
    for (const auto& pair : adj) {
        int degree = pair.second.size();
        total_edges += degree;
        max_degree = max(max_degree, degree);
        sum_degree += degree;
    }
    
    total_edges /= 2;
    cout << "Total Edges: " << total_edges << endl;
    cout << "Average Degree: " << fixed << setprecision(2)
         << (adj.size() > 0 ? sum_degree / adj.size() : 0.0) << endl;
    cout << "Max Degree: " << max_degree << endl;
}

void run_complete_demo(const Graph& g) {
    print_header("COMPLETE SYSTEM DEMONSTRATION");
    const int K_SEEDS = 5;
    const int NUM_SIMS = 1000;
    const auto& adj = g.get_adj_list();
    NodeID sample_user = adj.begin()->first;

    cout << "\n[1/4] Finding influential seeds using Betweenness Centrality..." << endl;
    auto start = chrono::high_resolution_clock::now();
    auto bc_seeds = BetweennessCentrality::get_top_k_nodes(g, K_SEEDS);
    auto end = chrono::high_resolution_clock::now();
    auto bc_time = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    
    cout << "Top " << K_SEEDS << " seeds: ";
    for (NodeID seed : bc_seeds) cout << seed << " ";
    cout << "\nTime taken: " << bc_time << " ms" << endl;

    cout << "\n[2/4] Simulating influence spread with ICM..." << endl;
    set<NodeID> seed_set(bc_seeds.begin(), bc_seeds.end());
    int spread = InfluenceMaximization::simulate_ICM(g, seed_set, NUM_SIMS);
    cout << "Average spread: " << spread << " nodes influenced" << endl;

    cout << "\n[3/4] Generating friend recommendations for User " << sample_user << "..." << endl;
    auto recommendations = FriendRecommendation::get_recommendations(g, sample_user, 5);
    
    if (recommendations.empty()) {
        cout << "No recommendations available for this user." << endl;
    } else {
        cout << left << setw(10) << "User ID" << setw(15) << "Common Friends"
             << setw(15) << "Jaccard" << setw(15) << "Adamic-Adar" << endl;
        cout << string(55, '-') << endl;
        for (const auto& rec : recommendations) {
            cout << left << setw(10) << rec.candidate_id
                 << setw(15) << rec.common_neighbors_count
                 << setw(15) << fixed << setprecision(4) << rec.jaccard_score
                 << setw(15) << fixed << setprecision(4) << rec.adamic_adar_score << endl;
        }
    }

    cout << "\n[4/4] Finding influential friend candidates (HYBRID)..." << endl;
    auto influential_friends = HybridAnalysis::find_influential_friend_candidates(g, sample_user, 5);
    
    if (influential_friends.empty()) {
        cout << "No influential friend candidates found." << endl;
    } else {
        cout << "Top 5 influential friend recommendations:" << endl;
        for (size_t i = 0; i < influential_friends.size(); ++i) {
            cout << "  " << (i+1) << ". User " << influential_friends[i].first
                 << " (hybrid score: " << fixed << setprecision(4)
                 << influential_friends[i].second << ")" << endl;
        }
    }

    print_header("DEMO COMPLETE");
    cout << "Summary:" << endl;
    cout << "  • BC calculation time: " << bc_time << " ms" << endl;
    cout << "  • Influence spread: " << spread << " / " << adj.size()
         << " nodes (" << fixed << setprecision(1)
         << (100.0 * spread / adj.size()) << "%)" << endl;
    cout << "  • Recommendations generated for sample user" << endl;
    cout << "  • Hybrid analysis combining both approaches" << endl;
}

int main() {
    Graph my_network;
    const string filename = "0.edges";
    
    print_header("INTEGRATED SOCIAL NETWORK SYSTEM");
    cout << "Loading network data from " << filename << "..." << endl;
    load_graph_from_file(my_network, filename);
    
    const auto& adj = my_network.get_adj_list();
    if (adj.empty()) {
        cerr << "Error: Graph is empty!" << endl;
        return 1;
    }
    
    cout << "System ready! Network has " << adj.size() << " users." << endl;
    
    int choice;
    do {
        show_menu();
        cin >> choice;
        
        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Invalid input!" << endl;
            continue;
        }
        
        switch (choice) {
            case 1: {
                print_header("BETWEENNESS CENTRALITY ANALYSIS");
                int k;
                cout << "Enter number of seeds (K): ";
                cin >> k;
                
                cout << "\nCalculating betweenness centrality..." << endl;
                auto start = chrono::high_resolution_clock::now();
                auto seeds = BetweennessCentrality::get_top_k_nodes(my_network, k);
                auto end = chrono::high_resolution_clock::now();
                auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
                
                cout << "\nTop " << k << " influential nodes:" << endl;
                auto bc_scores = BetweennessCentrality::calculate(my_network);
                for (size_t i = 0; i < seeds.size(); ++i) {
                    cout << "  " << (i+1) << ". Node " << seeds[i]
                         << " (BC score: " << fixed << setprecision(2)
                         << bc_scores[seeds[i]] << ")" << endl;
                }
                cout << "\nTime: " << duration.count() << " ms" << endl;
                break;
            }
            
            case 2: {
                print_header("INFLUENCE SPREAD SIMULATION (ICM)");
                cout << "Enter seed nodes (space-separated, -1 to end): ";
                set<NodeID> seeds;
                NodeID seed;
                while (cin >> seed && seed != -1) {
                    if (adj.count(seed)) {
                        seeds.insert(seed);
                    } else {
                        cout << "Warning: Node " << seed << " not in graph" << endl;
                    }
                }
                cin.clear();
                
                if (seeds.empty()) {
                    cout << "No valid seeds provided!" << endl;
                    break;
                }
                
                int num_sims;
                cout << "Number of simulations (default 1000): ";
                cin >> num_sims;
                if (cin.fail()) {
                    num_sims = 1000;
                    cin.clear();
                    cin.ignore(10000, '\n');
                }
                
                cout << "\nRunning ICM simulation..." << endl;
                auto start = chrono::high_resolution_clock::now();
                int spread = InfluenceMaximization::simulate_ICM(my_network, seeds, num_sims);
                auto end = chrono::high_resolution_clock::now();
                auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
                
                cout << "\nResults:" << endl;
                cout << "  Seeds: " << seeds.size() << endl;
                cout << "  Average Influence Spread: " << spread << " nodes" << endl;
                cout << "  Coverage: " << fixed << setprecision(2)
                     << (100.0 * spread / adj.size()) << "%" << endl;
                cout << "  Time: " << duration.count() << " ms" << endl;
                break;
            }
            
            case 3: {
                print_header("COMPARING SEED SELECTION STRATEGIES");
                cout << "This will take some time..." << endl;
                cout << "Enter K (number of seeds, recommend K<=3 for speed): ";
                int k;
                cin >> k;
                
                cout << "\n[1/2] Betweenness Centrality method..." << endl;
                auto bc_seeds = BetweennessCentrality::get_top_k_nodes(my_network, k);
                set<NodeID> bc_set(bc_seeds.begin(), bc_seeds.end());
                int bc_spread = InfluenceMaximization::simulate_ICM(my_network, bc_set, 500);
                
                cout << "BC Seeds: ";
                for (NodeID s : bc_seeds) cout << s << " ";
                cout << "\nBC Spread: " << bc_spread << " nodes" << endl;
                
                cout << "\n[2/2] Greedy method (this may take a while)..." << endl;
                auto greedy_set = InfluenceMaximization::greedy_seed_selection(my_network, k, 50);
                int greedy_spread = InfluenceMaximization::simulate_ICM(my_network, greedy_set, 500);
                
                cout << "Greedy Spread: " << greedy_spread << " nodes" << endl;
                cout << "\n--- Comparison ---" << endl;
                cout << "BC Method: " << bc_spread << " nodes" << endl;
                cout << "Greedy Method: " << greedy_spread << " nodes" << endl;
                cout << "Winner: " << (greedy_spread > bc_spread ? "Greedy" : "BC") << endl;
                break;
            }
            
            case 4: {
                print_header("FRIEND RECOMMENDATIONS");
                NodeID user;
                cout << "Enter User ID: ";
                cin >> user;
                
                if (!adj.count(user)) {
                    cout << "User not found!" << endl;
                    break;
                }
                
                int num_recs;
                cout << "Number of recommendations (default 10): ";
                cin >> num_recs;
                if (cin.fail()) {
                    num_recs = 10;
                    cin.clear();
                    cin.ignore(10000, '\n');
                }
                
                auto recs = FriendRecommendation::get_recommendations(my_network, user, num_recs);
                cout << "\nUser " << user << " has " << adj.at(user).size() << " friends" << endl;
                cout << "\nTop " << num_recs << " Recommendations:" << endl;
                
                if (recs.empty()) {
                    cout << "No recommendations available." << endl;
                } else {
                    cout << left << setw(8) << "Rank" << setw(12) << "User ID"
                         << setw(10) << "Common" << setw(12) << "Jaccard"
                         << setw(12) << "Adamic-Adar" << setw(12) << "Influence%" << endl;
                    cout << string(70, '-') << endl;
                    
                    for (size_t i = 0; i < recs.size(); ++i) {
                        cout << left << setw(8) << (i+1)
                             << setw(12) << recs[i].candidate_id
                             << setw(10) << recs[i].common_neighbors_count
                             << setw(12) << fixed << setprecision(4) << recs[i].jaccard_score
                             << setw(12) << fixed << setprecision(4) << recs[i].adamic_adar_score
                             << setw(12) << fixed << setprecision(1)
                             << (recs[i].influence_potential * 100) << "%" << endl;
                    }
                }
                break;
            }
            
            case 5: {
                print_header("INFLUENTIAL FRIEND CANDIDATES (HYBRID)");
                NodeID user;
                cout << "Enter User ID: ";
                cin >> user;
                
                if (!adj.count(user)) {
                    cout << "User not found!" << endl;
                    break;
                }
                
                cout << "\nFinding influential users who would be good friends..." << endl;
                auto influential = HybridAnalysis::find_influential_friend_candidates(my_network, user, 10);
                
                if (influential.empty()) {
                    cout << "No candidates found." << endl;
                } else {
                    cout << "\nThese users are both similar to you AND influential in the network:" << endl;
                    for (size_t i = 0; i < influential.size(); ++i) {
                        cout << "  " << (i+1) << ". User " << influential[i].first
                             << " (hybrid score: " << fixed << setprecision(4)
                             << influential[i].second << ")" << endl;
                    }
                }
                break;
            }
            
            case 6: {
                print_header("RECOMMENDATION IMPACT ANALYSIS");
                NodeID user;
                cout << "Enter User ID: ";
                cin >> user;
                
                if (!adj.count(user)) {
                    cout << "User not found!" << endl;
                    break;
                }
                
                cout << "Enter seed nodes for influence spread (space-separated, -1 to end): ";
                set<NodeID> seeds;
                NodeID seed;
                while (cin >> seed && seed != -1) {
                    if (adj.count(seed)) seeds.insert(seed);
                }
                cin.clear();
                
                if (seeds.empty()) {
                    auto bc_seeds = BetweennessCentrality::get_top_k_nodes(my_network, 3);
                    seeds = set<NodeID>(bc_seeds.begin(), bc_seeds.end());
                    cout << "Using default BC seeds: ";
                    for (NodeID s : seeds) cout << s << " ";
                    cout << endl;
                }
                
                HybridAnalysis::analyze_recommendation_impact(my_network, user, seeds, 500);
                break;
            }
            
            case 7: {
                show_graph_stats(my_network);
                break;
            }
            
            case 8: {
                run_complete_demo(my_network);
                break;
            }
            
            case 0:
                cout << "\nThank you for using the Integrated Social Network System!" << endl;
                break;
                
            default:
                cout << "Invalid choice!" << endl;
        }
        
    } while (choice != 0);
    
    return 0;
}
