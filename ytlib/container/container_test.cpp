#include <gtest/gtest.h>

#include "binary_tree.hpp"
#include "graph.hpp"
#include "heap.hpp"
#include "ring_buf.hpp"
#include "shared_buf.hpp"

#include "ytlib/misc/stl_util.hpp"

namespace ytlib {

TEST(BINARY_TREE_TEST, SET_test) {
  typedef BinTreeNode<int> Bt;

  struct TestCase {
    std::string name;

    Bt::NodePtr node1;
    Bt::NodePtr node2;

    Bt::NodePtr want_node1_pf;
    Bt::NodePtr want_node1_pl;
    Bt::NodePtr want_node1_pr;

    Bt::NodePtr want_node2_pf;
    Bt::NodePtr want_node2_pl;
    Bt::NodePtr want_node2_pr;
  };
  std::vector<TestCase> test_cases;
  {
    TestCase test_case{
        .name = "case 1",
        .node1 = std::make_shared<Bt>(1),
        .node2 = std::make_shared<Bt>(2)};

    test_cases.emplace_back(std::move(test_case));
  }
  {
    TestCase test_case{
        .name = "case 2",
        .node1 = std::make_shared<Bt>(1),
        .node2 = std::make_shared<Bt>(2)};

    SetLChild(test_case.node1, test_case.node2);

    test_case.want_node1_pl = test_case.node2;
    test_case.want_node2_pf = test_case.node1;

    test_cases.emplace_back(std::move(test_case));
  }
  {
    TestCase test_case{
        .name = "case 3",
        .node1 = std::make_shared<Bt>(1),
        .node2 = std::make_shared<Bt>(2)};

    SetRChild(test_case.node1, test_case.node2);

    test_case.want_node1_pr = test_case.node2;
    test_case.want_node2_pf = test_case.node1;

    test_cases.emplace_back(std::move(test_case));
  }
  {
    TestCase test_case{
        .name = "case 4",
        .node1 = std::make_shared<Bt>(1),
        .node2 = std::make_shared<Bt>(2)};

    SetLChild(test_case.node1, test_case.node2);
    BreakLChild(test_case.node1);

    test_cases.emplace_back(std::move(test_case));
  }
  {
    TestCase test_case{
        .name = "case 5",
        .node1 = std::make_shared<Bt>(1),
        .node2 = std::make_shared<Bt>(2)};

    SetRChild(test_case.node1, test_case.node2);
    BreakRChild(test_case.node1);

    test_cases.emplace_back(std::move(test_case));
  }
  {
    TestCase test_case{
        .name = "case 6",
        .node1 = std::make_shared<Bt>(1),
        .node2 = std::make_shared<Bt>(2)};

    SetRChild(test_case.node1, test_case.node2);
    BreakFather(test_case.node2);

    test_cases.emplace_back(std::move(test_case));
  }
  {
    TestCase test_case{
        .name = "case 7",
        .node1 = std::make_shared<Bt>(1),
        .node2 = std::make_shared<Bt>(2)};

    SetRChild(test_case.node1, test_case.node2);
    SetLChild(test_case.node1, test_case.node2);

    test_case.want_node1_pl = test_case.node2;
    test_case.want_node2_pf = test_case.node1;

    test_cases.emplace_back(std::move(test_case));
  }
  {
    TestCase test_case{
        .name = "case 8",
        .node1 = std::make_shared<Bt>(1),
        .node2 = std::make_shared<Bt>(2)};

    SetLChild(test_case.node1, test_case.node2);
    SetRChild(test_case.node1, test_case.node2);

    test_case.want_node1_pr = test_case.node2;
    test_case.want_node2_pf = test_case.node1;

    test_cases.emplace_back(std::move(test_case));
  }

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    EXPECT_EQ(test_cases[ii].node1->pf.lock(), test_cases[ii].want_node1_pf)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
    EXPECT_EQ(test_cases[ii].node1->pl, test_cases[ii].want_node1_pl)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
    EXPECT_EQ(test_cases[ii].node1->pr, test_cases[ii].want_node1_pr)
        << "Test " << test_cases[ii].name << " failed, index " << ii;

    EXPECT_EQ(test_cases[ii].node2->pf.lock(), test_cases[ii].want_node2_pf)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
    EXPECT_EQ(test_cases[ii].node2->pl, test_cases[ii].want_node2_pl)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
    EXPECT_EQ(test_cases[ii].node2->pr, test_cases[ii].want_node2_pr)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(BINARY_TREE_TEST, CHECK_CHILD_test) {
  typedef BinTreeNode<int> Bt;

  struct TestCase {
    std::string name;

    Bt::NodePtr node1;
    Bt::NodePtr node2;

    bool want_result_CheckLChild;
    bool want_result_CheckRChild;

    bool want_exp_GetLR;
    bool want_result_GetLR;
  };
  std::vector<TestCase> test_cases;
  {
    TestCase test_case{
        .name = "case 1",
        .node1 = std::make_shared<Bt>(1),
        .node2 = std::make_shared<Bt>(2),
        .want_result_CheckLChild = false,
        .want_result_CheckRChild = false,
        .want_exp_GetLR = true,
        .want_result_GetLR = false};

    test_cases.emplace_back(std::move(test_case));
  }
  {
    TestCase test_case{
        .name = "case 2",
        .node1 = std::make_shared<Bt>(1),
        .node2 = std::make_shared<Bt>(2),
        .want_result_CheckLChild = true,
        .want_result_CheckRChild = false,
        .want_exp_GetLR = false,
        .want_result_GetLR = true};

    SetLChild(test_case.node1, test_case.node2);

    test_cases.emplace_back(std::move(test_case));
  }
  {
    TestCase test_case{
        .name = "case 3",
        .node1 = std::make_shared<Bt>(1),
        .node2 = std::make_shared<Bt>(2),
        .want_result_CheckLChild = false,
        .want_result_CheckRChild = true,
        .want_exp_GetLR = false,
        .want_result_GetLR = false};

    SetRChild(test_case.node1, test_case.node2);

    test_cases.emplace_back(std::move(test_case));
  }

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    EXPECT_EQ(CheckLChild(test_cases[ii].node1, test_cases[ii].node2), test_cases[ii].want_result_CheckLChild)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
    EXPECT_EQ(CheckRChild(test_cases[ii].node1, test_cases[ii].node2), test_cases[ii].want_result_CheckRChild)
        << "Test " << test_cases[ii].name << " failed, index " << ii;

    bool get_exp = false;
    try {
      EXPECT_EQ(GetLR(test_cases[ii].node2), test_cases[ii].want_result_GetLR)
          << "Test " << test_cases[ii].name << " failed, index " << ii;
    } catch (const std::exception&) {
      get_exp = true;
    }
    EXPECT_EQ(get_exp, test_cases[ii].want_exp_GetLR)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(BINARY_TREE_TEST, SERIALIZE_test) {
  typedef BinTreeNode<int> Bt;

  std::function<std::string(const std::pair<bool, int>&)> print_fun =
      [](const std::pair<bool, int>& val) -> std::string {
    return val.first ? std::to_string(val.second) : "null";
  };

  struct TestCase {
    std::string name;

    Bt::NodePtr root_node;

    std::vector<std::pair<bool, int> > want_result;
  };
  std::vector<TestCase> test_cases;
  {
    TestCase test_case{
        .name = "case 1",
        .root_node = std::make_shared<Bt>(1),
        .want_result = {{true, 1}, {false, 0}, {false, 0}}};

    test_cases.emplace_back(std::move(test_case));
  }
  {
    TestCase test_case{
        .name = "case 2",
        .root_node = std::make_shared<Bt>(1),
        .want_result = {{true, 1}, {true, 3}, {false, 0}, {false, 0}, {true, 2}, {false, 0}, {false, 0}}};

    SetRChild(test_case.root_node, std::make_shared<Bt>(2));
    SetLChild(test_case.root_node, std::make_shared<Bt>(3));

    test_cases.emplace_back(std::move(test_case));
  }

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    std::vector<std::pair<bool, int> > ret_vec;
    SerializeTree(test_cases[ii].root_node, ret_vec);
    EXPECT_STREQ(Vec2Str(ret_vec, print_fun).c_str(), Vec2Str(test_cases[ii].want_result, print_fun).c_str())
        << "Test " << test_cases[ii].name << " failed, index " << ii;

    Bt::NodePtr ret_node;
    DeserializeTree(ret_node, test_cases[ii].want_result);
    EXPECT_TRUE(CompareTreeObj(test_cases[ii].root_node, ret_node))
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(BINARY_TREE_TEST, TRAVERSE_test) {
  typedef BinTreeNode<int> Bt;

  std::function<std::string(const Bt::NodePtr&)> print_fun =
      [](const Bt::NodePtr& val) -> std::string {
    return val ? std::to_string(val->obj) : "null";
  };

  struct TestCase {
    std::string name;

    Bt::NodePtr root_node;

    std::vector<int> want_result_DLR;
    std::vector<int> want_result_LDR;
    std::vector<int> want_result_LRD;
    std::vector<int> want_result_TraByLevel;
  };
  std::vector<TestCase> test_cases;
  {
    TestCase test_case{
        .name = "case 1",
        .root_node = std::make_shared<Bt>(1),
        .want_result_DLR = {1},
        .want_result_LDR = {1},
        .want_result_LRD = {1},
        .want_result_TraByLevel = {1}};

    test_cases.emplace_back(std::move(test_case));
  }
  {
    TestCase test_case{
        .name = "case 2",
        .root_node = std::make_shared<Bt>(1),
        .want_result_DLR = {1, 3, 2},
        .want_result_LDR = {3, 1, 2},
        .want_result_LRD = {3, 2, 1},
        .want_result_TraByLevel = {1, 3, 2}};

    SetRChild(test_case.root_node, std::make_shared<Bt>(2));
    SetLChild(test_case.root_node, std::make_shared<Bt>(3));

    test_cases.emplace_back(std::move(test_case));
  }
  {
    TestCase test_case{
        .name = "case 3",
        .root_node = std::make_shared<Bt>(1),
        .want_result_DLR = {1, 3, 4, 5, 7, 2, 6},
        .want_result_LDR = {4, 3, 5, 7, 1, 2, 6},
        .want_result_LRD = {4, 7, 5, 3, 6, 2, 1},
        .want_result_TraByLevel = {1, 3, 2, 4, 5, 6, 7}};

    SetRChild(test_case.root_node, std::make_shared<Bt>(2));
    SetLChild(test_case.root_node, std::make_shared<Bt>(3));
    SetLChild(test_case.root_node->pl, std::make_shared<Bt>(4));
    SetRChild(test_case.root_node->pl, std::make_shared<Bt>(5));
    SetRChild(test_case.root_node->pr, std::make_shared<Bt>(6));
    SetRChild(test_case.root_node->pl->pr, std::make_shared<Bt>(7));

    test_cases.emplace_back(std::move(test_case));
  }

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    std::vector<Bt::NodePtr> ret_DLR;
    DLR(test_cases[ii].root_node, ret_DLR);
    EXPECT_STREQ(Vec2Str(ret_DLR, print_fun).c_str(), Vec2Str(test_cases[ii].want_result_DLR).c_str())
        << "Test " << test_cases[ii].name << " failed, index " << ii;

    std::vector<Bt::NodePtr> ret_LDR;
    LDR(test_cases[ii].root_node, ret_LDR);
    EXPECT_STREQ(Vec2Str(ret_LDR, print_fun).c_str(), Vec2Str(test_cases[ii].want_result_LDR).c_str())
        << "Test " << test_cases[ii].name << " failed, index " << ii;

    std::vector<Bt::NodePtr> ret_LRD;
    LRD(test_cases[ii].root_node, ret_LRD);
    EXPECT_STREQ(Vec2Str(ret_LRD, print_fun).c_str(), Vec2Str(test_cases[ii].want_result_LRD).c_str())
        << "Test " << test_cases[ii].name << " failed, index " << ii;

    std::vector<Bt::NodePtr> ret_TraByLevel;
    TraByLevel(test_cases[ii].root_node, ret_TraByLevel);
    EXPECT_STREQ(Vec2Str(ret_TraByLevel, print_fun).c_str(), Vec2Str(test_cases[ii].want_result_TraByLevel).c_str())
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(BINARY_TREE_TEST, CHECK_TREE_test) {
  typedef BinTreeNode<int> Bt;
  typedef BinSearchTreeNode<int> Bst;

  struct TestCase {
    std::string name;

    Bt::NodePtr root_node;

    bool want_result_CheckBinTree;
    bool want_result_CheckBinSearchTree;
  };
  std::vector<TestCase> test_cases;
  {
    TestCase test_case{
        .name = "case 1",
        .root_node = std::make_shared<Bt>(1),
        .want_result_CheckBinTree = true,
        .want_result_CheckBinSearchTree = true};

    test_cases.emplace_back(std::move(test_case));
  }
  {
    TestCase test_case{
        .name = "case 2",
        .want_result_CheckBinTree = true,
        .want_result_CheckBinSearchTree = false};

    Bt::NodePtr node = std::make_shared<Bt>(1);
    SetRChild(node, std::make_shared<Bt>(2));
    SetLChild(node, std::make_shared<Bt>(3));

    CopyTree(node, test_case.root_node);

    test_cases.emplace_back(std::move(test_case));
  }
  {
    TestCase test_case{
        .name = "case 3",
        .want_result_CheckBinTree = true,
        .want_result_CheckBinSearchTree = true};

    Bst::NodePtr node = std::make_shared<Bst>(2);

    for (int ii = 0; ii < 5; ++ii) {
      node->Insert(std::make_shared<Bst>(ii));
    }

    CopyTree(node, test_case.root_node);

    test_cases.emplace_back(std::move(test_case));
  }

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    EXPECT_EQ(CheckBinTree(test_cases[ii].root_node), test_cases[ii].want_result_CheckBinTree)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(BINARY_TREE_TEST, GET_ATTR_test) {
  typedef BinTreeNode<int> Bt;

  struct TestCase {
    std::string name;

    Bt::NodePtr root_node;
    Bt::NodePtr target_node;

    size_t want_result_GetDepth;
    size_t want_result_GetHeight;
    size_t want_result_GetMaxChain;
    size_t want_result_GetMinChain;
    size_t want_result_GetNodeNum;
    Bt::NodePtr want_result_GetRootNode;
  };
  std::vector<TestCase> test_cases;
  {
    TestCase test_case{
        .name = "case 1",
        .want_result_GetDepth = 0,
        .want_result_GetHeight = 0,
        .want_result_GetMaxChain = 0,
        .want_result_GetMinChain = 0,
        .want_result_GetNodeNum = 0};

    test_cases.emplace_back(std::move(test_case));
  }
  {
    TestCase test_case{
        .name = "case 2",
        .root_node = std::make_shared<Bt>(1),
        .want_result_GetDepth = 0,
        .want_result_GetHeight = 1,
        .want_result_GetMaxChain = 1,
        .want_result_GetMinChain = 1,
        .want_result_GetNodeNum = 1};

    test_case.target_node = test_case.root_node;
    test_case.want_result_GetRootNode = test_case.root_node;

    test_cases.emplace_back(std::move(test_case));
  }
  {
    TestCase test_case{
        .name = "case 3",
        .root_node = std::make_shared<Bt>(1),
        .want_result_GetDepth = 0,
        .want_result_GetHeight = 4,
        .want_result_GetMaxChain = 4,
        .want_result_GetMinChain = 3,
        .want_result_GetNodeNum = 7};

    SetRChild(test_case.root_node, std::make_shared<Bt>(2));
    SetLChild(test_case.root_node, std::make_shared<Bt>(3));
    SetLChild(test_case.root_node->pl, std::make_shared<Bt>(4));
    SetRChild(test_case.root_node->pl, std::make_shared<Bt>(5));
    SetRChild(test_case.root_node->pr, std::make_shared<Bt>(6));
    SetRChild(test_case.root_node->pl->pr, std::make_shared<Bt>(7));

    test_case.target_node = test_case.root_node;
    test_case.want_result_GetRootNode = test_case.root_node;

    test_cases.emplace_back(std::move(test_case));
  }
  {
    TestCase test_case{
        .name = "case 4",
        .root_node = std::make_shared<Bt>(1),
        .want_result_GetDepth = 1,
        .want_result_GetHeight = 4,
        .want_result_GetMaxChain = 4,
        .want_result_GetMinChain = 2,
        .want_result_GetNodeNum = 6};

    SetLChild(test_case.root_node, std::make_shared<Bt>(2));
    SetRChild(test_case.root_node, std::make_shared<Bt>(3));

    SetLChild(test_case.root_node->pl, std::make_shared<Bt>(4));
    SetRChild(test_case.root_node->pl, std::make_shared<Bt>(5));

    SetLChild(test_case.root_node->pl->pl, std::make_shared<Bt>(6));
    SetRChild(test_case.root_node->pl->pl, std::make_shared<Bt>(7));

    SetLChild(test_case.root_node->pl->pl->pl, std::make_shared<Bt>(8));

    test_case.target_node = test_case.root_node->pl;
    test_case.want_result_GetRootNode = test_case.root_node;

    test_cases.emplace_back(std::move(test_case));
  }

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    EXPECT_EQ(GetDepth(test_cases[ii].target_node), test_cases[ii].want_result_GetDepth)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
    EXPECT_EQ(GetHeight(test_cases[ii].target_node), test_cases[ii].want_result_GetHeight)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
    EXPECT_EQ(GetMaxChain(test_cases[ii].target_node), test_cases[ii].want_result_GetMaxChain)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
    EXPECT_EQ(GetMinChain(test_cases[ii].target_node), test_cases[ii].want_result_GetMinChain)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
    EXPECT_EQ(GetNodeNum(test_cases[ii].target_node), test_cases[ii].want_result_GetNodeNum)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
    EXPECT_EQ(GetRootNode(test_cases[ii].target_node), test_cases[ii].want_result_GetRootNode)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(BINARY_TREE_TEST, BinSearchTreeNode_test) {
  typedef BinSearchTreeNode<int> Bst;

  std::function<std::string(const std::pair<bool, int>&)> print_fun =
      [](const std::pair<bool, int>& val) -> std::string {
    return val.first ? std::to_string(val.second) : "null";
  };

  struct TestCase {
    std::string name;

    Bst::NodePtr root_node;

    std::vector<std::pair<bool, int> > want_result;
  };
  std::vector<TestCase> test_cases;
  {
    TestCase test_case{
        .name = "case 1",
        .root_node = std::make_shared<Bst>(1),
        .want_result = {{true, 1}, {false, 0}, {false, 0}}};

    test_cases.emplace_back(std::move(test_case));
  }
  {
    TestCase test_case{
        .name = "case 2",
        .root_node = std::make_shared<Bst>(3),
        .want_result = {
            {true, 3},
            {true, 0},
            {false, 0},
            {true, 1},
            {false, 0},
            {true, 2},
            {false, 0},
            {false, 0},
            {true, 3},
            {false, 0},
            {true, 4},
            {false, 0},
            {false, 0}}};

    for (int ii = 0; ii < 5; ++ii) {
      test_case.root_node->Insert(std::make_shared<Bst>(ii));
    }

    test_cases.emplace_back(std::move(test_case));
  }
  {
    TestCase test_case{
        .name = "case 3",
        .root_node = std::make_shared<Bst>(3),
        .want_result = {
            {true, 3},
            {true, 0},
            {false, 0},
            {true, 1},
            {false, 0},
            {false, 0},
            {true, 3},
            {false, 0},
            {true, 4},
            {false, 0},
            {false, 0}}};

    for (int ii = 0; ii < 5; ++ii) {
      test_case.root_node->Insert(std::make_shared<Bst>(ii));
    }

    test_case.root_node->pl->pr->pr->Erase();

    test_cases.emplace_back(std::move(test_case));
  }
  {
    TestCase test_case{
        .name = "case 4",
        .root_node = std::make_shared<Bst>(3),
        .want_result = {
            {true, 3},
            {true, 0},
            {false, 0},
            {true, 1},
            {false, 0},
            {true, 2},
            {false, 0},
            {false, 0},
            {true, 4},
            {false, 0},
            {false, 0}}};

    for (int ii = 0; ii < 5; ++ii) {
      test_case.root_node->Insert(std::make_shared<Bst>(ii));
    }

    test_case.root_node->pr->Erase();

    test_cases.emplace_back(std::move(test_case));
  }
  {
    TestCase test_case{
        .name = "case 5",
        .root_node = std::make_shared<Bst>(3),
        .want_result = {
            {true, 2},
            {true, 0},
            {false, 0},
            {true, 1},
            {false, 0},
            {false, 0},
            {true, 3},
            {false, 0},
            {true, 4},
            {false, 0},
            {false, 0}}};

    for (int ii = 0; ii < 5; ++ii) {
      test_case.root_node->Insert(std::make_shared<Bst>(ii));
    }

    test_case.root_node = test_case.root_node->Erase();

    test_cases.emplace_back(std::move(test_case));
  }

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    std::vector<std::pair<bool, int> > ret_vec;
    SerializeTree(test_cases[ii].root_node, ret_vec);
    EXPECT_STREQ(Vec2Str(ret_vec, print_fun).c_str(), Vec2Str(test_cases[ii].want_result, print_fun).c_str())
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(BINARY_TREE_TEST, BinSearch_test) {
  typedef BinSearchTreeNode<int> Bst;

  struct TestCase {
    std::string name;

    Bst::NodePtr root_node;
    int val;

    Bst::NodePtr want_result;
  };
  std::vector<TestCase> test_cases;
  {
    TestCase test_case{
        .name = "case 1",
        .root_node = std::make_shared<Bst>(3),
        .val = 2};

    test_case.root_node->Insert(std::make_shared<Bst>(1));
    test_case.root_node->Insert(std::make_shared<Bst>(3));
    test_case.root_node->Insert(std::make_shared<Bst>(4));

    test_case.want_result = std::make_shared<Bst>(2);
    test_case.root_node->Insert(test_case.want_result);

    test_cases.emplace_back(std::move(test_case));
  }
  {
    TestCase test_case{
        .name = "case 2",
        .root_node = std::make_shared<Bst>(3),
        .val = 2};

    test_case.root_node->Insert(std::make_shared<Bst>(1));
    test_case.root_node->Insert(std::make_shared<Bst>(3));
    test_case.root_node->Insert(std::make_shared<Bst>(4));

    test_case.want_result = std::shared_ptr<Bst>();

    test_cases.emplace_back(std::move(test_case));
  }

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = BinSearch(test_cases[ii].root_node, test_cases[ii].val);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(BINARY_TREE_TEST, AVLTreeNode_test) {
  typedef AVLTreeNode<int> Avlt;

  std::function<std::string(const std::pair<bool, int>&)> print_fun =
      [](const std::pair<bool, int>& val) -> std::string {
    return val.first ? std::to_string(val.second) : "null";
  };

  struct TestCase {
    std::string name;

    Avlt::NodePtr root_node;

    std::vector<std::pair<bool, int> > want_result;
  };
  std::vector<TestCase> test_cases;
  {
    TestCase test_case{
        .name = "case 1",
        .root_node = std::make_shared<Avlt>(1),
        .want_result = {{true, 1}, {false, 0}, {false, 0}}};

    test_cases.emplace_back(std::move(test_case));
  }

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    std::vector<std::pair<bool, int> > ret_vec;
    SerializeTree(test_cases[ii].root_node, ret_vec);
    EXPECT_STREQ(Vec2Str(ret_vec, print_fun).c_str(), Vec2Str(test_cases[ii].want_result, print_fun).c_str())
        << "Test " << test_cases[ii].name << " failed, index " << ii;

    EXPECT_TRUE(CheckAVLTree(test_cases[ii].root_node))
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(BINARY_TREE_TEST, BRTreeNode_test) {
  typedef BRTreeNode<int> Brt;

  std::function<std::string(const std::pair<bool, int>&)> print_fun =
      [](const std::pair<bool, int>& val) -> std::string {
    return val.first ? std::to_string(val.second) : "null";
  };

  struct TestCase {
    std::string name;

    Brt::NodePtr root_node;

    std::vector<std::pair<bool, int> > want_result;
  };
  std::vector<TestCase> test_cases;
  {
    TestCase test_case{
        .name = "case 1",
        .root_node = std::make_shared<Brt>(1),
        .want_result = {{true, 1}, {false, 0}, {false, 0}}};

    test_cases.emplace_back(std::move(test_case));
  }

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    std::vector<std::pair<bool, int> > ret_vec;
    SerializeTree(test_cases[ii].root_node, ret_vec);
    EXPECT_STREQ(Vec2Str(ret_vec, print_fun).c_str(), Vec2Str(test_cases[ii].want_result, print_fun).c_str())
        << "Test " << test_cases[ii].name << " failed, index " << ii;

    EXPECT_TRUE(CheckBRTree(test_cases[ii].root_node))
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(RSTUDIO_CONTAINER_TEST, RingBuf_TEST) {
  class TestClass {
   public:
    TestClass() {}
    TestClass(uint32_t id) : id_(id) {}

    uint32_t id_ = 0;
  };

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

TEST(RSTUDIO_CONTAINER_TEST, Graph_TEST) {
  //todo 未完成
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

  //dfs
  std::vector<myGraph*> vec;
  clearFlag(myGraphVec);
  DFS(*myGraphVec[0], vec);

  //bfs
  vec.clear();
  clearFlag(myGraphVec);
  BFS(*myGraphVec[0], vec);

  //邻接矩阵
  g_sideMatrix M = createAdjMatrix<uint32_t>(myGraphVec);

  std::cout << M << std::endl;
  std::cout << std::endl;

  //dijkstra
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

  //floyd
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

TEST(RSTUDIO_CONTAINER_TEST, Heap_TEST) {
  //todo 未完成
  int data[10] = {7, 9, 2, 6, 4, 8, 1, 3, 10, 5};
  Heap<int> h1(data, 10);
  for (int ii = 0; ii < 10; ++ii) {
    std::cout << h1.container[ii] << " ";
  }
  std::cout << std::endl;

  h1.sort();
  for (int ii = 0; ii < 10; ++ii) {
    std::cout << h1.container[ii] << " ";
  }
  std::cout << std::endl;

  int data2[15] = {7, -5, 9, -7, 2, 6, -6, 4, 8, 1, 3, -8, 10, 5, -9};
  h1.assign(data2, 15);
  h1.sort();

  for (int ii = 0; ii < 15; ++ii) {
    std::cout << h1.container[ii] << " ";
  }
  std::cout << std::endl;

  h1.push(-3);
  h1.push(-10);
  h1.push(11);
  for (int ii = 0; ii < 18; ++ii) {
    std::cout << h1.container[ii] << " ";
  }

  h1.pop();
  h1.pop();
  h1.pop();
  for (int ii = 0; ii < 15; ++ii) {
    std::cout << h1.container[ii] << " ";
  }
  std::cout << std::endl;
}

TEST(MISC_TEST, sharedBuf_BASE) {
  std::string s = "test test";
  uint32_t n = static_cast<uint32_t>(s.size());
  SharedBuf buf1(n);
  ASSERT_EQ(buf1.Size(), n);

  buf1 = SharedBuf(s);
  ASSERT_STREQ(std::string(buf1.Get(), n).c_str(), s.c_str());

  // 浅拷贝
  SharedBuf buf2(buf1.GetSharedPtr(), n);
  ASSERT_STREQ(std::string(buf2.Get(), n).c_str(), s.c_str());
  ASSERT_EQ(buf1.Get(), buf2.Get());

  // 深拷贝
  SharedBuf buf3(buf1.Get(), n);
  ASSERT_STREQ(std::string(buf3.Get(), n).c_str(), s.c_str());
  ASSERT_NE(buf1.Get(), buf3.Get());

  // 深拷贝
  SharedBuf buf4 = SharedBuf::GetDeepCopy(buf1);
  ASSERT_STREQ(std::string(buf4.Get(), n).c_str(), s.c_str());
  ASSERT_NE(buf1.Get(), buf4.Get());
}

}  // namespace ytlib
