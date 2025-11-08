#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include<stack>
#include<queue>
using namespace std;

// Define the Node ID type
using NodeID = int;

// The graph is represented as an Adjacency List:
// Map: key is a NodeID, value is a vector of its neighbors (edges)
using AdjacencyList = std::unordered_map<NodeID, std::vector<NodeID>>;

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

        for(const pair<NodeID, vector<NodeID>> &i: adj){
            if(i.first != src){
                result.dist[i.first] = 1e9;
                result.sigma[i.first] = 0;
            }
            result.P[i.first] = {};   //initializing empty predecessor vector!!
        }

        while(!q.empty()){
            NodeID currNode = q.front(); q.pop();
            result.S.push(currNode);
            for(NodeID i: adj[currNode]){
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
        for(const pair<NodeID, vector<NodeID>> &i: adj){
            centrality_score[i.first] = 0.0;
        }
        for(const pair<NodeID, vector<NodeID>> &i: adj){
            BrandesPhase1Result result = Brandes_Phase_1_BFS(i.first);
            //delta stores the centrality score of each node for every seperate source-node run
            unordered_map<int, double> delta;
            for(const pair<NodeID, vector<NodeID>> &j: adj){
                delta[j.first] = 0.0;
            }

            while(!result.S.empty()){
                int w = result.S.top(); result.S.pop();
                for(int v: result.P[w]){
                    delta[v] += ((double)result.sigma[v]/result.sigma[w]) * (1.0 + delta[w]);
                    
                }
                //source node does not get betweenness credit for paths starting at itself
                if(i.first != w){
                    centrality_score[w] += delta[w];
                }
            }
        }

        //normalizing scores: since A->v->B and B->v->A calculates score twice
        for(const pair<NodeID, vector<NodeID>> &i: adj){
            centrality_score[i.first] /= 2.0;
        }
        return centrality_score;
    }

    // Function to get all nodes
    const AdjacencyList& get_adj_list() const {
        return adj;
    }
};