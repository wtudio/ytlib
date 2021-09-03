/**
 * @file binary_tree.hpp
 * @brief 二叉树
 * @note 提供模板化的一些树的实现和相关的算法，包括AVL树、红黑树
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include <algorithm>
#include <memory>
#include <vector>

namespace ytlib {

/**
 * @brief 二叉树
 * @note 一个BinTreeNode实例表示一个二叉树节点，也表示以此节点为根节点的一棵二叉树。
 * 如果根节点被析构，那么整个树中所有子节点将被析构，除非子节点有另外的智能指针指着
 * @tparam T 实际节点类型
 */
template <typename T>
class BinTreeNode : public std::enable_shared_from_this<BinTreeNode<T> > {
 public:
  typedef std::shared_ptr<BinTreeNode<T> > NodePtr;
  typedef std::weak_ptr<BinTreeNode<T> > NodeWeakPtr;

 public:
  BinTreeNode() {}
  explicit BinTreeNode(const T& input_obj) : obj(input_obj) {}

 public:
  T obj;           ///<实际节点值
  NodeWeakPtr pf;  ///<父节点
  NodePtr pl;      ///<左子节点
  NodePtr pr;      ///<右子节点
};

/**
 * @brief 二叉查找树
 * @note 与BinTreeNode类似，但模板参数T需要支持比较运算
 * @tparam T 实际节点类型，需要支持比较运算
 */
template <typename T>
class BinSearchTreeNode : public std::enable_shared_from_this<BinSearchTreeNode<T> > {
 public:
  typedef std::shared_ptr<BinSearchTreeNode<T> > NodePtr;
  typedef std::weak_ptr<BinSearchTreeNode<T> > NodeWeakPtr;

 public:
  BinSearchTreeNode() {}
  explicit BinSearchTreeNode(const T& input_obj) : obj(input_obj) {}

  ///向当前节点为根节点的二叉查找树中插入一个节点
  void Insert(NodePtr node) {
    if (!node) return;

    if (node->obj < obj) {
      if (pl) {
        pl->Insert(node);
      } else {
        SetLChild(this->shared_from_this(), node);
      }
    } else {
      if (pr) {
        pr->Insert(node);
      } else {
        SetRChild(this->shared_from_this(), node);
      }
    }
  }

  ///删除当前节点，并返回替代的节点
  NodePtr Erase() {
    if (!pl && !pr) {
      //左右都为空，为叶子节点
      BreakFather(this->shared_from_this());
      return NodePtr();
    }
    auto real_pf = pf.lock();
    if (pl && !pr) {
      //只有左子树
      NodePtr re = pl;
      if (!real_pf) {
        BreakLChild(this->shared_from_this());
      } else {
        pl->pf = real_pf;
        real_pf->pl = pl;
        pf.reset();
        pl.reset();
      }
      return re;
    }
    if (!pl && pr) {
      //只有右子树
      NodePtr re = pr;
      if (!real_pf) {
        BreakRChild(this->shared_from_this());
      } else {
        pr->pf = real_pf;
        real_pf->pr = pr;
        pf.reset();
        pr.reset();
      }
      return re;
    }
    //换左子树的前驱
    NodePtr re = pl;
    if (re->pr) {
      //左子节点有右子树，找到其前驱
      while (re->pr) re = re->pr;
      auto re_pf = re->pf.lock();
      re_pf->pr = re->pl;
      if (re->pl) re->pl->pf = re_pf;
      re->pl = pl;
      pl->pf = re;
    }
    re->pf = real_pf;
    re->pr = pr;
    pr->pf = re;
    if (real_pf) {
      if (GetLR(this->shared_from_this())) {
        real_pf->pl = re;
      } else {
        real_pf->pr = re;
      }
    }
    pf.reset();
    pl.reset();
    pr.reset();
    return re;
  }

 public:
  T obj;           ///<实际节点值
  NodeWeakPtr pf;  ///<父节点
  NodePtr pl;      ///<左子节点
  NodePtr pr;      ///<右子节点
};

/**
 * @brief AVL树(todo待完善)
 * @note 二叉平衡树。模板参数T需要支持比较运算。节点obj值不能重复
 * @tparam T 实际节点类型，需要支持比较运算
 */
template <typename T>
class AVLTreeNode : public std::enable_shared_from_this<AVLTreeNode<T> > {
 public:
  typedef std::shared_ptr<AVLTreeNode<T> > NodePtr;
  typedef std::weak_ptr<AVLTreeNode<T> > NodeWeakPtr;

 public:
  AVLTreeNode() {}
  explicit AVLTreeNode(const T& input_obj) : obj(input_obj) {}

  ///获取节点hgt值
  static size_t GetHgt(const NodePtr& p) {
    return (p ? (p->hgt) : 0);
  }

  ///刷新节点hgt值
  void RefreshHgt() {
    hgt = std::max(GetHgt(pl), GetHgt(pr)) + 1;
  }

  ///插入，必须是根节点才能进行此操作。因为根节点可能会变，所以返回插入后的新根节点
  NodePtr Insert(NodePtr node) {
    if (!pf.expired())
      throw std::logic_error("Must insert to a root node.");

    if (!node)
      return this->shared_from_this();

    //找到最终要插入的地方的父节点
    NodePtr pos;
    NodePtr tmppos = this->shared_from_this();
    do {
      pos = tmppos;
      if (pos->obj == node->obj) {
        return this->shared_from_this();
      }
      tmppos = (node->obj < pos->obj) ? (pos->pl) : (pos->pr);
    } while (tmppos);

    if (node->obj < pos->obj) {
      SetLChild(pos, node);
    } else {
      SetRChild(pos, node);
    }
    node->hgt = 1;

    //更新高度，进行旋转
    NodePtr re;
    NodePtr end = pf.lock();
    while (pos != end) {
      re.reset();
      size_t curhgt = pos->hgt;
      size_t lh = GetHgt(pos->pl), lr = GetHgt(pos->pr);
      if (lh >= lr + 2) {
        //左边比右边高了2
        if (GetHgt(pos->pl->pl) >= GetHgt(pos->pl->pr)) {
          re = pos->RotateL();
        } else {
          pos->pl->RotateR();
          re = pos->RotateL();
        }
      } else if (lr >= lh + 2) {
        //右边比左边高了2
        if (GetHgt(pos->pr->pr) >= GetHgt(pos->pr->pl)) {
          re = pos->RotateR();
        } else {
          pos->pr->RotateL();
          re = pos->RotateR();
        }
      }
      //如果发生旋转了，说明之前左右高度相差2，说明该节点高度一定发生改变
      size_t cghgt;
      if (re) {
        cghgt = re->hgt;
        pos = re->pf.lock();
      } else {
        cghgt = pos->hgt = std::max(lh, lr) + 1;
        if (curhgt == cghgt) return this->shared_from_this();
        pos = pos->pf.lock();
      }
    }
    if (re) return re;
    return this->shared_from_this();
  }

  ///在当前节点为根节点的树中删除一个节点，并返回删除后的根节点
  NodePtr Erase(NodePtr node) {
    if (!pf.expired())
      throw std::logic_error("Must erase from a root node.");

    if (!node) return this->shared_from_this();

    //先确定要删除的节点是自己的子节点
    if (GetRootNode(node) != this->shared_from_this())
      throw std::logic_error("Node must belong to root node.");

    return EraseImp(node);
  }
  ///在当前节点中删除节点值为val的节点。会先搜索再删除
  NodePtr Erase(const T& val) {
    if (!pf.expired())
      throw std::logic_error("Must erase from a root node.");

    NodePtr node = BinSearch<AVLTreeNode<T>, T>(this->shared_from_this(), val);
    if (!node) return this->shared_from_this();

    return EraseImp(node);
  }

 private:
  ///假如有重复的，删除第一个找到的
  NodePtr EraseImp(NodePtr node) {
    //调用者保证自身是根节点，node不为空且属于自身节点所在树
    NodePtr root_node = this->shared_from_this();  //如果要删除的是自身，则需要一个指针来保存root节点
    NodePtr node_pf = node->pf.lock();
    NodePtr pos = node_pf;
    if (!(node->pl) && !(node->pr)) {
      //左右都为空，为叶子节点
      if (node_pf) {
        BreakFather(node);
      } else {
        //整个树就一个要删除的根节点
        return NodePtr();
      }
    } else if (node->pl && !(node->pr)) {
      //只有左子树
      if (!node_pf) {
        root_node = node->pl;
        BreakLChild(node);
      } else {
        node->pl->pf = node_pf;
        node_pf->pl = node->pl;
        node->pf.reset();
        node->pl.reset();
      }
    } else if (!(node->pl) && node->pr) {
      //只有右子树
      if (!node_pf) {
        root_node = node->pr;
        BreakRChild(node);
      } else {
        node->pr->pf = node_pf;
        node_pf->pr = node->pr;
        node->pf.reset();
        node->pr.reset();
      }
    } else {
      //换左子树的前驱
      NodePtr tmp = node->pl;
      if (tmp->pr) {
        //左子节点有右子树，找到其前驱
        while (tmp->pr) tmp = tmp->pr;
        NodePtr tmp_pf = tmp->pf.lock();
        tmp_pf->pr = tmp->pl;
        if (tmp->pl) tmp->pl->pf = tmp_pf;
        tmp->pl = node->pl;
        node->pl->pf = tmp;
        pos = tmp_pf;
      } else {
        pos = tmp;
      }

      tmp->pf = node_pf;
      tmp->pr = node->pr;
      node->pr->pf = tmp;
      if (node_pf) {
        if (GetLR(node))
          node_pf->pl = tmp;
        else
          node_pf->pr = tmp;
      } else
        root_node = tmp;
      node->pf.reset();
      node->pl.reset();
      node->pr.reset();
      tmp->hgt = node->hgt;
    }
    //更新高度，进行旋转
    NodePtr re;
    while (pos != NULL) {
      re.reset();
      size_t curhgt = pos->hgt;
      size_t lh = GetHgt(pos->pl), lr = GetHgt(pos->pr);
      if (lh >= lr + 2) {
        //左边比右边高了2
        if (GetHgt(pos->pl->pl) >= GetHgt(pos->pl->pr))
          re = pos->RotateL();
        else {
          pos->pl->RotateR();
          re = pos->RotateL();
        }
      } else if (lr >= lh + 2) {
        //右边比左边高了2
        if (GetHgt(pos->pr->pr) >= GetHgt(pos->pr->pl))
          re = pos->RotateR();
        else {
          pos->pr->RotateL();
          re = pos->RotateR();
        }
      }
      //如果发生旋转了，说明之前左右高度相差2，说明该节点高度一定发生改变
      size_t cghgt;
      if (re) {
        cghgt = re->hgt;
        pos = re->pf.lock();
      } else {
        cghgt = pos->hgt = std::max(lh, lr) + 1;
        if (curhgt == cghgt) break;
        pos = pos->pf.lock();
      }
    }
    if (re) return re;
    return root_node;
  }

  ///左旋转，顺时针
  NodePtr RotateL() {
    NodePtr re = pl;
    if (re->pr) re->pr->pf = this->shared_from_this();
    pl = re->pr;

    NodePtr real_pf = pf.lock();
    if (!real_pf) {
      re->pr = this->shared_from_this();
    } else {
      if (GetLR(this->shared_from_this())) {
        re->pr = real_pf->pl;
        real_pf->pl = re;
      } else {
        re->pr = real_pf->pr;
        real_pf->pr = re;
      }
    }
    re->pf = real_pf;
    pf = re;
    RefreshHgt();
    re->RefreshHgt();
    return re;
  }
  ///右旋转，逆时针
  NodePtr RotateR() {
    NodePtr re = pr;
    if (re->pl) re->pl->pf = this->shared_from_this();
    pr = re->pl;

    NodePtr real_pf = pf.lock();
    if (!real_pf) {
      re->pl = this->shared_from_this();
    } else {
      if (GetLR(this->shared_from_this())) {
        re->pl = real_pf->pl;
        real_pf->pl = re;
      } else {
        re->pl = real_pf->pr;
        real_pf->pr = re;
      }
    }
    re->pf = real_pf;
    pf = re;
    RefreshHgt();
    re->RefreshHgt();
    return re;
  }

 public:
  T obj;           ///<实际节点值
  NodeWeakPtr pf;  ///<父节点
  NodePtr pl;      ///<左子节点
  NodePtr pr;      ///<右子节点

  size_t hgt = 1;  ///<节点高度
};

/**
 * @brief 红黑树(todo待完善)
 * @note 二叉平衡树。模板参数T需要支持比较运算
 * @tparam T 实际节点类型，需要支持比较运算
 */
template <typename T>
class BRTreeNode : public std::enable_shared_from_this<BRTreeNode<T> > {
 public:
  typedef std::shared_ptr<BRTreeNode<T> > NodePtr;
  typedef std::weak_ptr<BRTreeNode<T> > NodeWeakPtr;

 public:
  BRTreeNode() {}
  explicit BRTreeNode(const T& input_obj) : obj(input_obj) {}

  ///插入，必须是根节点才能进行此操作。因为根节点可能会变，所以返回插入后的新根节点
  NodePtr Insert(NodePtr node) {
    if (!pf.expired())
      throw std::logic_error("Must insert to a root node.");

    if (!node)
      return this->shared_from_this();

    //找到最终要插入的地方的父节点
    BRTreeNode<T>*pos = this, *tmppos = (node->obj < obj) ? pl.get() : pr.get();
    while (tmppos != NULL) {
      pos = tmppos;
      tmppos = (node->obj < pos->obj) ? pos->pl.get() : pos->pr.get();
    }
    if (node->obj == pos->obj) return this->shared_from_this();  //不允许重复
    if (node->obj < pos->obj)
      SetLChild(pos, node);
    else
      SetRChild(pos, node);

    //插入节点的颜色总是红色
    node->color = true;

    NodePtr re;
    BRTreeNode<T>* end = pf;
    tmppos = node.get();
    while (pos != end) {
      re.reset();
      //父节点是黑色
      if (!(pos->color)) {
        return this->shared_from_this();
      }
      BRTreeNode<T>* uncle = (GetLR(pos) ? pos->pf->pr.get() : pos->pf->pl.get());
      if (uncle != NULL && uncle->color) {
        //插入节点的父节点和其叔叔节点均为红色的
        pos->color = uncle->color = false;
        pos->pf->color = true;
        tmppos = pos->pf;
        pos = tmppos->pf;
      } else {
        //插入节点的父节点是红色，叔叔节点是黑色
        if (GetLR(pos)) {
          //父节点是祖父节点的左支
          if (GetLR(tmppos)) {
            //插入节点是其父节点的左子节点
            pos->color = false;
            pos->pf->color = true;
            re = pos->pf->RotateL();
            break;
          } else {
            //插入节点是其父节点的右子节点
            pos->RotateR();
            tmppos = pos;
            pos = tmppos->pf;
          }
        } else {
          //父节点是祖父节点的右支
          if (GetLR(tmppos)) {
            //插入节点是其父节点的左子节点
            pos->RotateL();
            tmppos = pos;
            pos = tmppos->pf;
          } else {
            //插入节点是其父节点的右子节点
            pos->color = false;
            pos->pf->color = true;
            re = pos->pf->RotateR();
            break;
          }
        }
      }
    }
    if (pos == end) tmppos->color = false;
    if (re && (re->pf == end)) return re;
    return this->shared_from_this();
  }

  ///在当前节点为根节点的树中删除一个节点，并返回删除后的根节点
  NodePtr Erase(NodePtr node) {
    if (!pf.expired())
      throw std::logic_error("Must erase from a root node.");

    if (!node) return this->shared_from_this();

    //先确定要删除的节点是自己的子节点
    if (GetRootNode(node) != this->shared_from_this())
      throw std::logic_error("Node must belong to root node.");

    return EraseImp(node);
  }
  ///在当前节点中删除节点值为val的节点。会先搜索再删除
  NodePtr Erase(const T& val) {
    if (!pf.expired())
      throw std::logic_error("Must erase from a root node.");

    NodePtr node = BinSearch<BRTreeNode<T>, T>(this->shared_from_this(), val);
    if (!node) return this->shared_from_this();

    return EraseImp(node);
  }

 private:
  NodePtr EraseImp(NodePtr node) {
    //调用者保证自身是根节点，node不为空且属于自身节点所在树
    if (!node) return this->shared_from_this();
    NodePtr root_node = this->shared_from_this();  //如果要删除的是自身，则需要一个指针来保存root节点
    BRTreeNode<T>* pos = node->pf;
    if (!(node->pl) && !(node->pr)) {
      //左右都为空，为叶子节点
      if (node->pf != NULL) {
        if (GetLR(node.get()))
          BreakLChild(node->pf);
        else
          BreakRChild(node->pf);
      } else {
        //整个树就一个要删除的根节点
        return NodePtr();
      }
    } else if (node->pl && !(node->pr)) {
      //只有左子树
      if (node->pf == NULL) {
        root_node = node->pl;
        BreakLChild(node.get());
      } else {
        node->pl->pf = node->pf;
        node->pf->pl = node->pl;
        node->pf = NULL;
        node->pl.reset();
      }
    } else if (!(node->pl) && node->pr) {
      //只有右子树
      if (node->pf == NULL) {
        root_node = node->pr;
        BreakRChild(node.get());
      } else {
        node->pr->pf = node->pf;
        node->pf->pr = node->pr;
        node->pf = NULL;
        node->pr.reset();
      }
    } else {
      //换左子树的前驱
      NodePtr tmp = node->pl;
      if (tmp->pr) {
        //左子节点有右子树，找到其前驱
        while (tmp->pr) tmp = tmp->pr;
        tmp->pf->pr = tmp->pl;
        if (tmp->pl) tmp->pl->pf = tmp->pf;
        tmp->pl = node->pl;
        node->pl->pf = tmp.get();
        pos = tmp->pf;
      } else
        pos = tmp.get();
      tmp->pf = node->pf;
      tmp->pr = node->pr;
      node->pr->pf = tmp.get();
      if (node->pf != NULL) {
        if (GetLR(node.get()))
          node->pf->pl = tmp;
        else
          node->pf->pr = tmp;
      } else
        root_node = tmp;
      node->pf = NULL;
      node->pl.reset();
      node->pr.reset();
      tmp->hgt = node->hgt;
    }
  }
  ///<左旋转，顺时针
  NodePtr RotateL() {
    NodePtr re = pl;
    if (re->pr) re->pr->pf = this;
    pl = re->pr;
    if (pf == NULL)
      re->pr = this->shared_from_this();
    else {
      if (GetLR(this)) {
        re->pr = pf->pl;
        pf->pl = re;
      } else {
        re->pr = pf->pr;
        pf->pr = re;
      }
    }
    re->pf = pf;
    pf = re.get();
    return re;
  }
  ///<右旋转，逆时针
  NodePtr RotateR() {
    NodePtr re = pr;
    if (re->pl) re->pl->pf = this;
    pr = re->pl;
    if (pf == NULL)
      re->pl = this->shared_from_this();
    else {
      if (GetLR(this)) {
        re->pl = pf->pl;
        pf->pl = re;
      } else {
        re->pl = pf->pr;
        pf->pr = re;
      }
    }
    re->pf = pf;
    pf = re.get();
    return re;
  }

 public:
  T obj;           ///<实际节点值
  NodeWeakPtr pf;  ///<父节点
  NodePtr pl;      ///<左子节点
  NodePtr pr;      ///<右子节点

  bool color = false;  ///<颜色，true为红，false为黑
};

/**
 * @brief 检查子节点是否为目标节点的左子节点
 * 
 * @tparam T 
 * @param target_node 
 * @param child_node 
 * @return true child_node是target_node左子节点
 * @return false child_node不是target_node左子节点
 */
template <typename T>
bool CheckLChild(const std::shared_ptr<T>& target_node, const std::shared_ptr<T>& child_node) {
  return (target_node && child_node && (target_node->pl == child_node) && (child_node->pf.lock() == target_node));
}

/**
 * @brief 检查子节点是否为目标节点的右子节点
 * 
 * @tparam T 
 * @param target_node 
 * @param child_node 
 * @return true child_node是target_node右子节点
 * @return false child_node不是target_node右子节点
 */
template <typename T>
bool CheckRChild(std::shared_ptr<T> target_node, std::shared_ptr<T> child_node) {
  return (target_node && child_node && (target_node->pr == child_node) && (child_node->pf.lock() == target_node));
}

/**
 * @brief 判断是父节点的左节点还是右节点
 * @note 如果既不是左节点也不是右节点，将抛出异常
 * @tparam T 节点类型
 * @param target_node 目标节点
 * @return true 目标节点是其父节点的左节点
 * @return false 目标节点是其父节点的右节点
 */
template <typename T>
bool GetLR(const std::shared_ptr<T> target_node) {
  if (!target_node) throw std::logic_error("target_node can not be empty.");

  auto target_pf = target_node->pf.lock();
  if (!target_pf) throw std::logic_error("target->pf can not be empty.");

  if (target_node == target_pf->pl) return true;
  if (target_node == target_pf->pr) return false;
  throw std::logic_error("Invalid target_node.");

  return true;
}

/**
 * @brief 与左子树断开
 * 
 * @tparam T 节点类型
 * @param target_node 待处理节点
 */
template <typename T>
void BreakLChild(std::shared_ptr<T> target_node) {
  if (!target_node || !(target_node->pl)) return;
  target_node->pl->pf.reset();
  target_node->pl.reset();
}

/**
 * @brief 与右子树断开
 * 
 * @tparam T 节点类型
 * @param target_node 待处理节点
 */
template <typename T>
void BreakRChild(std::shared_ptr<T> target_node) {
  if (!target_node || !(target_node->pr)) return;
  target_node->pr->pf.reset();
  target_node->pr.reset();
}

/**
 * @brief 与父节点断开
 * 
 * @tparam T 
 * @param target_node 待处理节点
 */
template <typename T>
void BreakFather(std::shared_ptr<T> target_node) {
  if (!target_node) return;

  auto target_node_pf = target_node->pf.lock();
  if (!target_node_pf) return;

  target_node->pf.reset();

  if (target_node_pf->pl == target_node)
    target_node_pf->pl.reset();
  else if (target_node_pf->pr == target_node)
    target_node_pf->pr.reset();
}

/**
 * @brief 设置左子节点
 * @note 将一个节点作为左子节点，与原左子节点断开。插入的节点与其原父节点断开
 * @tparam T 节点类型
 * @param target_node 父节点shared_ptr
 * @param child_node 子节点shared_ptr
 */
template <typename T>
void SetLChild(std::shared_ptr<T> target_node, std::shared_ptr<T> child_node) {
  if (!target_node) throw std::logic_error("target_node can not be empty.");
  if (target_node == child_node) throw std::logic_error("Can not set child to self.");

  if (!child_node) {
    if (target_node->pl) {
      target_node->pl->pf.reset();
      target_node->pl.reset();
    }
    return;
  }

  auto child_node_pf = child_node->pf.lock();
  if ((target_node->pl == child_node) && (child_node_pf == target_node)) return;

  if (target_node->pl) {
    target_node->pl->pf.reset();
  }

  if (child_node_pf) {
    if (child_node_pf->pl == child_node)
      child_node_pf->pl.reset();
    else if (child_node_pf->pr == child_node)
      child_node_pf->pr.reset();
  }

  target_node->pl = child_node;
  if (child_node_pf != target_node) {
    child_node->pf = target_node;
  }
}

/**
 * @brief 设置右子节点
 * @note 将一个节点作为右子节点，与原右子节点断开。插入的节点与其原父节点断开
 * @tparam T 节点类型
 * @param target_node 父节点shared_ptr
 * @param child_node 子节点shared_ptr
 */
template <typename T>
void SetRChild(std::shared_ptr<T> target_node, std::shared_ptr<T> child_node) {
  if (!target_node) throw std::logic_error("target_node can not be empty.");
  if (target_node == child_node) throw std::logic_error("Can not set child to self.");

  if (!child_node) {
    if (target_node->pr) {
      target_node->pr->pf.reset();
      target_node->pr.reset();
    }
    return;
  }

  auto child_node_pf = child_node->pf.lock();
  if ((target_node->pr == child_node) && (child_node_pf == target_node)) return;

  if (target_node->pr) {
    target_node->pr->pf.reset();
  }

  if (child_node_pf) {
    if (child_node_pf->pl == child_node)
      child_node_pf->pl.reset();
    else if (child_node_pf->pr == child_node)
      child_node_pf->pr.reset();
  }

  target_node->pr = child_node;
  if (child_node_pf != target_node) {
    child_node->pf = target_node;
  }
}

/**
 * @brief 二叉树序列化
 * @note 根据前序遍历进行的二叉树序列化，存储为vector<pair<bool,T> >
 * @tparam NodeType 节点类型
 * @tparam ValType 节点obj类型
 * @param[in] root_node 待序列化的树的根节点
 * @param[out] vec 要返回的序列化结果
 */
template <typename NodeType, typename ValType>
void SerializeTree(const std::shared_ptr<NodeType>& root_node, std::vector<std::pair<bool, ValType> >& vec) {
  if (root_node) {
    vec.emplace_back(true, root_node->obj);
    SerializeTree<NodeType, ValType>(root_node->pl, vec);
    SerializeTree<NodeType, ValType>(root_node->pr, vec);
  } else {
    vec.emplace_back(false, ValType());
  }
}

/**
 * @brief 二叉树反序列化
 * @note 根据前序遍历进行的二叉树反序列化，根据vector<pair<bool,T> >反序列化
 * @tparam NodeType 节点类型
 * @tparam ValType 节点obj类型
 * @param[out] root_node 待存放反序列化结果的树的根节点
 * @param[in] vec 根据其存储的数据进行反序列化
 */
template <typename NodeType, typename ValType>
void DeserializeTree(std::shared_ptr<NodeType>& root_node, const std::vector<std::pair<bool, ValType> >& vec) {
  typename std::vector<std::pair<bool, ValType> >::const_iterator itr = vec.begin();
  DeserializeTree<NodeType, ValType>(root_node, itr);
}

/**
 * @brief 二叉树反序列化
 * @note 根据前序遍历进行的二叉树反序列化，根据vector<pair<bool,T> >::const_iterator反序列化
 * @tparam NodeType 节点类型
 * @tparam ValType 节点obj类型
 * @param[out] root_node 待存放反序列化结果的树的根节点
 * @param[in] itr 指向要进行反序列化的vector开头的迭代器
 */
template <typename NodeType, typename ValType>
void DeserializeTree(std::shared_ptr<NodeType>& root_node, typename std::vector<std::pair<bool, ValType> >::const_iterator& itr) {
  if (itr->first) {
    root_node = std::make_shared<NodeType>(itr->second);
    DeserializeTree<NodeType, ValType>(root_node->pl, ++itr);
    if (root_node->pl) root_node->pl->pf = root_node;
    DeserializeTree<NodeType, ValType>(root_node->pr, ++itr);
    if (root_node->pr) root_node->pr->pf = root_node;
  }
}

/**
 * @brief 前序遍历
 * @note 以当前节点为根节点，递归前序遍历，返回一个指针数组。中-左-右
 * @tparam T 节点类型
 * @param[in] node 要前序遍历的根节点
 * @param[out] vec 指针数组，要返回的遍历结果
 */
template <typename T>
void DLR(const std::shared_ptr<T>& node, std::vector<std::shared_ptr<T> >& vec) {
  if (!node) return;
  vec.emplace_back(node);
  DLR(node->pl, vec);
  DLR(node->pr, vec);
}

/**
 * @brief 中序遍历
 * @note 以当前节点为根节点，递归中序遍历，返回一个指针数组。左-中-右
 * @tparam T 节点类型
 * @param[in] node 要遍历的根节点
 * @param[out] vec 指针数组，要返回的遍历结果
 */
template <typename T>
void LDR(const std::shared_ptr<T>& node, std::vector<std::shared_ptr<T> >& vec) {
  if (!node) return;
  LDR(node->pl, vec);
  vec.emplace_back(node);
  LDR(node->pr, vec);
}

/**
 * @brief 后序遍历
 * @note 以当前节点为根节点，递归后序遍历，返回一个指针数组。左-右-中
 * @tparam T 节点类型
 * @param[in] node 要遍历的根节点
 * @param[out] vec 指针数组，要返回的遍历结果
 */
template <typename T>
void LRD(const std::shared_ptr<T>& node, std::vector<std::shared_ptr<T> >& vec) {
  if (!node) return;
  LRD(node->pl, vec);
  LRD(node->pr, vec);
  vec.emplace_back(node);
}

/**
 * @brief 分层遍历
 * @note 分层遍历二叉树，将结果输出到vector数组中
 * @tparam T 节点类型
 * @param[in] node 待分层遍历的树的根节点
 * @param[out] vec 要返回的遍历结果
 */
template <typename T>
void TraByLevel(const std::shared_ptr<T>& node, std::vector<std::shared_ptr<T> >& vec) {
  if (!node) return;
  size_t pos1 = vec.size(), pos2;
  vec.emplace_back(node);
  while (pos1 < vec.size()) {
    pos2 = vec.size();
    while (pos1 < pos2) {
      if (vec[pos1]->pl) vec.emplace_back(vec[pos1]->pl);
      if (vec[pos1]->pr) vec.emplace_back(vec[pos1]->pr);
      ++pos1;
    }
  }
}

/**
 * @brief 获取一个节点的深度
 * @note 获取一个节点的深度，根节点深度为0，空节点深度为0
 * @tparam T 节点类型
 * @param node 要获取深度的节点
 * @return size_t 节点深度
 */
template <typename T>
size_t GetDepth(const std::shared_ptr<T>& node) {
  if (!node) return 0;

  std::shared_ptr<T> itr_node = node->pf.lock();
  size_t count = 0;
  while (itr_node) {
    ++count;
    itr_node = itr_node->pf.lock();
  }
  return count;
}

/**
 * @brief 获取一个节点的高度
 * @note 获取一个节点的高度，叶子节点高度为1，空节点高度为0
 * @tparam T 节点类型
 * @param node 要获取高度的节点
 * @return size_t 节点高度
 */
template <typename T>
size_t GetHeight(const std::shared_ptr<T>& node) {
  if (!node) return 0;
  return std::max(GetHeight(node->pl), GetHeight(node->pr)) + 1;
}

/**
 * @brief 获取一个树中最长根-叶链长度
 * @note 获取一个树中从根节点到叶子节点的最长节点个数。实际调用GetHeight
 * @tparam T 节点类型
 * @param node 要获取最长链长度的树的根节点
 * @return size_t 最长链长度
 */
template <typename T>
size_t GetMaxChain(const std::shared_ptr<T>& node) {
  return GetHeight(node);
}

/**
 * @brief 获取一个树中最短根-叶链长度
 * @note 获取一个树中从根节点到叶子节点的最短节点个数
 * @tparam T 节点类型
 * @param node 要获取最短链长度的树的根节点
 * @return size_t 最短链长度
 */
template <typename T>
size_t GetMinChain(const std::shared_ptr<T>& node) {
  if (!node) return 0;
  if (!node->pl)
    return GetMinChain(node->pr) + 1;
  else if (!node->pr)
    return GetMinChain(node->pl) + 1;
  else
    return std::min(GetMinChain(node->pl), GetMinChain(node->pr)) + 1;
}

/**
 * @brief 获取树中节点个数
 * 
 * @tparam T 节点类型
 * @param node 要获取节点个数的树的根节点
 * @return size_t 节点个数
 */
template <typename T>
size_t GetNodeNum(const std::shared_ptr<T>& node) {
  if (!node) return 0;
  return GetNodeNum(node->pl) + GetNodeNum(node->pr) + 1;
}

/**
 * @brief 获取所在树的根节点
 * 
 * @tparam T 
 * @param node 
 * @return std::shared_ptr<T> 
 */
template <typename T>
std::shared_ptr<T> GetRootNode(const std::shared_ptr<T>& node) {
  if (!node) return std::shared_ptr<T>();
  std::shared_ptr<T> re = node;
  std::shared_ptr<T> re_pf = re->pf.lock();
  while (re_pf) {
    re = re_pf;
    re_pf = re->pf.lock();
  }
  return re;
}

/**
 * @brief 二叉树深拷贝
 * @note 对二叉树进行深拷贝，可以在不同类型之间拷贝
 * @tparam NodeTypeFrom 
 * @tparam NodeTypeTo 
 * @param[in] root_node_from 
 * @param[out] root_node_to 
 */
template <typename NodeTypeFrom, typename NodeTypeTo>
void CopyTree(const std::shared_ptr<NodeTypeFrom>& root_node_from, std::shared_ptr<NodeTypeTo>& root_node_to) {
  if (!root_node_from) return;
  root_node_to = std::make_shared<NodeTypeTo>(root_node_from->obj);
  CopyTree(root_node_from->pl, root_node_to->pl);
  if (root_node_to->pl) root_node_to->pl->pf = root_node_to;
  CopyTree(root_node_from->pr, root_node_to->pr);
  if (root_node_to->pr) root_node_to->pr->pf = root_node_to;
}

/**
 * @brief 比较两个二叉树的结构和obj是否相等
 * 
 * @tparam T 
 * @param root_node1 
 * @param root_node2 
 * @return true 
 * @return false 
 */
template <typename T>
bool CompareTreeObj(const std::shared_ptr<T>& root_node1, const std::shared_ptr<T>& root_node2) {
  if (!root_node1 && !root_node2) return true;
  if (!root_node1 || !root_node2) return false;

  if (root_node1->obj != root_node2->obj) return false;
  if (!CompareTreeObj(root_node1->pl, root_node2->pl)) return false;
  if (!CompareTreeObj(root_node1->pr, root_node2->pr)) return false;
  return true;
}

/**
 * @brief 在二叉搜索树中进行查找
 * 
 * @tparam NodeType 节点类型
 * @tparam ValType 节点obj类型
 * @param root_node 
 * @param val 
 * @return std::shared_ptr<NodeType> 查找到的节点的智能指针，若没有查到则返回空智能指针
 */
template <typename NodeType, typename ValType>
std::shared_ptr<NodeType> BinSearch(const std::shared_ptr<NodeType>& root_node, const ValType& val) {
  std::shared_ptr<NodeType> p = root_node;
  while (p) {
    if (p->obj == val) return p;
    p = (val < p->obj) ? (p->pl) : (p->pr);
  }
  return std::shared_ptr<NodeType>();
}

/**
 * @brief 检查是否为二叉树
 * @note 检查一颗二叉树是否合法，子节点、父节点是否对应相连
 * @tparam T 节点类型
 * @param root_node 待检查的树的根节点
 * @return true 合法
 * @return false 不合法
 */
template <typename T>
bool CheckBinTree(const std::shared_ptr<T>& root_node) {
  if (root_node) {
    if (root_node->pl) {
      if (root_node->pl->pf.lock() != root_node) return false;
      if (!CheckBinTree(root_node->pl)) return false;
    }
    if (root_node->pr) {
      if (root_node->pr->pf.lock() != root_node) return false;
      if (!CheckBinTree(root_node->pr)) return false;
    }
  }
  return true;
}

/**
 * @brief 检查是否为搜索二叉树
 * @note 检查一颗二叉树是否为搜索二叉树，要求左<中<=右
 * @tparam T 节点类型
 * @param root_node 待检查的树的根节点
 * @return true 是搜索二叉树
 * @return false 不是搜索二叉树
 */
template <typename T>
bool CheckBinSearchTree(const std::shared_ptr<T>& root_node) {
  if (root_node) {
    if (root_node->pl) {
      if (root_node->pl->pf.lock() != root_node) return false;
      if (root_node->pl->obj >= root_node->obj) return false;
      if (!CheckBinSearchTree(root_node->pl)) return false;
    }
    if (root_node->pr) {
      if (root_node->pr->pf.lock() != root_node) return false;
      if (root_node->pr->obj < root_node->obj) return false;
      if (!CheckBinSearchTree(root_node->pr)) return false;
    }
  }
  return true;
}

/**
 * @brief 检查是否为AVL树
 * @note 要求各叶子节点深度相差<=1
 * @tparam T 节点类型
 * @param root_node 待检查的树的根节点
 * @return true 是AVL树
 * @return false 不是AVL树
 */
template <typename T>
bool CheckAVLTree(const std::shared_ptr<T>& root_node) {
  if (!CheckBinSearchTree(root_node)) return false;
  if (GetMaxChain(root_node) > (GetMinChain(root_node) + 1)) return false;
  return true;
}

/**
 * @brief 检查是否为红黑树(todo待完善)
 * @note 检查一颗二叉树是否为红黑树，要求根节点黑、没有连续红节点、所有路径有相同数目黑节点
 * @tparam T 节点类型
 * @param root_node 待检查的树的根节点
 * @return true 是红黑树
 * @return false 不是红黑树
 */
template <typename T>
bool CheckBRTree(const std::shared_ptr<T>& root_node) {
  if (!CheckBinSearchTree(root_node)) return false;
  if (root_node->color != false) return false;  //根节点要为黑

  return true;
}

}  // namespace ytlib
