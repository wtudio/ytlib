#include <gtest/gtest.h>

#include "binary_tree.hpp"

#include "ytlib/misc/stl_util.hpp"

// todo 未完成
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

TEST(BINARY_TREE_TEST, BinSearchTreeNode_Erase_RightChild_test) {
  typedef BinSearchTreeNode<int> Bst;

  // case A: 待删除节点是父的右孩子，且只有左子树。
  // 构造: root=5, root->pr=7, root->pr->pl=6
  // 删除 7 后期望 root->pr == 6（非 root->pl）。
  {
    Bst::NodePtr root = std::make_shared<Bst>(5);
    root->Insert(std::make_shared<Bst>(7));
    root->Insert(std::make_shared<Bst>(6));

    ASSERT_TRUE(CheckBinSearchTree(root));
    ASSERT_EQ(root->pr->obj, 7);
    ASSERT_EQ(root->pr->pl->obj, 6);

    root->pr->Erase();

    EXPECT_FALSE(static_cast<bool>(root->pl));
    ASSERT_TRUE(static_cast<bool>(root->pr));
    EXPECT_EQ(root->pr->obj, 6);
    EXPECT_EQ(root->pr->pf.lock(), root);
    EXPECT_TRUE(CheckBinSearchTree(root));
  }

  // case B: 待删除节点是父的右孩子，且只有右子树。
  // 构造: root=5, root->pr=7, root->pr->pr=8
  // 删除 7 后期望 root->pr == 8。
  {
    Bst::NodePtr root = std::make_shared<Bst>(5);
    root->Insert(std::make_shared<Bst>(7));
    root->Insert(std::make_shared<Bst>(8));

    ASSERT_TRUE(CheckBinSearchTree(root));
    ASSERT_EQ(root->pr->obj, 7);
    ASSERT_EQ(root->pr->pr->obj, 8);

    root->pr->Erase();

    EXPECT_FALSE(static_cast<bool>(root->pl));
    ASSERT_TRUE(static_cast<bool>(root->pr));
    EXPECT_EQ(root->pr->obj, 8);
    EXPECT_EQ(root->pr->pf.lock(), root);
    EXPECT_TRUE(CheckBinSearchTree(root));
  }

  // case C: 待删除节点是父的左孩子，且只有左子树。
  {
    Bst::NodePtr root = std::make_shared<Bst>(5);
    root->Insert(std::make_shared<Bst>(3));
    root->Insert(std::make_shared<Bst>(2));

    ASSERT_TRUE(CheckBinSearchTree(root));

    root->pl->Erase();

    ASSERT_TRUE(static_cast<bool>(root->pl));
    EXPECT_FALSE(static_cast<bool>(root->pr));
    EXPECT_EQ(root->pl->obj, 2);
    EXPECT_EQ(root->pl->pf.lock(), root);
    EXPECT_TRUE(CheckBinSearchTree(root));
  }

  // case D: 待删除节点是父的左孩子，且只有右子树。
  {
    Bst::NodePtr root = std::make_shared<Bst>(5);
    root->Insert(std::make_shared<Bst>(3));
    root->Insert(std::make_shared<Bst>(4));

    ASSERT_TRUE(CheckBinSearchTree(root));

    root->pl->Erase();

    ASSERT_TRUE(static_cast<bool>(root->pl));
    EXPECT_FALSE(static_cast<bool>(root->pr));
    EXPECT_EQ(root->pl->obj, 4);
    EXPECT_EQ(root->pl->pf.lock(), root);
    EXPECT_TRUE(CheckBinSearchTree(root));
  }
}

TEST(BINARY_TREE_TEST, AVLTreeNode_Erase_RightChild_test) {
  typedef AVLTreeNode<int> Avlt;

  // 构造一棵平衡的 AVL 树，使被删除节点是父的右孩子且单边子树
  // 这里用 Erase(val) 接口。
  // 插入序列设计：5, 3, 8, 9 → root=5, pr=8, pr->pr=9 (AVL 仍平衡 hgt 2/3)。
  // 实际上 5 插入 3 后 8 后 9 时会触发旋转，需要小心选择。
  // 改用 Erase(node) 直接构造：插入后 Erase 一个右孩子单子树节点。
  {
    Avlt::NodePtr root = std::make_shared<Avlt>(10);
    root = root->Insert(std::make_shared<Avlt>(5));
    root = root->Insert(std::make_shared<Avlt>(15));
    root = root->Insert(std::make_shared<Avlt>(20));
    // 树形：root=10, pl=5, pr=15, pr->pr=20

    ASSERT_TRUE(CheckAVLTree(root));
    ASSERT_EQ(root->obj, 10);
    ASSERT_TRUE(static_cast<bool>(root->pr));

    // 删除 20 (叶子) 后再删 15 (此时 15 是右孩子且无子节点，特殊路径)
    root = root->Erase(20);
    ASSERT_TRUE(CheckAVLTree(root));

    // 重新构造 15 是右孩子且只有左子树的情形
    Avlt::NodePtr root2 = std::make_shared<Avlt>(10);
    root2 = root2->Insert(std::make_shared<Avlt>(5));
    root2 = root2->Insert(std::make_shared<Avlt>(15));
    root2 = root2->Insert(std::make_shared<Avlt>(13));
    // root=10, pl=5, pr=15, pr->pl=13

    ASSERT_TRUE(CheckAVLTree(root2));
    root2 = root2->Erase(15);
    EXPECT_TRUE(CheckAVLTree(root2));
    EXPECT_TRUE(CheckBinSearchTree(root2));
    // 删除后期望 13 顶替 15
    ASSERT_TRUE(static_cast<bool>(root2->pr));
    EXPECT_EQ(root2->pr->obj, 13);
  }
}

TEST(BINARY_TREE_TEST, CheckBRTree_NullRoot_test) {
  typedef BRTreeNode<int> Brt;
  Brt::NodePtr empty;
  EXPECT_TRUE(CheckBRTree(empty));
}

}  // namespace ytlib
