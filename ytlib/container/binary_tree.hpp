/**
 * @file binary_tree.hpp
 * @brief 二叉树
 * @note 提供模板化的一些树的实现和相关的算法，包括AVL树、红黑树。不能使用继承，因为有成员是自身类型的智能指针
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>

namespace ytlib {

/**
 * @brief 二叉树
 * 模板参数T为实际节点值。
 * 一个BinTreeNode实例表示一个二叉树节点，也表示以此节点为根节点的一棵二叉树。
 * 如果根节点被析构，那么整个树中所有子节点将被析构，除非子节点有另外的智能指针指着
 */
template <typename T>
class BinTreeNode {
 private:
  typedef std::shared_ptr<BinTreeNode<T> > nodePtr;

 public:
  BinTreeNode() : pf(NULL) {}
  explicit BinTreeNode(const T& _obj) : obj(_obj), pf(NULL) {}

  T obj;               ///<实际节点值
  BinTreeNode<T>* pf;  ///<父节点。父节点不可使用智能指针，否则会造成循环引用
  nodePtr pl;          ///<左子节点
  nodePtr pr;          ///<右子节点
};
/**
 * @brief 二叉查找树
 * 与BinTreeNode类似，但模板参数T需要支持比较运算
 */
template <typename T>
class BinSearchTreeNode {
 private:
  typedef std::shared_ptr<BinSearchTreeNode<T> > BSTNodePtr;

 public:
  BinSearchTreeNode() : pf(NULL) {}
  explicit BinSearchTreeNode(const T& _obj) : obj(_obj), pf(NULL) {}

  T obj;                     ///<实际节点值
  BinSearchTreeNode<T>* pf;  ///<父节点。父节点不可使用智能指针，否则会造成循环引用
  BSTNodePtr pl;             ///<左子节点
  BSTNodePtr pr;             ///<右子节点

  ///向当前节点为根节点的二叉查找树中插入一个节点
  void insert(BSTNodePtr ndptr) {
    assert(ndptr);
    if (ndptr->obj < obj) {
      if (pl)
        pl->insert(ndptr);
      else
        setLChild(this, ndptr);
    } else {
      if (pr)
        pr->insert(ndptr);
      else
        setRChild(this, ndptr);
    }
  }

  ///删除当前节点，并返回替代的节点
  BSTNodePtr erase() {
    if (!pl && !pr) {
      //左右都为空，为叶子节点
      if (pf != NULL) {
        if (getLR(this))
          breakLChild(pf);
        else
          breakRChild(pf);
      }
      return BSTNodePtr();
    }
    if (pl && !pr) {
      //只有左子树
      BSTNodePtr re = pl;
      if (pf == NULL)
        breakLChild(this);
      else {
        pl->pf = pf;
        pf->pl = pl;
        pf = NULL;
        pl.reset();
      }
      return re;
    }
    if (!pl && pr) {
      //只有右子树
      BSTNodePtr re = pr;
      if (pf == NULL)
        breakRChild(this);
      else {
        pr->pf = pf;
        pf->pr = pr;
        pf = NULL;
        pr.reset();
      }
      return re;
    }
    //换左子树的前驱
    BSTNodePtr tmp = pl;
    if (tmp->pr) {
      //左子节点有右子树，找到其前驱
      while (tmp->pr) tmp = tmp->pr;
      tmp->pf->pr = tmp->pl;
      if (tmp->pl) tmp->pl->pf = tmp->pf;
      tmp->pl = pl;
      pl->pf = tmp.get();
    }
    tmp->pf = pf;
    tmp->pr = pr;
    pr->pf = tmp.get();
    if (pf != NULL) {
      if (getLR(this))
        pf->pl = tmp;
      else
        pf->pr = tmp;
    }
    pf = NULL;
    pl.reset();
    pr.reset();
    return tmp;
  }
};

/**
 * @brief 在二叉搜索树中进行查找
 * @note 使用递归方法
 * @param NodeType 模板参数，节点类型
 * @param ValType 模板参数，节点obj类型
 * @return 查找到的节点的智能指针，若没有查到则返回空智能指针
 */
template <typename NodeType, typename ValType>
std::shared_ptr<NodeType> binSearch(const std::shared_ptr<NodeType>& proot, const ValType& val) {
  std::shared_ptr<NodeType> p = proot;
  while (p) {
    if (p->obj == val) return p;
    if (val < p->obj)
      p = p->pl;
    else if (val > p->obj)
      p = p->pr;
    else
      return std::shared_ptr<NodeType>();
  }
  return std::shared_ptr<NodeType>();
}

/**
 * @brief AVL树
 * 二叉平衡树。模板参数T需要支持比较运算
 */
template <typename T>
class AVLTreeNode : public std::enable_shared_from_this<AVLTreeNode<T> > {
 private:
  typedef std::shared_ptr<AVLTreeNode<T> > AVLTNodePtr;

 public:
  AVLTreeNode() : pf(NULL), hgt(1) {}
  explicit AVLTreeNode(const T& _obj) : obj(_obj), pf(NULL), hgt(1) {}

  T obj;               ///<实际节点值
  AVLTreeNode<T>* pf;  ///<父节点
  AVLTNodePtr pl;      ///<左子节点
  AVLTNodePtr pr;      ///<右子节点
  size_t hgt;          ///<节点高度

#define HGT(p) ((p) ? p->hgt : 0)

  ///插入，因为根节点可能会变，所以返回根节点
  AVLTNodePtr insert(AVLTNodePtr ndptr) {
    assert(ndptr);
    //找到最终要插入的地方的父节点
    AVLTreeNode<T>*pos = this, *tmppos = (ndptr->obj < obj) ? pl.get() : pr.get();
    while (tmppos != NULL) {
      pos = tmppos;
      tmppos = (ndptr->obj < pos->obj) ? pos->pl.get() : pos->pr.get();
    }
    if (ndptr->obj == pos->obj) return this->shared_from_this();  //不允许重复
    if (ndptr->obj < pos->obj)
      setLChild(pos, ndptr);
    else
      setRChild(pos, ndptr);

    ndptr->hgt = 1;
    //更新高度，进行旋转
    AVLTNodePtr re;
    AVLTreeNode<T>* end = pf;
    while (pos != end) {
      re.reset();
      size_t curhgt = pos->hgt;
      size_t lh = HGT(pos->pl), lr = HGT(pos->pr);
      if (lh >= lr + 2) {
        //左边比右边高了2
        if (HGT(pos->pl->pl) >= HGT(pos->pl->pr))
          re = pos->rotateL();
        else {
          pos->pl->rotateR();
          re = pos->rotateL();
        }
      } else if (lr >= lh + 2) {
        //右边比左边高了2
        if (HGT(pos->pr->pr) >= HGT(pos->pr->pl))
          re = pos->rotateR();
        else {
          pos->pr->rotateL();
          re = pos->rotateR();
        }
      }
      //如果发生旋转了，说明之前左右高度相差2，说明该节点高度一定发生改变
      size_t cghgt;
      if (re) {
        cghgt = re->hgt;
        pos = re->pf;
      } else {
        cghgt = pos->hgt = std::max(lh, lr) + 1;
        if (curhgt == cghgt) return this->shared_from_this();
        pos = pos->pf;
      }
    }
    if (re) return re;
    return this->shared_from_this();
  }

  ///在当前节点为根节点的树中删除一个节点，并返回删除后的根节点
  AVLTNodePtr erase(AVLTNodePtr ndptr) {
    if (!ndptr) return this->shared_from_this();
    //先确定要删除的节点是自己的子节点
    AVLTreeNode<T>* pos = ndptr.get();
    while (pos != NULL) {
      if (pos == this) break;
      pos = pos->pf;
    }
    if (pos == this) return _erase(ndptr);
    return this->shared_from_this();
  }
  ///在当前节点中删除节点值为val的节点。会先搜索再删除
  AVLTNodePtr erase(const T& val) {
    return _erase(binSearch<AVLTreeNode<T>, T>(this->shared_from_this(), val));
  }

 private:
  ///假如有重复的，删除第一个找到的
  AVLTNodePtr _erase(AVLTNodePtr ndptr) {
    assert(pf == NULL);  //自身需要是根节点
    if (!ndptr) return this->shared_from_this();
    AVLTNodePtr proot = this->shared_from_this();  //如果要删除的是自身，则需要一个指针来保存root节点
    AVLTreeNode<T>* pos = ndptr->pf;
    if (!(ndptr->pl) && !(ndptr->pr)) {
      //左右都为空，为叶子节点
      if (ndptr->pf != NULL) {
        if (getLR(ndptr.get()))
          breakLChild(ndptr->pf);
        else
          breakRChild(ndptr->pf);
      } else {
        //整个树就一个要删除的根节点
        return AVLTNodePtr();
      }
    } else if (ndptr->pl && !(ndptr->pr)) {
      //只有左子树
      if (ndptr->pf == NULL) {
        proot = ndptr->pl;
        breakLChild(ndptr.get());
      } else {
        ndptr->pl->pf = ndptr->pf;
        ndptr->pf->pl = ndptr->pl;
        ndptr->pf = NULL;
        ndptr->pl.reset();
      }
    } else if (!(ndptr->pl) && ndptr->pr) {
      //只有右子树
      if (ndptr->pf == NULL) {
        proot = ndptr->pr;
        breakRChild(ndptr.get());
      } else {
        ndptr->pr->pf = ndptr->pf;
        ndptr->pf->pr = ndptr->pr;
        ndptr->pf = NULL;
        ndptr->pr.reset();
      }
    } else {
      //换左子树的前驱
      AVLTNodePtr tmp = ndptr->pl;
      if (tmp->pr) {
        //左子节点有右子树，找到其前驱
        while (tmp->pr) tmp = tmp->pr;
        tmp->pf->pr = tmp->pl;
        if (tmp->pl) tmp->pl->pf = tmp->pf;
        tmp->pl = ndptr->pl;
        ndptr->pl->pf = tmp.get();
        pos = tmp->pf;
      } else
        pos = tmp.get();
      tmp->pf = ndptr->pf;
      tmp->pr = ndptr->pr;
      ndptr->pr->pf = tmp.get();
      if (ndptr->pf != NULL) {
        if (getLR(ndptr.get()))
          ndptr->pf->pl = tmp;
        else
          ndptr->pf->pr = tmp;
      } else
        proot = tmp;
      ndptr->pf = NULL;
      ndptr->pl.reset();
      ndptr->pr.reset();
      tmp->hgt = ndptr->hgt;
    }
    //更新高度，进行旋转
    AVLTNodePtr re;
    while (pos != NULL) {
      re.reset();
      size_t curhgt = pos->hgt;
      size_t lh = HGT(pos->pl), lr = HGT(pos->pr);
      if (lh >= lr + 2) {
        //左边比右边高了2
        if (HGT(pos->pl->pl) >= HGT(pos->pl->pr))
          re = pos->rotateL();
        else {
          pos->pl->rotateR();
          re = pos->rotateL();
        }
      } else if (lr >= lh + 2) {
        //右边比左边高了2
        if (HGT(pos->pr->pr) >= HGT(pos->pr->pl))
          re = pos->rotateR();
        else {
          pos->pr->rotateL();
          re = pos->rotateR();
        }
      }
      //如果发生旋转了，说明之前左右高度相差2，说明该节点高度一定发生改变
      size_t cghgt;
      if (re) {
        cghgt = re->hgt;
        pos = re->pf;
      } else {
        cghgt = pos->hgt = std::max(lh, lr) + 1;
        if (curhgt == cghgt) break;
        pos = pos->pf;
      }
    }
    if (re) return re;
    return proot;
  }
  ///获取当前节点高度
  inline size_t getHgt() {
    size_t lh = (pl) ? pl->hgt : 0;
    size_t lr = (pr) ? pr->hgt : 0;
    return std::max(lh, lr) + 1;
  }

  ///左旋转，顺时针
  AVLTNodePtr rotateL() {
    AVLTNodePtr re = pl;
    if (re->pr) re->pr->pf = this;
    pl = re->pr;
    if (pf == NULL)
      re->pr = this->shared_from_this();
    else {
      if (getLR(this)) {
        re->pr = pf->pl;
        pf->pl = re;
      } else {
        re->pr = pf->pr;
        pf->pr = re;
      }
    }
    re->pf = pf;
    pf = re.get();
    hgt = getHgt();
    re->hgt = re->getHgt();
    return re;
  }
  ///右旋转，逆时针
  AVLTNodePtr rotateR() {
    AVLTNodePtr re = pr;
    if (re->pl) re->pl->pf = this;
    pr = re->pl;
    if (pf == NULL)
      re->pl = this->shared_from_this();
    else {
      if (getLR(this)) {
        re->pl = pf->pl;
        pf->pl = re;
      } else {
        re->pl = pf->pr;
        pf->pr = re;
      }
    }
    re->pf = pf;
    pf = re.get();
    hgt = getHgt();
    re->hgt = re->getHgt();
    return re;
  }
};
/**
 * @brief 红黑树
 * 二叉平衡树。模板参数T需要支持比较运算
 * todo待完善
 */
template <typename T>
class BRTreeNode : public std::enable_shared_from_this<BRTreeNode<T> > {
 private:
  typedef std::shared_ptr<BRTreeNode<T> > BRTreeNodePtr;

 public:
  BRTreeNode() : pf(NULL), color(false) {}
  explicit BRTreeNode(const T& _obj) : obj(_obj), pf(NULL), color(false) {}

  T obj;              ///<实际节点值
  BRTreeNode<T>* pf;  ///<父节点
  BRTreeNodePtr pl;   ///<左子节点
  BRTreeNodePtr pr;   ///<右子节点
  bool color;         ///<颜色，true为红，false为黑

  ///插入，因为根节点可能会变，所以返回根节点
  BRTreeNodePtr insert(BRTreeNodePtr ndptr) {
    assert(ndptr && !color);
    //找到最终要插入的地方的父节点
    BRTreeNode<T>*pos = this, *tmppos = (ndptr->obj < obj) ? pl.get() : pr.get();
    while (tmppos != NULL) {
      pos = tmppos;
      tmppos = (ndptr->obj < pos->obj) ? pos->pl.get() : pos->pr.get();
    }
    if (ndptr->obj == pos->obj) return this->shared_from_this();  //不允许重复
    if (ndptr->obj < pos->obj)
      setLChild(pos, ndptr);
    else
      setRChild(pos, ndptr);

    //插入节点的颜色总是红色
    ndptr->color = true;

    BRTreeNodePtr re;
    BRTreeNode<T>* end = pf;
    tmppos = ndptr.get();
    while (pos != end) {
      re.reset();
      //父节点是黑色
      if (!(pos->color)) {
        return this->shared_from_this();
      }
      BRTreeNode<T>* uncle = (getLR(pos) ? pos->pf->pr.get() : pos->pf->pl.get());
      if (uncle != NULL && uncle->color) {
        //插入节点的父节点和其叔叔节点均为红色的
        pos->color = uncle->color = false;
        pos->pf->color = true;
        tmppos = pos->pf;
        pos = tmppos->pf;
      } else {
        //插入节点的父节点是红色，叔叔节点是黑色
        if (getLR(pos)) {
          //父节点是祖父节点的左支
          if (getLR(tmppos)) {
            //插入节点是其父节点的左子节点
            pos->color = false;
            pos->pf->color = true;
            re = pos->pf->rotateL();
            break;
          } else {
            //插入节点是其父节点的右子节点
            pos->rotateR();
            tmppos = pos;
            pos = tmppos->pf;
          }
        } else {
          //父节点是祖父节点的右支
          if (getLR(tmppos)) {
            //插入节点是其父节点的左子节点
            pos->rotateL();
            tmppos = pos;
            pos = tmppos->pf;
          } else {
            //插入节点是其父节点的右子节点
            pos->color = false;
            pos->pf->color = true;
            re = pos->pf->rotateR();
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
  BRTreeNodePtr erase(BRTreeNodePtr ndptr) {
    if (!ndptr) return this->shared_from_this();
    //先确定要删除的节点是自己的子节点
    BRTreeNode<T>* pos = ndptr.get();
    while (pos != NULL) {
      if (pos == this) break;
      pos = pos->pf;
    }
    if (pos == this) return _erase(ndptr);
    return this->shared_from_this();
  }
  ///在当前节点中删除节点值为val的节点。会先搜索再删除
  BRTreeNodePtr erase(const T& val) {
    return _erase(binSearch<BRTreeNode<T>, T>(this->shared_from_this(), val));
  }

 private:
  BRTreeNodePtr _erase(BRTreeNodePtr ndptr) {
    assert(pf == NULL);  //自身需要是根节点
    if (!ndptr) return this->shared_from_this();
    BRTreeNodePtr proot = this->shared_from_this();  //如果要删除的是自身，则需要一个指针来保存root节点
    BRTreeNode<T>* pos = ndptr->pf;
    if (!(ndptr->pl) && !(ndptr->pr)) {
      //左右都为空，为叶子节点
      if (ndptr->pf != NULL) {
        if (getLR(ndptr.get()))
          breakLChild(ndptr->pf);
        else
          breakRChild(ndptr->pf);
      } else {
        //整个树就一个要删除的根节点
        return BRTreeNodePtr();
      }
    } else if (ndptr->pl && !(ndptr->pr)) {
      //只有左子树
      if (ndptr->pf == NULL) {
        proot = ndptr->pl;
        breakLChild(ndptr.get());
      } else {
        ndptr->pl->pf = ndptr->pf;
        ndptr->pf->pl = ndptr->pl;
        ndptr->pf = NULL;
        ndptr->pl.reset();
      }
    } else if (!(ndptr->pl) && ndptr->pr) {
      //只有右子树
      if (ndptr->pf == NULL) {
        proot = ndptr->pr;
        breakRChild(ndptr.get());
      } else {
        ndptr->pr->pf = ndptr->pf;
        ndptr->pf->pr = ndptr->pr;
        ndptr->pf = NULL;
        ndptr->pr.reset();
      }
    } else {
      //换左子树的前驱
      BRTreeNodePtr tmp = ndptr->pl;
      if (tmp->pr) {
        //左子节点有右子树，找到其前驱
        while (tmp->pr) tmp = tmp->pr;
        tmp->pf->pr = tmp->pl;
        if (tmp->pl) tmp->pl->pf = tmp->pf;
        tmp->pl = ndptr->pl;
        ndptr->pl->pf = tmp.get();
        pos = tmp->pf;
      } else
        pos = tmp.get();
      tmp->pf = ndptr->pf;
      tmp->pr = ndptr->pr;
      ndptr->pr->pf = tmp.get();
      if (ndptr->pf != NULL) {
        if (getLR(ndptr.get()))
          ndptr->pf->pl = tmp;
        else
          ndptr->pf->pr = tmp;
      } else
        proot = tmp;
      ndptr->pf = NULL;
      ndptr->pl.reset();
      ndptr->pr.reset();
      tmp->hgt = ndptr->hgt;
    }
  }
  ///<左旋转，顺时针
  BRTreeNodePtr rotateL() {
    BRTreeNodePtr re = pl;
    if (re->pr) re->pr->pf = this;
    pl = re->pr;
    if (pf == NULL)
      re->pr = this->shared_from_this();
    else {
      if (getLR(this)) {
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
  BRTreeNodePtr rotateR() {
    BRTreeNodePtr re = pr;
    if (re->pl) re->pl->pf = this;
    pr = re->pl;
    if (pf == NULL)
      re->pl = this->shared_from_this();
    else {
      if (getLR(this)) {
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
};
/**
 * @brief 前序遍历
 * @note 以当前节点为根节点，递归前序遍历，返回一个指针数组。中-左-右
 * @param T 模板参数，节点类型
 * @param nd 要前序遍历的根节点
 * @param vec 指针数组，要返回的遍历结果
 * @return 无
 */
template <typename T>
void DLR(std::shared_ptr<T> nd, std::vector<std::shared_ptr<T> >& vec) {
  assert(nd);
  vec.push_back(nd);
  if (nd->pl) DLR(nd->pl, vec);
  if (nd->pr) DLR(nd->pr, vec);
}
/**
 * @brief 中序遍历
 * @note 以当前节点为根节点，递归中序遍历，返回一个指针数组。左-中-右
 * @param T 模板参数，节点类型
 * @param nd 要前序遍历的根节点
 * @param vec 指针数组，要返回的遍历结果
 * @return 无
 */
template <typename T>
void LDR(std::shared_ptr<T> nd, std::vector<std::shared_ptr<T> >& vec) {
  assert(nd);
  if (nd->pl) LDR(nd->pl, vec);
  vec.push_back(nd);
  if (nd->pr) LDR(nd->pr, vec);
}
/**
 * @brief 后序遍历
 * @note 以当前节点为根节点，递归后序遍历，返回一个指针数组。左-右-中
 * @param T 模板参数，节点类型
 * @param nd 要前序遍历的根节点
 * @param vec 指针数组，要返回的遍历结果
 * @return 无
 */
template <typename T>
void LRD(std::shared_ptr<T> nd, std::vector<std::shared_ptr<T> >& vec) {
  assert(nd);
  if (nd->pl) LRD(nd->pl, vec);
  if (nd->pr) LRD(nd->pr, vec);
  vec.push_back(nd);
}
/**
 * @brief 获取一个节点的深度
 * @note 获取一个节点的深度，根节点深度为0
 * @param T 模板参数，节点类型
 * @param pnode 要获取深度的节点
 * @return 节点深度
 */
template <typename T>
size_t getDepth(const T* pnode) {
  assert(pnode != NULL);
  pnode = pnode->pf;
  size_t count = 0;
  while (pnode != NULL) {
    ++count;
    pnode = pnode->pf;
  }
  return count;
}
/**
 * @brief 获取一个节点的高度
 * @note 获取一个节点的高度，叶子节点高度为1
 * @param T 模板参数，节点类型
 * @param pnode 要获取高度的节点
 * @return 节点高度
 */
template <typename T>
size_t getHeight(const T* pnode) {
  assert(pnode != NULL);
  size_t lh = 0, rh = 0;
  if (pnode->pl) lh = getHeight(pnode->pl.get());
  if (pnode->pr) rh = getHeight(pnode->pr.get());
  return std::max(lh, rh) + 1;
}
/**
 * @brief 获取一个树中最长根-叶链长度
 * @note 获取一个树中从根节点到叶子节点的最长节点个数。实际调用getHeight
 * @param T 模板参数，节点类型
 * @param pnode 要获取最长链长度的树的根节点
 * @return 最长链长度
 */
template <typename T>
size_t getMaxChain(const T* pnode) {
  return getHeight(pnode);
}
/**
 * @brief 获取一个树中最短根-叶链长度
 * @note 获取一个树中从根节点到叶子节点的最短节点个数
 * @param T 模板参数，节点类型
 * @param pnode 要获取最短链长度的树的根节点
 * @return 最短链长度
 */
template <typename T>
size_t getMinChain(const T* pnode) {
  assert(pnode != NULL);
  size_t lh = 0, rh = 0;
  if (pnode->pl) lh = getHeight(pnode->pl.get());
  if (pnode->pr) rh = getHeight(pnode->pr.get());
  return std::min(lh, rh) + 1;
}
/**
 * @brief 获取树中节点个数
 * @note 获取树中节点个数
 * @param T 模板参数，节点类型
 * @param pnode 要获取节点个数的树的根节点
 * @return 节点个数
 */
template <typename T>
size_t getNodeNum(const T* pnode) {
  assert(pnode != NULL);
  size_t num = 1;
  if (pnode->pl) num += getNodeNum(pnode->pl.get());
  if (pnode->pr) num += getNodeNum(pnode->pr.get());
  return num;
}
/**
 * @brief 设置子节点
 * @note 将一个节点作为左子节点，与原左子节点断开。插入的节点与其原父节点断开
 * @param T 模板参数，节点类型
 * @param pfather 父节点
 * @param pchild 子节点
 * @return 无
 */
template <typename T>
void setLChild(T* pfather, std::shared_ptr<T> pchild) {
  assert((pfather != NULL) && pchild);
  pchild->pf = pfather;
  if (pfather->pl) pfather->pl->pf = NULL;
  pfather->pl = pchild;
}
/**
 * @brief 设置子节点
 * @note 将一个节点作为右子节点，与原右子节点断开。插入的节点与其原父节点断开
 * @param T 模板参数，节点类型
 * @param pfather 父节点
 * @param pchild 子节点
 * @return 无
 */
template <typename T>
void setRChild(T* pfather, std::shared_ptr<T> pchild) {
  assert((pfather != NULL) && pchild);
  pchild->pf = pfather;
  if (pfather->pr) pfather->pr->pf = NULL;
  pfather->pr = pchild;
}
/**
 * @brief 与左子树断开
 * @note 与左子树断开
 * @param T 模板参数，节点类型
 * @param pnode 待处理节点
 * @return 无
 */
template <typename T>
void breakLChild(T* pnode) {
  assert((pnode != NULL) && pnode->pl);
  pnode->pl->pf = NULL;
  pnode->pl.reset();
}
/**
 * @brief 与右子树断开
 * @note 与右子树断开
 * @param T 模板参数，节点类型
 * @param pnode 待处理节点
 * @return 无
 */
template <typename T>
void breakRChild(T* pnode) {
  assert((pnode != NULL) && pnode->pr);
  pnode->pr->pf = NULL;
  pnode->pr.reset();
}
/**
 * @brief 判断是否为父节点的左节点
 * @note 判断是父节点的左节点还是右节点。true表示左。使用前应检查父节点是否为空
 * @param T 模板参数，节点类型
 * @param pnode 待处理节点
 * @return 无
 */
template <typename T>
bool getLR(const T* pnode) {
  assert(pnode && pnode->pf != NULL);
  if (pnode == pnode->pf->pl.get()) return true;
  if (pnode == pnode->pf->pr.get()) return false;
  assert(0);  //不是父节点的左右节点。报错
  return true;
}
/**
 * @brief 分层遍历
 * @note 分层遍历二叉树，将结果输出到vector数组中
 * @param T 模板参数，节点类型
 * @param nd 待分层遍历的树的根节点
 * @param vec 要返回的遍历结果
 * @return 无
 */
template <typename T>
void traByLevel(std::shared_ptr<T> nd, std::vector<std::shared_ptr<T> >& vec) {
  assert(nd);
  size_t pos1 = vec.size(), pos2;
  vec.push_back(nd);
  while (pos1 < vec.size()) {
    pos2 = vec.size();
    while (pos1 < pos2) {
      if (vec[pos1]->pl) vec.push_back(vec[pos1]->pl);
      if (vec[pos1]->pr) vec.push_back(vec[pos1]->pr);
      ++pos1;
    }
  }
}
/**
 * @brief 二叉树序列化
 * @note 根据前序遍历进行的二叉树序列化，存储为vector<pair<bool,T> >
 * @param NodeType 模板参数，节点类型
 * @param ValType 模板参数，节点obj类型
 * @param proot 待序列化的树的根节点
 * @param vec 要返回的序列化结果
 * @return 无
 */
template <typename NodeType, typename ValType>
void SerializeTree(const std::shared_ptr<NodeType>& proot, std::vector<std::pair<bool, ValType> >& vec) {
  if (proot) {
    vec.push_back(std::pair<bool, ValType>(true, proot->obj));
    SerializeTree(proot->pl, vec);
    SerializeTree(proot->pr, vec);
  } else {
    vec.push_back(std::pair<bool, ValType>(false, ValType()));
  }
}
/**
 * @brief 二叉树反序列化
 * @note 根据前序遍历进行的二叉树反序列化，根据vector<pair<bool,T> >反序列化
 * @param NodeType 模板参数，节点类型
 * @param ValType 模板参数，节点obj类型
 * @param proot 待存放反序列化结果的树的根节点
 * @param vec 根据其存储的数据进行反序列化
 * @return 无
 */
template <typename NodeType, typename ValType>
void DeserializeTree(std::shared_ptr<NodeType>& proot, const std::vector<std::pair<bool, ValType> >& vec) {
  typename std::vector<std::pair<bool, ValType> >::const_iterator itr = vec.begin();
  DeserializeTree<NodeType, ValType>(proot, itr);
}
/**
 * @brief 二叉树反序列化
 * @note 根据前序遍历进行的二叉树反序列化，根据vector<pair<bool,T> >::const_iterator反序列化
 * @param NodeType 模板参数，节点类型
 * @param ValType 模板参数，节点obj类型
 * @param proot 待存放反序列化结果的树的根节点
 * @param itr 指向要进行反序列化的vector开头的迭代器
 * @return 无
 */
template <typename NodeType, typename ValType>
void DeserializeTree(std::shared_ptr<NodeType>& proot, typename std::vector<std::pair<bool, ValType> >::const_iterator& itr) {
  if (itr->first) {
    proot = std::make_shared<NodeType>(itr->second);
    DeserializeTree<NodeType, ValType>(proot->pl, ++itr);
    if (proot->pl) proot->pl->pf = proot.get();
    DeserializeTree<NodeType, ValType>(proot->pr, ++itr);
    if (proot->pr) proot->pr->pf = proot.get();
  }
}
/**
 * @brief 二叉树深拷贝
 * @note 对二叉树进行深拷贝
 * @param T 模板参数，节点类型
 * @param proot 待拷贝的树的根节点
 * @return 深拷贝结果
 */
template <typename T>
std::shared_ptr<T> copyTree(const std::shared_ptr<T>& proot) {
  std::shared_ptr<T> p = std::make_shared<T>(T(proot->obj));
  if (proot->pl) setLChild(p.get(), copyTree(proot->pl));
  if (proot->pr) setRChild(p.get(), copyTree(proot->pr));
  return p;
}

/**
 * @brief 检查是否为二叉树
 * @note 检查一颗二叉树是否合法，子节点、父节点是否对应相连
 * @param T 模板参数，节点类型
 * @param proot 待检查的树的根节点
 * @return 二叉树是否合法
 */
template <typename T>
bool checkBinTree(const std::shared_ptr<T>& proot) {
  if (proot) {
    if (proot->pl) {
      if (proot->pl->pf != proot.get()) return false;
      if (!checkBinTree(proot->pl)) return false;
    }
    if (proot->pr) {
      if (proot->pr->pf != proot.get()) return false;
      if (!checkBinTree(proot->pr)) return false;
    }
  }
  return true;
}

/**
 * @brief 检查是否为搜索二叉树
 * @note 检查一颗二叉树是否为搜索二叉树，要求左<中<=右
 * @param T 模板参数，节点类型
 * @param proot 待检查的树的根节点
 * @return 是否为搜索二叉树
 */
template <typename T>
bool checkBinSearchTree(const std::shared_ptr<T>& proot) {
  if (proot) {
    if (proot->pl) {
      if (proot->pl->pf != proot.get()) return false;
      if (proot->pl->obj >= proot->obj) return false;
      if (!checkBinSearchTree(proot->pl)) return false;
    }
    if (proot->pr) {
      if (proot->pr->pf != proot.get()) return false;
      if (proot->pr->obj < proot->obj) return false;
      if (!checkBinSearchTree(proot->pr)) return false;
    }
  }
  return true;
}
/**
 * @brief 检查是否为AVL树
 * @note 检查一颗二叉树是否为AVL树，要求各叶子节点深度相差<=1
 * @param T 模板参数，节点类型
 * @param proot 待检查的树的根节点
 * @return 是否为AVL树
 */
template <typename T>
bool checkAVLTree(const std::shared_ptr<T>& proot) {
  if (!checkBinSearchTree(proot)) return false;
  if (getMaxChain(proot.get()) > getMinChain(proot.get()) + 1) return false;
  return true;
}
/**
 * @brief 检查是否为红黑树
 * @note 检查一颗二叉树是否为红黑树，要求根节点黑、没有连续红节点、所有路径有相同数目黑节点
 * @param T 模板参数，节点类型
 * @param proot 待检查的树的根节点
 * @return 是否为红黑树
 */
template <typename T>
bool checkBRTree(const std::shared_ptr<T>& proot) {
  if (!checkBinSearchTree(proot)) return false;
  if (proot->color != false) return false;  //根节点要为黑

  return true;
}
}  // namespace ytlib
