#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "binary_tree.hpp"
#include "graph.hpp"
#include "heap.hpp"
#include "ring_buf.hpp"

namespace ytlib {

using std::cout;
using std::endl;
using std::vector;

class TestClass {
 public:
  TestClass() {}
  TestClass(uint32_t id) : id_(id) {}

  uint32_t id_ = 0;
};

TEST(RSTUDIO_CONTAINER_TEST, RingBuf_TEST) {
  const uint32_t kBufSize = 10;
  using RingBufTest = RingBuf<TestClass, kBufSize>;
  RingBufTest ring;

  ASSERT_EQ(ring.Empty(), true);
  ASSERT_EQ(ring.Full(), false);
  ASSERT_EQ(ring.Capacity(), kBufSize - 1);
  ASSERT_EQ(ring.Size(), 0);
  ASSERT_EQ(ring.UnusedCapacity(), kBufSize - 1);

  TestClass obj1(1);
  TestClass obj2(2);

  ASSERT_EQ(ring.Push(obj1), true);
  ASSERT_EQ(ring.Push(std::move(obj2)), true);

  ASSERT_EQ(ring.Empty(), false);
  ASSERT_EQ(ring.Full(), false);
  ASSERT_EQ(ring.Size(), 2);
  ASSERT_EQ(ring.UnusedCapacity(), kBufSize - 3);

  ASSERT_EQ(ring.Top().id_, 1);
  ASSERT_EQ(ring.Get(0).id_, 1);
  ASSERT_EQ(ring.Get(1).id_, 2);

  for (uint32_t ii = 3; ii < kBufSize; ++ii) {
    ASSERT_EQ(ring.Push(TestClass(ii)), true);
  }

  ASSERT_EQ(ring.Empty(), false);
  ASSERT_EQ(ring.Full(), true);
  ASSERT_EQ(ring.Size(), kBufSize - 1);
  ASSERT_EQ(ring.UnusedCapacity(), 0);

  ASSERT_EQ(ring.Push(TestClass(kBufSize)), false);

  ASSERT_EQ(ring.Pop(), true);

  ASSERT_EQ(ring.Empty(), false);
  ASSERT_EQ(ring.Full(), false);
  ASSERT_EQ(ring.Size(), kBufSize - 2);
  ASSERT_EQ(ring.UnusedCapacity(), 1);

  ring.Clear();
  ASSERT_EQ(ring.Pop(), false);

  ASSERT_EQ(ring.Empty(), true);
  ASSERT_EQ(ring.Full(), false);
  ASSERT_EQ(ring.Size(), 0);
  ASSERT_EQ(ring.UnusedCapacity(), kBufSize - 1);
}

TEST(RSTUDIO_CONTAINER_TEST, RingBuf_char_TEST) {
  const uint32_t kBufSize = 15;
  using RingCharBufTest = RingBuf<char, kBufSize>;
  RingCharBufTest ring;

  std::string s = "0123456789";
  uint32_t s_size = static_cast<uint32_t>(s.size());
  ASSERT_EQ(ring.PushArray(s.c_str(), s_size), true);

  ASSERT_EQ(ring.Size(), s_size);
  ASSERT_EQ(ring.UnusedCapacity(), kBufSize - s_size - 1);

  ASSERT_EQ(ring.PushArray(s.c_str(), s_size), false);

  char* ps = nullptr;
  ASSERT_EQ(ring.TopArray(ps, s_size), true);

  ASSERT_STREQ(std::string(ps, s_size).c_str(), s.c_str());

  uint32_t get_pos = 5;
  ASSERT_EQ(ring.GetArray(get_pos, ps, s_size - get_pos), true);

  ASSERT_STREQ(std::string(ps, s_size - get_pos).c_str(), s.substr(get_pos).c_str());

  ASSERT_EQ(ring.PopArray(s_size), true);
  ASSERT_EQ(ring.PopArray(s_size), false);

  ASSERT_EQ(ring.Size(), 0);
  ASSERT_EQ(ring.UnusedCapacity(), kBufSize - 1);

  // -----------------------

  ASSERT_EQ(ring.PushArray(s.c_str(), s_size), true);

  ASSERT_EQ(ring.Size(), s_size);
  ASSERT_EQ(ring.UnusedCapacity(), kBufSize - s_size - 1);

  ASSERT_EQ(ring.PushArray(s.c_str(), s_size), false);

  ps = nullptr;
  ASSERT_EQ(ring.TopArray(ps, s_size), false);

  char ps2[kBufSize];
  ASSERT_EQ(ring.TopArray(ps = ps2, s_size), true);

  ASSERT_STREQ(std::string(ps, s_size).c_str(), s.c_str());

  ASSERT_EQ(ring.PopArray(s_size), true);
  ASSERT_EQ(ring.PopArray(s_size), false);

  ASSERT_EQ(ring.Size(), 0);
  ASSERT_EQ(ring.UnusedCapacity(), kBufSize - 1);
}

TEST(RSTUDIO_CONTAINER_TEST, BinTreeNode_TEST) {
  //BinTreeNode及基本节点操作
  typedef BinTreeNode<int> mybt;
  typedef std::shared_ptr<mybt> mybtPtr;
  mybtPtr rootp = std::make_shared<mybt>(0);  //0
  setLChild(rootp.get(), std::make_shared<mybt>(1));
  setRChild(rootp.get(), std::make_shared<mybt>(2));
  ASSERT_EQ(rootp->obj, 0);
  ASSERT_EQ(rootp->pl->obj, 1);
  ASSERT_EQ(rootp->pr->obj, 2);
  ASSERT_EQ(rootp->pf, nullptr);

  mybtPtr p = rootp->pl;  //1
  setLChild(p.get(), std::make_shared<mybt>(3));
  setRChild(p.get(), std::make_shared<mybt>(4));
  ASSERT_EQ(p->obj, 1);
  ASSERT_EQ(p->pl->obj, 3);
  ASSERT_EQ(p->pr->obj, 4);
  ASSERT_EQ(p->pf, rootp.get());
  ASSERT_TRUE(checkBinTree(rootp));

  //getHeight / getDepth / getNodeNum
  p = p->pr;  //4
  setRChild(p.get(), std::make_shared<mybt>(5));
  ASSERT_EQ(getHeight(rootp.get()), 4);
  ASSERT_EQ(getDepth(p.get()), 2);
  ASSERT_EQ(getNodeNum(rootp.get()), 6);

  p = rootp->pr;  //2
  setRChild(p.get(), std::make_shared<mybt>(6));
  p = p->pr;  //6
  setRChild(p.get(), std::make_shared<mybt>(7));
  p = p->pr;  //7
  setRChild(p.get(), std::make_shared<mybt>(8));
  ASSERT_EQ(getHeight(rootp.get()), 5);
  ASSERT_EQ(getDepth(p.get()), 3);
  ASSERT_EQ(getNodeNum(rootp.get()), 9);
  ASSERT_TRUE(checkBinTree(rootp));

  //前序遍历
  vector<mybtPtr> vec;
  vector<int> answerDLR = {0, 1, 3, 4, 5, 2, 6, 7, 8};
  DLR(rootp, vec);
  ASSERT_TRUE(checkBinTree(rootp));
  ASSERT_EQ(vec.size(), answerDLR.size());
  for (size_t ii = 0; ii < vec.size(); ++ii) {
    ASSERT_EQ(vec[ii]->obj, answerDLR[ii]);
  }

  //中序遍历
  vec.clear();
  vector<int> answerLDR = {3, 1, 4, 5, 0, 2, 6, 7, 8};
  LDR(rootp, vec);
  ASSERT_TRUE(checkBinTree(rootp));
  ASSERT_EQ(vec.size(), answerLDR.size());
  for (size_t ii = 0; ii < vec.size(); ++ii) {
    ASSERT_EQ(vec[ii]->obj, answerLDR[ii]);
  }

  //后续遍历
  vec.clear();
  vector<int> answerLRD = {3, 5, 4, 1, 8, 7, 6, 2, 0};
  LRD(rootp, vec);
  ASSERT_TRUE(checkBinTree(rootp));
  ASSERT_EQ(vec.size(), answerLRD.size());
  for (size_t ii = 0; ii < vec.size(); ++ii) {
    ASSERT_EQ(vec[ii]->obj, answerLRD[ii]);
  }

  //分层遍历
  vec.clear();
  vector<int> answerTbl = {0, 1, 2, 3, 4, 6, 5, 7, 8};
  traByLevel(rootp, vec);
  ASSERT_TRUE(checkBinTree(rootp));
  ASSERT_EQ(vec.size(), answerTbl.size());
  for (size_t ii = 0; ii < vec.size(); ++ii) {
    ASSERT_EQ(vec[ii]->obj, answerTbl[ii]);
  }

  //二叉树序列化 / 反序列化
  vector<std::pair<bool, int> > seriVec;
  SerializeTree(rootp, seriVec);
  ASSERT_TRUE(checkBinTree(rootp));
  mybtPtr rootp2;
  DeserializeTree(rootp2, seriVec);
  ASSERT_TRUE(checkBinTree(rootp2));
  vec.clear();
  traByLevel(rootp2, vec);
  ASSERT_EQ(vec.size(), answerTbl.size());
  for (size_t ii = 0; ii < vec.size(); ++ii) {
    ASSERT_EQ(vec[ii]->obj, answerTbl[ii]);
  }

  //二叉树深拷贝
  mybtPtr rootp3 = copyTree(rootp);
  ASSERT_TRUE(checkBinTree(rootp));
  ASSERT_TRUE(checkBinTree(rootp3));
  vector<mybtPtr> vecCkCpy;
  traByLevel(rootp3, vecCkCpy);
  ASSERT_EQ(vecCkCpy.size(), answerTbl.size());
  for (size_t ii = 0; ii < vec.size(); ++ii) {
    ASSERT_EQ(vecCkCpy[ii]->obj, answerTbl[ii]);
  }
  vec.clear();
  traByLevel(rootp, vec);
  ASSERT_EQ(vecCkCpy.size(), answerTbl.size());
  for (size_t ii = 0; ii < vec.size(); ++ii) {
    ASSERT_EQ(vecCkCpy[ii]->obj, answerTbl[ii]);
  }

  //复杂节点操作（左）
  p = rootp->pl;
  ASSERT_EQ(getLR(p.get()), true);
  setLChild(rootp.get(), std::make_shared<mybt>(-1));
  ASSERT_TRUE(checkBinTree(rootp));
  ASSERT_EQ(rootp->pl->obj, -1);
  ASSERT_EQ(p->pf, nullptr);

  setLChild(rootp.get(), p);
  ASSERT_TRUE(checkBinTree(rootp));
  ASSERT_EQ(rootp->pl->obj, 1);
  ASSERT_EQ(p->pf, rootp.get());

  breakLChild(rootp.get());
  ASSERT_TRUE(checkBinTree(rootp));
  ASSERT_EQ(p->pf, nullptr);
  ASSERT_FALSE(rootp->pl);

  //复杂节点操作（右）
  p = rootp->pr;
  ASSERT_EQ(getLR(p.get()), false);
  setRChild(rootp.get(), std::make_shared<mybt>(-1));
  ASSERT_TRUE(checkBinTree(rootp));
  ASSERT_EQ(rootp->pr->obj, -1);
  ASSERT_EQ(p->pf, nullptr);

  setRChild(rootp.get(), p);
  ASSERT_TRUE(checkBinTree(rootp));
  ASSERT_EQ(rootp->pr->obj, 2);
  ASSERT_EQ(p->pf, rootp.get());

  breakRChild(rootp.get());
  ASSERT_TRUE(checkBinTree(rootp));
  ASSERT_EQ(p->pf, nullptr);
  ASSERT_FALSE(rootp->pr);
}

TEST(RSTUDIO_CONTAINER_TEST, BinSearchTreeNode_TEST) {
  //二叉查找树BinSearchTreeNode
  typedef BinSearchTreeNode<int> mybst;
  typedef std::shared_ptr<mybst> mybstPtr;
  mybstPtr rootp = std::make_shared<mybst>(10);
  for (int ii = 0; ii < 20; ii += 2) {
    rootp->insert(std::make_shared<mybst>(ii));
  }
  ASSERT_TRUE(checkBinSearchTree(rootp));

  vector<mybstPtr> vec;
  vector<int> answerDLR = {10, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18};
  DLR(rootp, vec);
  ASSERT_TRUE(checkBinSearchTree(rootp));
  ASSERT_EQ(vec.size(), answerDLR.size());
  for (size_t ii = 0; ii < vec.size(); ++ii) {
    ASSERT_EQ(vec[ii]->obj, answerDLR[ii]);
  }

  vec.clear();
  vector<int> answerTbl = {10, 0, 10, 2, 12, 4, 14, 6, 16, 8, 18};
  traByLevel(rootp, vec);
  ASSERT_TRUE(checkBinSearchTree(rootp));
  ASSERT_EQ(vec.size(), answerTbl.size());
  for (size_t ii = 0; ii < vec.size(); ++ii) {
    ASSERT_EQ(vec[ii]->obj, answerTbl[ii]);
  }

  //二叉搜索树中进行查找binSearch
  mybstPtr bsRe = binSearch(rootp, 10);
  ASSERT_EQ(bsRe.get(), rootp.get());
  bsRe = binSearch(rootp, 0);
  ASSERT_EQ(bsRe.get(), rootp->pl.get());
  bsRe = binSearch(rootp, 12);
  ASSERT_EQ(bsRe.get(), rootp->pr->pr.get());
  bsRe = binSearch(rootp, 9);
  ASSERT_FALSE(bsRe);
}
TEST(RSTUDIO_CONTAINER_TEST, AVLTreeNode_TEST) {
  //AVL树
  typedef AVLTreeNode<int> myavlt;
  typedef std::shared_ptr<myavlt> myavltPtr;
  myavltPtr rootp = std::make_shared<myavlt>(10);

  //插入
  for (int ii = 0; ii < 20; ii += 2) {
    rootp = rootp->insert(std::make_shared<myavlt>(ii));
  }
  ASSERT_TRUE(checkAVLTree(rootp));

  vector<myavltPtr> vec;
  vector<int> answerLDR = {0, 2, 4, 6, 8, 10, 12, 14, 16, 18};
  LDR(rootp, vec);
  ASSERT_TRUE(checkAVLTree(rootp));
  ASSERT_EQ(vec.size(), answerLDR.size());
  for (size_t ii = 0; ii < vec.size(); ++ii) {
    ASSERT_EQ(vec[ii]->obj, answerLDR[ii]);
  }

  vec.clear();
  vector<int> answerDLR = {6, 2, 0, 4, 14, 10, 8, 12, 16, 18};
  DLR(rootp, vec);
  ASSERT_TRUE(checkAVLTree(rootp));
  ASSERT_EQ(vec.size(), answerDLR.size());
  for (size_t ii = 0; ii < vec.size(); ++ii) {
    ASSERT_EQ(vec[ii]->obj, answerDLR[ii]);
  }

  vec.clear();
  vector<int> answerTbl = {6, 2, 14, 0, 4, 10, 16, 8, 12, 18};
  traByLevel(rootp, vec);
  ASSERT_TRUE(checkAVLTree(rootp));
  ASSERT_EQ(vec.size(), answerTbl.size());
  for (size_t ii = 0; ii < vec.size(); ++ii) {
    ASSERT_EQ(vec[ii]->obj, answerTbl[ii]);
  }

  //删除
  rootp = rootp->erase(100);
  ASSERT_TRUE(checkAVLTree(rootp));
  vec.clear();
  DLR(rootp, vec);
  ASSERT_EQ(vec.size(), answerDLR.size());
  for (size_t ii = 0; ii < vec.size(); ++ii) {
    ASSERT_EQ(vec[ii]->obj, answerDLR[ii]);
  }

  rootp = rootp->erase(14);
  ASSERT_TRUE(checkAVLTree(rootp));
  vector<int> answerDLR2 = {6, 2, 0, 4, 12, 10, 8, 16, 18};
  vec.clear();
  DLR(rootp, vec);
  ASSERT_EQ(vec.size(), answerDLR2.size());
  for (size_t ii = 0; ii < vec.size(); ++ii) {
    ASSERT_EQ(vec[ii]->obj, answerDLR2[ii]);
  }

  //删除根节点
  rootp = rootp->erase(rootp);
  ASSERT_TRUE(checkAVLTree(rootp));
  vector<int> answerDLR3 = {4, 2, 0, 12, 10, 8, 16, 18};
  vec.clear();
  DLR(rootp, vec);
  ASSERT_EQ(vec.size(), answerDLR3.size());
  for (size_t ii = 0; ii < vec.size(); ++ii) {
    ASSERT_EQ(vec[ii]->obj, answerDLR3[ii]);
  }

  rootp = rootp->erase(0);
  ASSERT_TRUE(checkAVLTree(rootp));
  vector<int> answerDLR4 = {12, 4, 2, 10, 8, 16, 18};
  vec.clear();
  DLR(rootp, vec);
  ASSERT_EQ(vec.size(), answerDLR4.size());
  for (size_t ii = 0; ii < vec.size(); ++ii) {
    ASSERT_EQ(vec[ii]->obj, answerDLR4[ii]);
  }

  rootp = rootp->erase(rootp->pr->pr);
  ASSERT_TRUE(checkAVLTree(rootp));
  vector<int> answerDLR5 = {10, 4, 2, 8, 12, 16};
  vec.clear();
  DLR(rootp, vec);
  ASSERT_EQ(vec.size(), answerDLR5.size());
  for (size_t ii = 0; ii < vec.size(); ++ii) {
    ASSERT_EQ(vec[ii]->obj, answerDLR5[ii]);
  }
}
TEST(RSTUDIO_CONTAINER_TEST, BRTreeNode_TEST) {
  //todo 未完成

  //红黑树
  typedef BRTreeNode<int> mybrt;
  typedef std::shared_ptr<mybrt> mybrtPtr;
  mybrtPtr rootp = std::make_shared<mybrt>(10);
  //插入

  for (int ii = 0; ii < 20; ii += 2) {
    rootp = rootp->insert(std::make_shared<mybrt>(ii));
  }
  ASSERT_TRUE(checkBRTree(rootp));

  //删除
}

TEST(RSTUDIO_CONTAINER_TEST, Graph_TEST) {
  //todo 未完成
  typedef Graph<uint32_t> myGraph;

  vector<myGraph*> myGraphVec;
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

  vector<myGraph*> myGraphVec2 = copyGraph(myGraphVec);
  ASSERT_EQ(myGraphVec.size(), myGraphVec2.size());
  for (size_t ii = 0; ii < myGraphVec.size(); ++ii) {
    ASSERT_NE(myGraphVec[ii], myGraphVec2[ii]);
    ASSERT_EQ(myGraphVec[ii]->obj, myGraphVec2[ii]->obj);
  }

  releaseGraphVec(myGraphVec2);

  //dfs
  vector<myGraph*> vec;
  clearFlag(myGraphVec);
  DFS(*myGraphVec[0], vec);

  //bfs
  vec.clear();
  clearFlag(myGraphVec);
  BFS(*myGraphVec[0], vec);

  //邻接矩阵
  g_sideMatrix M = createAdjMatrix<uint32_t>(myGraphVec);

  cout << M << endl;
  cout << endl;

  //dijkstra
  auto dj = dijkstra(*myGraphVec[0], myGraphVec);
  for (size_t ii = 0; ii < dj.first.size(); ++ii) {
    cout << dj.first[ii] << " ";
  }
  cout << endl;
  for (size_t ii = 0; ii < dj.first.size(); ++ii) {
    cout << dj.second[ii] << " ";
  }
  cout << endl;

  vector<int32_t> djpath = dijkstraPath(7, dj.second);
  for (size_t ii = 0; ii < djpath.size(); ++ii) {
    cout << myGraphVec[djpath[ii]]->obj << "-";
  }
  cout << endl;

  //floyd
  auto fl = floyd(myGraphVec);
  cout << fl.first << endl;
  cout << endl;
  cout << fl.second << endl;
  cout << endl
       << endl;

  vector<int32_t> flpath = floydPath(0, 7, fl.second);
  for (size_t ii = 0; ii < flpath.size(); ++ii) {
    cout << myGraphVec[flpath[ii]]->obj << "-";
  }
  cout << endl;

  releaseGraphVec(myGraphVec);
}

TEST(RSTUDIO_CONTAINER_TEST, Heap_TEST) {
  //todo 未完成
  int data[10] = {7, 9, 2, 6, 4, 8, 1, 3, 10, 5};
  Heap<int> h1(data, 10);
  for (int ii = 0; ii < 10; ++ii) {
    cout << h1.container[ii] << " ";
  }
  cout << endl;

  h1.sort();
  for (int ii = 0; ii < 10; ++ii) {
    cout << h1.container[ii] << " ";
  }
  cout << endl;

  int data2[15] = {7, -5, 9, -7, 2, 6, -6, 4, 8, 1, 3, -8, 10, 5, -9};
  h1.assign(data2, 15);
  h1.sort();

  for (int ii = 0; ii < 15; ++ii) {
    cout << h1.container[ii] << " ";
  }
  cout << endl;

  h1.push(-3);
  h1.push(-10);
  h1.push(11);
  for (int ii = 0; ii < 18; ++ii) {
    cout << h1.container[ii] << " ";
  }

  h1.pop();
  h1.pop();
  h1.pop();
  for (int ii = 0; ii < 15; ++ii) {
    cout << h1.container[ii] << " ";
  }
  cout << endl;
}

}  // namespace ytlib
