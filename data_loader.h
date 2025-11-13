#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include<stack>
#include<queue>
using namespace std;

// Define the Node ID type
using NodeID = int;

// --- MODIFICATION 1: Using InfluenceEdge struct ---
// This is needed for compatibility with ICM functions.
struct InfluenceEdge {
    NodeID target_node;
    double probability; // Probability of successful influence along this edge
};

//MODIFICATION 2: using modified list to handle Influence edges matching Raghav's ICM code structure ------
// The graph is represented as an Adjacency List:
// Map: key is a NodeID, value is a vector of its neighbors and the value of successful probability influence(edges)
using AdjacencyList = std::unordered_map<NodeID, std::vector<InfluenceEdge>>;

/**
* @brief: Stores all results from the Brandes' algorithm Phase 1 (Forward Pass)
*
* @var: S: stores the path order through which we accessed all the nodes in the BFS Traversal
* @var: dist: stores node and it's distance from a selected source 
* @var: sigma: stores the node and the frequency of distinct shortest paths from source to the node itself
* @var: P: stores the predecessors for a node from source on all it's shortest paths
*
**/
struct BrandesPhase1Result{
    stack<int> S;
    unordered_map<int, int> dist;
    unordered_map<int, long long> sigma;
    unordered_map<int, vector<int>> P;
};

class Graph {
private:
    AdjacencyList adj;

public:
    // Function to add an edge (undirected)
    // --- MODIFICATION 3: Update add_edge to accept a probability 'p' ---
    void add_edge(NodeID u, NodeID v, double p) {
        adj[u].push_back({v, p});
        adj[v].push_back({u, p}); // Social network connections are usually two-way
    }

    // Function to get the neighbors of a node
    const std::vector<InfluenceEdge>& get_neighbors(NodeID u) const {
        // Return an empty vector if the node doesn't exist to prevent crash
        static const std::vector<InfluenceEdge> empty_vec; 
        auto it = adj.find(u);
        if (it != adj.end()) {
            return it->second;
        }
        return empty_vec;
    }
    /**
    *@brief: performs the Brandes_Phase_1 traversal for shortest path
    *
    *@param: src: the starting point(source) through which we assign credit scores and observe traversals
    *@var: q: stores the neighbor nodes which will be called later during BFS traversal
    *@return: returns the traversal results:
    *               1. number of shortest paths from source to each node
    *               2. the return stack order
    *               3. the predecessors for each node on all their shortest paths
    *               4. the shortest distance of each node from the source
    *
    **/
    BrandesPhase1Result Brandes_Phase_1_BFS(NodeID src) { // <-- Added 'all_nodes' for safety
        BrandesPhase1Result result;

        result.dist[src] = 0;
        result.sigma[src] = 1;
        queue<int> q;
        q.push(src);

        //making changes on applying InfluenceEdge
        for(const pair<NodeID, vector<InfluenceEdge>> &i: adj){
            if(i.first != src){
                result.dist[i.first] = 1e9;
                result.sigma[i.first] = 0;
            }
            result.P[i.first] = {};   //initializing empty predecessor vector!!
        }

        while(!q.empty()){
            NodeID currNode = q.front(); q.pop();
            result.S.push(currNode);

            // --- MODIFICATION 5: Adapt loop to use InfluenceEdge ---
            for(InfluenceEdge& edge: adj[currNode]){
                NodeID i = edge.target_node; // Get the neighbor ID

                if(result.dist[currNode] + 1 < result.dist[i]){
                    result.dist[i] = result.dist[currNode] + 1;
                    result.sigma[i] = result.sigma[currNode];
                    result.P[i].push_back(currNode);
                    q.push(i);
                }
                else if(result.dist[currNode] + 1 == result.dist[i]){
                    result.sigma[i] += result.sigma[currNode];
                    result.P[i].push_back(currNode);
                }
            }
        }
        return result;
    }

    /**
    *@brief: Uses the Brandes_Phase_1 traversal and computes the centrality scores using the Brandes_Phase_2 algorithm implementation
    *which performs the back traversal to assign the credit scores for each node recursively
    *@var: centrality_score: Stores the centrality scores of all nodes in the network
    *@var: delta: Stores centrality score of every node for every separate source to node traversal
    *
    **/
    unordered_map<int, double> compute_betweenness_centrality(){
        unordered_map<int, double> centrality_score;
        for(const pair<NodeID, vector<InfluenceEdge>> &i: adj){
            centrality_score[i.first] = 0.0;
        }
        for(const pair<NodeID, vector<InfluenceEdge>> &i: adj){
            BrandesPhase1Result result = Brandes_Phase_1_BFS(i.first);
            //delta stores the centrality score of each node for every seperate source-node run
            unordered_map<int, double> delta;
            for(const pair<NodeID, vector<InfluenceEdge>> &j: adj){
                delta[j.first] = 0.0;
            }

            while(!result.S.empty()){
                int w = result.S.top(); result.S.pop();
                for(int v: result.P[w]){
                    if(result.sigma[w] != 0){
                        delta[v] += ((double)result.sigma[v]/result.sigma[w]) * (1.0 + delta[w]);
                    }
                    
                }
                //source node does not get betweenness credit for paths starting at itself
                if(i.first != w){
                    centrality_score[w] += delta[w];
                }
            }
        }

        //normalizing scores: since A->v->B and B->v->A calculates score twice
        for(const pair<NodeID, vector<InfluenceEdge>> &i: adj){ //Adapted loop
            centrality_score[i.first] /= 2.0;
        }
        return centrality_score;
    }

    // Function to get all nodes
    const AdjacencyList& get_adj_list() const {
        return adj;
    }
};