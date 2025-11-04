#include "data_loader.h" // Include your header file
#include <fstream>
#include <sstream>
#include <algorithm>

// --- Data Loading Function ---
void load_graph_from_file(Graph& g, const std::string& filename) {
    const double DEFAULT_PROBABILITY = 0.01;
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
            g.add_edge(u, v, DEFAULT_PROBABILITY);
        }
    }
    std::cout << "Graph loaded successfully from " << filename << std::endl;
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
        // Optionally print a few neighbors
        // std::cout << "First three neighbors: ";
        // for (size_t i = 0; i < std::min((size_t)3, neighbors.size()); ++i) {
        //     std::cout << neighbors[i] << " ";
        // }
        // std::cout << std::endl;
    } 
    else{
        std::cout << "Node 0 not found in the graph." << std::endl;
    }
    
    return 0;
}