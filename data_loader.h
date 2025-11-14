#ifndef DATA_LOADER_H
#define DATA_LOADER_H

#include <vector>
#include <map>

using NodeID = int;

struct InfluenceEdge {
    NodeID target;
    double probability;
};

class Graph {
private:
    std::map<NodeID, std::vector<InfluenceEdge>> adj;

public:
    void add_edge(NodeID u, NodeID v, double probability) {
        adj[u].push_back({v, probability});
        adj[v].push_back({u, probability});
    }

    const std::map<NodeID, std::vector<InfluenceEdge>>& get_adj_list() const {
        return adj;
    }

    const std::vector<InfluenceEdge>& get_neighbors(NodeID node) const {
        auto it = adj.find(node);
        if (it != adj.end()) {
            return it->second;
        }
        static const std::vector<InfluenceEdge> empty_vec;
        return empty_vec;
    }
};

#endif
