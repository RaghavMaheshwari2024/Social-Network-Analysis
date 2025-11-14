#include <gtest/gtest.h>
#include "data_loader.h"

TEST(GraphTest, AddEdgeAndNeighbors) {
    Graph g;
    g.add_edge(1, 2, 0.5);
    g.add_edge(2, 3, 0.3);

    auto n1 = g.get_neighbors(1);
    ASSERT_EQ(n1.size(), 1);
    EXPECT_EQ(n1[0].target, 2);

    auto n2 = g.get_neighbors(2);
    EXPECT_EQ(n2.size(), 2);

    auto n4 = g.get_neighbors(999);
    EXPECT_EQ(n4.size(), 0);
}
