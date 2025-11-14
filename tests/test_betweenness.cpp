#include <gtest/gtest.h>
#include "data_loader.h"
#include "integrated_social_network.h"

TEST(BetweennessTest, TriangleGraphEquality) {
    Graph g;
    g.add_edge(1,2,0.5);
    g.add_edge(2,3,0.5);
    g.add_edge(1,3,0.5);

    auto bc = BetweennessCentrality::compute_betweenness_centrality(g);

    EXPECT_NEAR(bc[1], bc[2], 1e-6);
    EXPECT_NEAR(bc[2], bc[3], 1e-6);
}
