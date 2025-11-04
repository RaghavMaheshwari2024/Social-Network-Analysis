#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>

// Define the Node ID type
using NodeID = int;

// The graph is represented as an Adjacency List:
// Map: key is a NodeID, value is a vector of its neighbors (edges)
struct InfluenceEdge {
    NodeID target_node;
    double probability; // Probability of successful influence along this edge
};

using AdjacencyList = std::unordered_map<NodeID, std::vector<InfluenceEdge>>;

class Graph {
private:
    AdjacencyList adj;

public:
    // Function to add an edge (undirected)
    void add_edge(NodeID u, NodeID v,double p){
        adj[u].push_back({v,p});
        adj[v].push_back({u,p}); // Social network connections are usually two-way
    }

    // Function to get the neighbors of a node
    const std::vector<InfluenceEdge>&get_neighbors(NodeID u) const {
        // Return an empty vector if the node doesn't exist to prevent crash
        static const std::vector<InfluenceEdge> empty_vec; 
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