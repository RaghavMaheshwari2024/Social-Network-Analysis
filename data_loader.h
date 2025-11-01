#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>

// Define the Node ID type
using NodeID = int;

// The graph is represented as an Adjacency List:
// Map: key is a NodeID, value is a vector of its neighbors (edges)
using AdjacencyList = std::unordered_map<NodeID, std::vector<NodeID>>;

class Graph {
private:
    AdjacencyList adj;

public:
    // Function to add an edge (undirected)
    void add_edge(NodeID u, NodeID v) {
        adj[u].push_back(v);
        adj[v].push_back(u); // Social network connections are usually two-way
    }

    // Function to get the neighbors of a node
    const std::vector<NodeID>& get_neighbors(NodeID u) const {
        // Return an empty vector if the node doesn't exist to prevent crash
        static const std::vector<NodeID> empty_vec; 
        auto it = adj.find(u);
        if (it != adj.end()) {
            return it->second;
        }
        return empty_vec;
    }

    // Function to get all nodes
    const AdjacencyList& get_adj_list() const {
        return adj;
    }
};