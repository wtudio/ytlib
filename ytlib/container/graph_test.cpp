#include <gtest/gtest.h>

#include "graph.hpp"

namespace ytlib {

// todo 未完成
TEST(GRAPH_TEST, BASE_test) {
  typedef Graph<uint32_t> myGraph;

  std::vector<myGraph*> myGraphVec;
  uint32_t num = 10;
  for (uint32_t ii = 0; ii < num; ++ii) {
    myGraphVec.push_back(new myGraph(ii));
  }
  connectGraphNode<uint32_t>(*myGraphVec[0], *myGraphVec[1], 1);
  connectGraphNode<uint32_t>(*myGraphVec[0], *myGraphVec[5], 2);
  connectGraphNode<uint32_t>(*myGraphVec[0], *myGraphVec[9], 3);
  connectGraphNode<uint32_t>(*myGraphVec[1], *myGraphVec[2], 4);
  connectGraphNode<uint32_t>(*myGraphVec[2], *myGraphVec[8], 5);
  connectGraphNode<uint32_t>(*myGraphVec[3], *myGraphVec[4], 6);
  connectGraphNode<uint32_t>(*myGraphVec[3], *myGraphVec[7], 7);
  connectGraphNode<uint32_t>(*myGraphVec[4], *myGraphVec[5], 8);
  connectGraphNode<uint32_t>(*myGraphVec[6], *myGraphVec[7], 9);
  connectGraphNode<uint32_t>(*myGraphVec[6], *myGraphVec[8], 10);
  connectGraphNode<uint32_t>(*myGraphVec[8], *myGraphVec[9], 11);

  ASSERT_TRUE(isUndirGraph(myGraphVec));

  std::vector<myGraph*> myGraphVec2 = copyGraph(myGraphVec);
  ASSERT_EQ(myGraphVec.size(), myGraphVec2.size());
  for (size_t ii = 0; ii < myGraphVec.size(); ++ii) {
    ASSERT_NE(myGraphVec[ii], myGraphVec2[ii]);
    ASSERT_EQ(myGraphVec[ii]->obj, myGraphVec2[ii]->obj);
  }

  releaseGraphVec(myGraphVec2);

  // dfs
  std::vector<myGraph*> vec;
  clearFlag(myGraphVec);
  DFS(*myGraphVec[0], vec);

  // bfs
  vec.clear();
  clearFlag(myGraphVec);
  BFS(*myGraphVec[0], vec);

  //邻接矩阵
  g_sideMatrix M = createAdjMatrix<uint32_t>(myGraphVec);

  std::cout << M << std::endl;
  std::cout << std::endl;

  // dijkstra
  auto dj = dijkstra(*myGraphVec[0], myGraphVec);
  for (size_t ii = 0; ii < dj.first.size(); ++ii) {
    std::cout << dj.first[ii] << " ";
  }
  std::cout << std::endl;
  for (size_t ii = 0; ii < dj.first.size(); ++ii) {
    std::cout << dj.second[ii] << " ";
  }
  std::cout << std::endl;

  std::vector<int32_t> djpath = dijkstraPath(7, dj.second);
  for (size_t ii = 0; ii < djpath.size(); ++ii) {
    std::cout << myGraphVec[djpath[ii]]->obj << "-";
  }
  std::cout << std::endl;

  // floyd
  auto fl = floyd(myGraphVec);
  std::cout << fl.first << std::endl;
  std::cout << std::endl;
  std::cout << fl.second << std::endl;
  std::cout << std::endl
            << std::endl;

  std::vector<int32_t> flpath = floydPath(0, 7, fl.second);
  for (size_t ii = 0; ii < flpath.size(); ++ii) {
    std::cout << myGraphVec[flpath[ii]]->obj << "-";
  }
  std::cout << std::endl;

  releaseGraphVec(myGraphVec);
}

}  // namespace ytlib
