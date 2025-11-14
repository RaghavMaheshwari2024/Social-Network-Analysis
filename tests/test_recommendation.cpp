#include <gtest/gtest.h>
#include "data_loader.h"
#include "integrated_social_network.h"

TEST(RecommendationTest, JaccardBasic) {
    Graph g;
    g.add_edge(1,2,0.5);
    g.add_edge(1,3,0.5);
    g.add_edge(4,2,0.5);
    g.add_edge(4,3,0.5);

    double j = FriendRecommendation::jaccard_coefficient(g, 1, 4);
    EXPECT_DOUBLE_EQ(j, 1.0);
}
