#include "t_DataStructure.h"
#include <ytlib/Common/Util.h>
#include <boost/core/lightweight_test.hpp>
#include <iostream>
using namespace std;
namespace ytlib {
///测试二叉树
void test_BinaryTree() {
  YT_DEBUG_PRINTF("test BinTreeNode\n");
  {
    //BinTreeNode及基本节点操作
    typedef BinTreeNode<int> mybt;
    typedef shared_ptr<mybt> mybtPtr;
    mybtPtr rootp = make_shared<mybt>(0);  //0
    setLChild(rootp.get(), make_shared<mybt>(1));
    setRChild(rootp.get(), make_shared<mybt>(2));
    BOOST_TEST_EQ(rootp->obj, 0);
    BOOST_TEST_EQ(rootp->pl->obj, 1);
    BOOST_TEST_EQ(rootp->pr->obj, 2);
    BOOST_TEST_EQ(rootp->pf, nullptr);

    mybtPtr p = rootp->pl;  //1
    setLChild(p.get(), make_shared<mybt>(3));
    setRChild(p.get(), make_shared<mybt>(4));
    BOOST_TEST_EQ(p->obj, 1);
    BOOST_TEST_EQ(p->pl->obj, 3);
    BOOST_TEST_EQ(p->pr->obj, 4);
    BOOST_TEST_EQ(p->pf, rootp.get());
    BOOST_TEST(checkBinTree(rootp));

    //getHeight / getDepth / getNodeNum
    p = p->pr;  //4
    setRChild(p.get(), make_shared<mybt>(5));
    BOOST_TEST_EQ(getHeight(rootp.get()), 4);
    BOOST_TEST_EQ(getDepth(p.get()), 2);
    BOOST_TEST_EQ(getNodeNum(rootp.get()), 6);

    p = rootp->pr;  //2
    setRChild(p.get(), make_shared<mybt>(6));
    p = p->pr;  //6
    setRChild(p.get(), make_shared<mybt>(7));
    p = p->pr;  //7
    setRChild(p.get(), make_shared<mybt>(8));
    BOOST_TEST_EQ(getHeight(rootp.get()), 5);
    BOOST_TEST_EQ(getDepth(p.get()), 3);
    BOOST_TEST_EQ(getNodeNum(rootp.get()), 9);
    BOOST_TEST(checkBinTree(rootp));

    //前序遍历
    vector<mybtPtr> vec;
    vector<int> answerDLR = {0, 1, 3, 4, 5, 2, 6, 7, 8};
    DLR(rootp, vec);
    BOOST_TEST(checkBinTree(rootp));
    BOOST_TEST_ALL_WITH(vec.begin(), vec.end(), answerDLR.begin(), answerDLR.end(), [](mybtPtr re, int an) { return re->obj == an; });

    //中序遍历
    vec.clear();
    vector<int> answerLDR = {3, 1, 4, 5, 0, 2, 6, 7, 8};
    LDR(rootp, vec);
    BOOST_TEST(checkBinTree(rootp));
    BOOST_TEST_ALL_WITH(vec.begin(), vec.end(), answerLDR.begin(), answerLDR.end(), [](mybtPtr re, int an) { return re->obj == an; });

    //后续遍历
    vec.clear();
    vector<int> answerLRD = {3, 5, 4, 1, 8, 7, 6, 2, 0};
    LRD(rootp, vec);
    BOOST_TEST(checkBinTree(rootp));
    BOOST_TEST_ALL_WITH(vec.begin(), vec.end(), answerLRD.begin(), answerLRD.end(), [](mybtPtr re, int an) { return re->obj == an; });

    //分层遍历
    vec.clear();
    vector<int> answerTbl = {0, 1, 2, 3, 4, 6, 5, 7, 8};
    traByLevel(rootp, vec);
    BOOST_TEST(checkBinTree(rootp));
    BOOST_TEST_ALL_WITH(vec.begin(), vec.end(), answerTbl.begin(), answerTbl.end(), [](mybtPtr re, int an) { return re->obj == an; });

    //二叉树序列化 / 反序列化
    vector<pair<bool, int> > seriVec;
    SerializeTree(rootp, seriVec);
    BOOST_TEST(checkBinTree(rootp));
    mybtPtr rootp2;
    DeserializeTree(rootp2, seriVec);
    BOOST_TEST(checkBinTree(rootp2));
    vec.clear();
    traByLevel(rootp2, vec);
    BOOST_TEST_ALL_WITH(vec.begin(), vec.end(), answerTbl.begin(), answerTbl.end(), [](mybtPtr re, int an) { return re->obj == an; });

    //二叉树深拷贝
    mybtPtr rootp3 = copyTree(rootp);
    BOOST_TEST(checkBinTree(rootp));
    BOOST_TEST(checkBinTree(rootp3));
    vector<mybtPtr> vecCkCpy;
    traByLevel(rootp3, vecCkCpy);
    BOOST_TEST_ALL_WITH(vecCkCpy.begin(), vecCkCpy.end(), answerTbl.begin(), answerTbl.end(), [](mybtPtr re, int an) { return re->obj == an; });
    vec.clear();
    traByLevel(rootp, vec);
    BOOST_TEST_ALL_WITH(vecCkCpy.begin(), vecCkCpy.end(), vec.begin(), vec.end(), [](mybtPtr p1, mybtPtr p2) { return p1.get() != p2.get(); });

    //复杂节点操作（左）
    p = rootp->pl;
    BOOST_TEST_EQ(getLR(p.get()), true);
    setLChild(rootp.get(), make_shared<mybt>(-1));
    BOOST_TEST(checkBinTree(rootp));
    BOOST_TEST_EQ(rootp->pl->obj, -1);
    BOOST_TEST_EQ(p->pf, nullptr);

    setLChild(rootp.get(), p);
    BOOST_TEST(checkBinTree(rootp));
    BOOST_TEST_EQ(rootp->pl->obj, 1);
    BOOST_TEST_EQ(p->pf, rootp.get());

    breakLChild(rootp.get());
    BOOST_TEST(checkBinTree(rootp));
    BOOST_TEST_EQ(p->pf, nullptr);
    BOOST_TEST_NOT(rootp->pl);

    //复杂节点操作（右）
    p = rootp->pr;
    BOOST_TEST_EQ(getLR(p.get()), false);
    setRChild(rootp.get(), make_shared<mybt>(-1));
    BOOST_TEST(checkBinTree(rootp));
    BOOST_TEST_EQ(rootp->pr->obj, -1);
    BOOST_TEST_EQ(p->pf, nullptr);

    setRChild(rootp.get(), p);
    BOOST_TEST(checkBinTree(rootp));
    BOOST_TEST_EQ(rootp->pr->obj, 2);
    BOOST_TEST_EQ(p->pf, rootp.get());

    breakRChild(rootp.get());
    BOOST_TEST(checkBinTree(rootp));
    BOOST_TEST_EQ(p->pf, nullptr);
    BOOST_TEST_NOT(rootp->pr);
  }
  YT_DEBUG_PRINTF("test BinSearchTreeNode\n");
  {
    //二叉查找树BinSearchTreeNode
    typedef BinSearchTreeNode<int> mybst;
    typedef shared_ptr<mybst> mybstPtr;
    mybstPtr rootp = make_shared<mybst>(10);
    for (int ii = 0; ii < 20; ii += 2) {
      rootp->insert(make_shared<mybst>(ii));
    }
    BOOST_TEST(checkBinSearchTree(rootp));

    vector<mybstPtr> vec;
    vector<int> answerDLR = {10, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18};
    DLR(rootp, vec);
    BOOST_TEST(checkBinSearchTree(rootp));
    BOOST_TEST_ALL_WITH(vec.begin(), vec.end(), answerDLR.begin(), answerDLR.end(), [](mybstPtr re, int an) { return re->obj == an; });

    vec.clear();
    vector<int> answerTbl = {10, 0, 10, 2, 12, 4, 14, 6, 16, 8, 18};
    traByLevel(rootp, vec);
    BOOST_TEST(checkBinSearchTree(rootp));
    BOOST_TEST_ALL_WITH(vec.begin(), vec.end(), answerTbl.begin(), answerTbl.end(), [](mybstPtr re, int an) { return re->obj == an; });

    //二叉搜索树中进行查找binSearch
    mybstPtr bsRe = binSearch(rootp, 10);
    BOOST_TEST_EQ(bsRe.get(), rootp.get());
    bsRe = binSearch(rootp, 0);
    BOOST_TEST_EQ(bsRe.get(), rootp->pl.get());
    bsRe = binSearch(rootp, 12);
    BOOST_TEST_EQ(bsRe.get(), rootp->pr->pr.get());
    bsRe = binSearch(rootp, 9);
    BOOST_TEST_NOT(bsRe);
  }
  YT_DEBUG_PRINTF("test AVLTreeNode\n");
  {
    //AVL树
    typedef AVLTreeNode<int> myavlt;
    typedef shared_ptr<myavlt> myavltPtr;
    myavltPtr rootp = make_shared<myavlt>(10);

    //插入
    for (int ii = 0; ii < 20; ii += 2) {
      rootp = rootp->insert(make_shared<myavlt>(ii));
    }
    BOOST_TEST(checkAVLTree(rootp));

    vector<myavltPtr> vec;
    vector<int> answerLDR = {0, 2, 4, 6, 8, 10, 12, 14, 16, 18};
    LDR(rootp, vec);
    BOOST_TEST(checkAVLTree(rootp));
    BOOST_TEST_ALL_WITH(vec.begin(), vec.end(), answerLDR.begin(), answerLDR.end(), [](myavltPtr re, int an) { return re->obj == an; });

    vec.clear();
    vector<int> answerDLR = {6, 2, 0, 4, 14, 10, 8, 12, 16, 18};
    DLR(rootp, vec);
    BOOST_TEST(checkAVLTree(rootp));
    BOOST_TEST_ALL_WITH(vec.begin(), vec.end(), answerDLR.begin(), answerDLR.end(), [](myavltPtr re, int an) { return re->obj == an; });

    vec.clear();
    vector<int> answerTbl = {6, 2, 14, 0, 4, 10, 16, 8, 12, 18};
    traByLevel(rootp, vec);
    BOOST_TEST(checkAVLTree(rootp));
    BOOST_TEST_ALL_WITH(vec.begin(), vec.end(), answerTbl.begin(), answerTbl.end(), [](myavltPtr re, int an) { return re->obj == an; });

    //删除
    rootp = rootp->erase(100);
    BOOST_TEST(checkAVLTree(rootp));
    vec.clear();
    DLR(rootp, vec);
    BOOST_TEST_ALL_WITH(vec.begin(), vec.end(), answerDLR.begin(), answerDLR.end(), [](myavltPtr re, int an) { return re->obj == an; });

    rootp = rootp->erase(14);
    BOOST_TEST(checkAVLTree(rootp));
    vector<int> answerDLR2 = {6, 2, 0, 4, 12, 10, 8, 16, 18};
    vec.clear();
    DLR(rootp, vec);
    BOOST_TEST_ALL_WITH(vec.begin(), vec.end(), answerDLR2.begin(), answerDLR2.end(), [](myavltPtr re, int an) { return re->obj == an; });

    //删除根节点
    rootp = rootp->erase(rootp);
    BOOST_TEST(checkAVLTree(rootp));
    vector<int> answerDLR3 = {4, 2, 0, 12, 10, 8, 16, 18};
    vec.clear();
    DLR(rootp, vec);
    BOOST_TEST_ALL_WITH(vec.begin(), vec.end(), answerDLR3.begin(), answerDLR3.end(), [](myavltPtr re, int an) { return re->obj == an; });

    rootp = rootp->erase(0);
    BOOST_TEST(checkAVLTree(rootp));
    vector<int> answerDLR4 = {12, 4, 2, 10, 8, 16, 18};
    vec.clear();
    DLR(rootp, vec);
    BOOST_TEST_ALL_WITH(vec.begin(), vec.end(), answerDLR4.begin(), answerDLR4.end(), [](myavltPtr re, int an) { return re->obj == an; });

    rootp = rootp->erase(rootp->pr->pr);
    BOOST_TEST(checkAVLTree(rootp));
    vector<int> answerDLR5 = {10, 4, 2, 8, 12, 16};
    vec.clear();
    DLR(rootp, vec);
    BOOST_TEST_ALL_WITH(vec.begin(), vec.end(), answerDLR5.begin(), answerDLR5.end(), [](myavltPtr re, int an) { return re->obj == an; });
  }
  YT_DEBUG_PRINTF("test BRTreeNode\n");  //todo 未完成
  {
    //红黑树
    typedef BRTreeNode<int> mybrt;
    typedef shared_ptr<mybrt> mybrtPtr;
    mybrtPtr rootp = make_shared<mybrt>(10);
    //插入

    for (int ii = 0; ii < 20; ii += 2) {
      rootp = rootp->insert(make_shared<mybrt>(ii));
    }
    BOOST_TEST(checkBRTree(rootp));

    //删除
  }
}

void test_Graph() {
  YT_DEBUG_PRINTF("test Graph\n");  //todo 未完成
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

  BOOST_TEST(isUndirGraph(myGraphVec));

  vector<myGraph*> myGraphVec2 = copyGraph(myGraphVec);
  BOOST_TEST_ALL_WITH(myGraphVec.begin(), myGraphVec.end(), myGraphVec2.begin(), myGraphVec2.end(), [](myGraph* val1, myGraph* val2) { return (val1 != val2) && (val1->obj == val2->obj); });

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

void test_Heap() {
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