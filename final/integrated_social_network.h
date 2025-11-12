#ifndef INTEGRATED_SOCIAL_NETWORK_H
#define INTEGRATED_SOCIAL_NETWORK_H

#include "data_loader.h"
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <queue>
#include <stack>
#include <random>
#include <algorithm>
#include <cmath>
#include <iostream>

using namespace std;

// UTILITY FUNCTIONS
int count_common_neighbors(const Graph& g, NodeID A, NodeID B) {
    set<NodeID> neighbors_A;
    for (const auto& edge : g.get_neighbors(A)) {
        neighbors_A.insert(edge.target);  // FIXED: was target_node
    }

    int common_count = 0;
    for (const auto& edge : g.get_neighbors(B)) {
        NodeID v = edge.target;  // FIXED: was target_node
        if (v == A || v == B) continue;
        if (neighbors_A.count(v)) {
            common_count++;
        }
    }
    return common_count;
}

double calculate_influence_probability(int common_neighbors) {
    const double SCALING_FACTOR = 0.1;
    return min(1.0, common_neighbors * SCALING_FACTOR);
}

// BETWEENNESS CENTRALITY
class BetweennessCentrality {
public:
    static unordered_map<NodeID, double> calculate(const Graph& g) {
        unordered_map<NodeID, double> BC;
        vector<NodeID> all_nodes;

        for (const auto& pair : g.get_adj_list()) {
            all_nodes.push_back(pair.first);
            BC[pair.first] = 0.0;
        }

        for (NodeID s : all_nodes) {
            stack<NodeID> S;
            queue<NodeID> Q;
            unordered_map<NodeID, vector<NodeID>> P;
            unordered_map<NodeID, int> sigma;
            unordered_map<NodeID, int> dist;

            sigma[s] = 1;
            dist[s] = 0;
            Q.push(s);

            while (!Q.empty()) {
                NodeID v = Q.front();
                Q.pop();
                S.push(v);
                if (P.find(v) == P.end()) P[v] = vector<NodeID>();

                for (const auto& edge : g.get_neighbors(v)) {
                    NodeID w = edge.target;  // FIXED: was target_node

                    if (dist.find(w) == dist.end()) {
                        dist[w] = dist[v] + 1;
                        sigma[w] = 0;
                        Q.push(w);
                    }

                    if (dist[w] == dist[v] + 1) {
                        sigma[w] += sigma[v];
                        if (P.find(w) == P.end()) P[w] = vector<NodeID>();
                        P[w].push_back(v);
                    }
                }
            }

            unordered_map<NodeID, double> delta;
            for (NodeID node : all_nodes) delta[node] = 0.0;

            while (!S.empty()) {
                NodeID w = S.top();
                S.pop();
                for (NodeID v : P[w]) {
                    double fraction = (double)sigma[v] / sigma[w];
                    delta[v] += fraction * (1.0 + delta[w]);
                }
                if (w != s) {
                    BC[w] += delta[w];
                }
            }
        }
        return BC;
    }

    static vector<NodeID> get_top_k_nodes(const Graph& g, int k) {
        auto bc_scores = calculate(g);
        vector<pair<double, NodeID>> sorted_nodes;

        for (const auto& pair : bc_scores) {
            sorted_nodes.push_back({pair.second, pair.first});
        }

        sort(sorted_nodes.begin(), sorted_nodes.end(),
             [](const pair<double, NodeID>& a, const pair<double, NodeID>& b) {
                 return a.first > b.first;
             });

        vector<NodeID> top_nodes;
        for (int i = 0; i < min((int)sorted_nodes.size(), k); ++i) {
            top_nodes.push_back(sorted_nodes[i].second);
        }
        return top_nodes;
    }
};

// INDEPENDENT CASCADE MODEL
class InfluenceMaximization {
public:
    static int simulate_ICM(const Graph& g, const set<NodeID>& seed_set,
                           int num_simulations = 1000) {
        long long total_spread = 0;
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<> dis(0.0, 1.0);

        for (int sim = 0; sim < num_simulations; ++sim) {
            set<NodeID> active = seed_set;
            queue<NodeID> q;
            for (NodeID node : seed_set) q.push(node);

            while (!q.empty()) {
                NodeID u = q.front();
                q.pop();

                for (const auto& edge : g.get_neighbors(u)) {
                    NodeID v = edge.target;  // FIXED: was target_node
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

    static set<NodeID> greedy_seed_selection(const Graph& g, int k,
                                             int simulations_per_eval = 100) {
        set<NodeID> seeds;
        const auto& adj = g.get_adj_list();
        vector<NodeID> candidates;

        for (const auto& pair : adj) {
            candidates.push_back(pair.first);
        }

        cout << "Starting greedy seed selection (k=" << k << ")..." << endl;
        for (int i = 0; i < k; ++i) {
            NodeID best_node = -1;
            int best_spread = 0;

            for (NodeID candidate : candidates) {
                if (seeds.count(candidate)) continue;
                set<NodeID> temp_seeds = seeds;
                temp_seeds.insert(candidate);
                int spread = simulate_ICM(g, temp_seeds, simulations_per_eval);
                if (spread > best_spread) {
                    best_spread = spread;
                    best_node = candidate;
                }
            }

            if (best_node != -1) {
                seeds.insert(best_node);
                cout << "  Seed " << (i+1) << ": Node " << best_node
                     << " (marginal spread: " << best_spread << ")" << endl;
            }
        }
        return seeds;
    }
};

// FRIEND RECOMMENDATION SYSTEM
struct RecommendationScore {
    NodeID candidate_id;
    int common_neighbors_count;
    double jaccard_score;
    double adamic_adar_score;
    double combined_score;
    double influence_potential;

    RecommendationScore() : candidate_id(-1), common_neighbors_count(0),
                           jaccard_score(0.0), adamic_adar_score(0.0),
                           combined_score(0.0), influence_potential(0.0) {}
};

class FriendRecommendation {
public:
    static double jaccard_coefficient(const Graph& g, NodeID u, NodeID v) {
        set<NodeID> neighbors_u, neighbors_v, union_set;

        for (const auto& edge : g.get_neighbors(u)) {
            neighbors_u.insert(edge.target);  // FIXED
            union_set.insert(edge.target);    // FIXED
        }

        for (const auto& edge : g.get_neighbors(v)) {
            neighbors_v.insert(edge.target);  // FIXED
            union_set.insert(edge.target);    // FIXED
        }

        int intersection = 0;
        for (NodeID node : neighbors_v) {
            if (neighbors_u.count(node)) intersection++;
        }

        if (union_set.empty()) return 0.0;
        return (double)intersection / union_set.size();
    }

    static double adamic_adar_index(const Graph& g, NodeID u, NodeID v) {
        set<NodeID> neighbors_u;
        for (const auto& edge : g.get_neighbors(u)) {
            neighbors_u.insert(edge.target);  // FIXED
        }

        double score = 0.0;
        for (const auto& edge : g.get_neighbors(v)) {
            NodeID neighbor = edge.target;  // FIXED
            if (neighbors_u.count(neighbor)) {
                int degree = g.get_neighbors(neighbor).size();
                if (degree > 1) {
                    score += 1.0 / log(degree);
                }
            }
        }
        return score;
    }

    static vector<RecommendationScore> get_recommendations(
        const Graph& g, NodeID user, int max_recs = 10) {
        
        set<NodeID> direct_friends;
        for (const auto& edge : g.get_neighbors(user)) {
            direct_friends.insert(edge.target);  // FIXED
        }
        direct_friends.insert(user);

        set<NodeID> candidates;
        for (const auto& edge : g.get_neighbors(user)) {
            NodeID friend_node = edge.target;  // FIXED
            for (const auto& edge2 : g.get_neighbors(friend_node)) {
                NodeID fof = edge2.target;  // FIXED
                if (!direct_friends.count(fof)) {
                    candidates.insert(fof);
                }
            }
        }

        vector<RecommendationScore> recommendations;
        for (NodeID candidate : candidates) {
            RecommendationScore score;
            score.candidate_id = candidate;
            score.common_neighbors_count = count_common_neighbors(g, user, candidate);
            score.jaccard_score = jaccard_coefficient(g, user, candidate);
            score.adamic_adar_score = adamic_adar_index(g, user, candidate);
            score.influence_potential = calculate_influence_probability(
                score.common_neighbors_count);
            score.combined_score = 0.5 * score.adamic_adar_score +
                                  0.3 * score.jaccard_score +
                                  0.2 * score.influence_potential;
            recommendations.push_back(score);
        }

        sort(recommendations.begin(), recommendations.end(),
             [](const RecommendationScore& a, const RecommendationScore& b) {
                 return a.combined_score > b.combined_score;
             });

        if (recommendations.size() > (size_t)max_recs) {
            recommendations.resize(max_recs);
        }
        return recommendations;
    }

    static vector<NodeID> recommend_friends_simple(const Graph& g, NodeID user, int n = 10) {
        auto recs = get_recommendations(g, user, n);
        vector<NodeID> result;
        for (const auto& rec : recs) {
            result.push_back(rec.candidate_id);
        }
        return result;
    }
};

// HYBRID ANALYSIS
class HybridAnalysis {
public:
    static vector<pair<NodeID, double>> find_influential_friend_candidates(
        const Graph& g, NodeID user, int top_k = 10) {
        
        auto bc_scores = BetweennessCentrality::calculate(g);
        auto recommendations = FriendRecommendation::get_recommendations(g, user, 50);

        vector<pair<NodeID, double>> scored_candidates;
        for (const auto& rec : recommendations) {
            double bc_score = bc_scores[rec.candidate_id];
            double hybrid_score = 0.7 * rec.combined_score + 0.3 * (bc_score / 100.0);
            scored_candidates.push_back({rec.candidate_id, hybrid_score});
        }

        sort(scored_candidates.begin(), scored_candidates.end(),
             [](const pair<NodeID, double>& a, const pair<NodeID, double>& b) {
                 return a.second > b.second;
             });

        if (scored_candidates.size() > (size_t)top_k) {
            scored_candidates.resize(top_k);
        }
        return scored_candidates;
    }

    static void analyze_recommendation_impact(const Graph& g, NodeID user,
                                             const set<NodeID>& initial_seeds,
                                             int num_simulations = 1000) {
        cout << "\n=== Analyzing Recommendation Impact on Influence Spread ===" << endl;
        
        int baseline_spread = InfluenceMaximization::simulate_ICM(g, initial_seeds, num_simulations);
        cout << "Baseline spread: " << baseline_spread << " nodes" << endl;

        auto recommendations = FriendRecommendation::recommend_friends_simple(g, user, 5);
        cout << "\nTop 5 recommended friends for User " << user << ":" << endl;
        for (size_t i = 0; i < recommendations.size(); ++i) {
            cout << "  " << (i+1) << ". Node " << recommendations[i] << endl;
        }

        cout << "\nInfluence potential of connecting with each recommendation:" << endl;
        for (NodeID candidate : recommendations) {
            int common = count_common_neighbors(g, user, candidate);
            double prob = calculate_influence_probability(common);
            cout << "  Node " << candidate << ": " << common
                 << " common neighbors → " << (prob * 100) << "% influence probability" << endl;
        }
    }
};

#endif
