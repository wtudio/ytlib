#pragma once
#include <ytlib/Common/Util.h>
#include <memory>
#include <vector>


namespace ytlib {
	//几种树的类的实现---------------------------------------------------------------------------
	//全部使用模板化。这里不能使用继承，因为有成员是自身类型的智能指针
	//一个BinTreeNode实例表示一个二叉树节点，也表示以此节点为根节点的一棵二叉树
	//如果根节点被析构，那么整个树中所有子节点将被析构，除非子节点有另外的智能指针指着
	template<typename T>
	class BinTreeNode {
	private:
		typedef std::shared_ptr<BinTreeNode<T> > nodePtr;
	public:
		BinTreeNode():pf(NULL){}
		explicit BinTreeNode(const T& _obj) :obj(_obj), pf(NULL) {}

		T obj;
		BinTreeNode<T>* pf;//父节点。父节点不可使用智能指针，否则会造成循环引用
		nodePtr pl;//左子节点
		nodePtr pr;//右子节点

	};

	//二叉查找树，T需要支持比较运算
	template<typename T>
	class BinSearchTreeNode {
	private:
		typedef std::shared_ptr<BinSearchTreeNode<T> > BSTNodePtr;
	public:
		BinSearchTreeNode() :pf(NULL) {}
		explicit BinSearchTreeNode(const T& _obj) :obj(_obj), pf(NULL) {}

		T obj;
		BinSearchTreeNode<T>* pf;//父节点
		BSTNodePtr pl;//左子节点
		BSTNodePtr pr;//右子节点

		//向当前节点为根节点的二叉查找树中插入一个节点
		void insert(BSTNodePtr& ndptr) {
			assert(ndptr);
			if (ndptr->obj < obj) {
				if (pl) pl->insert(ndptr);
				else setLChild(this,ndptr);
			}
			else {
				if (pr) pr->insert(ndptr);
				else setRChild(this, ndptr);
			}
		}

		//删除当前节点，并返回替代的节点
		BSTNodePtr erase() {
			if (!pl && !pr) {
				//左右都为空，为叶子节点
				if (pf != NULL) {
					if (getLR(this)) breakLChild(pf);
					else breakRChild(pf);
				}
				return BSTNodePtr();
			}
			if (pl && !pr) {
				//只有左子树
				BSTNodePtr re = pl;
				if (pf == NULL) breakLChild(this);
				else {
					pl->pf = pf;
					pf->pl = pl;
					pf = NULL;pl.reset();
				}
				return re;
			}
			if (!pl && pr) {
				//只有右子树
				BSTNodePtr re = pr;
				if (pf == NULL) breakRChild(this);
				else {
					pr->pf = pf;
					pf->pr = pr;
					pf = NULL;pr.reset();
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
				tmp->pl = pl; pl->pf = tmp.get();
			}
			tmp->pf = pf;
			tmp->pr = pr; pr->pf = tmp.get();
			if (pf != NULL) {
				if (getLR(this)) pf->pl = tmp;
				else pf->pr = tmp;
			}
			pf = NULL;pl.reset();pr.reset();
			return tmp;
		}
	};

	//AVL树
	template<typename T>
	class AVLTreeNode :public std::enable_shared_from_this<AVLTreeNode<T> > {
	private:
		typedef std::shared_ptr<AVLTreeNode<T> > AVLTNodePtr;
	public:
		AVLTreeNode() :pf(NULL), hgt(1){}
		explicit AVLTreeNode(const T& _obj) :obj(_obj), pf(NULL), hgt(1) {}

		T obj;
		AVLTreeNode<T>* pf;//父节点
		AVLTNodePtr pl;//左子节点
		AVLTNodePtr pr;//右子节点
		size_t hgt;//节点高度

#define HGT(p)	((p)?p->hgt:0)

		//插入，因为根节点可能会变，所以返回根节点
		AVLTNodePtr insert(AVLTNodePtr& ndptr) {
			assert(ndptr);
			//找到最终要插入的地方的父节点
			AVLTreeNode<T>* pos = this, *tmppos = (ndptr->obj < obj) ? pl.get() : pr.get();
			while (tmppos != NULL) {
				pos = tmppos;
				tmppos = (ndptr->obj < pos->obj) ? pos->pl.get() : pos->pr.get();
			}
			if (ndptr->obj == pos->obj) return shared_from_this();//不允许重复
			if (ndptr->obj < pos->obj) setLChild(pos, ndptr);
			else setRChild(pos, ndptr);

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
					if (HGT(pos->pl->pl) >= HGT(pos->pl->pr)) re = pos->rotateL();
					else {
						pos->pl->rotateR();
						re = pos->rotateL();
					}
				}
				else if (lr >= lh + 2) {
					//右边比左边高了2
					if (HGT(pos->pr->pr) >= HGT(pos->pr->pl)) re = pos->rotateR();
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
				}
				else {
					cghgt = pos->hgt = max(lh, lr) + 1;
					if (curhgt == cghgt) return shared_from_this();
					pos = pos->pf;
				}
				
			}
			if (re) return re;
			return shared_from_this();
		}

		//在当前节点为根节点的树中删除一个节点，并返回删除后的根节点
		AVLTNodePtr erase(AVLTNodePtr& ndptr) {
			if (!ndptr) return shared_from_this();
			//先确定要删除的节点是自己的子节点
			AVLTreeNode<T>* pos = ndptr.get();
			while (pos != NULL) {
				if (pos == this) break;
				pos = pos->pf;
			}
			if (pos == this) return _erase(ndptr);
			return shared_from_this();
		}

		AVLTNodePtr erase(const T& val) {
			return _erase(binSearch<AVLTreeNode<T>, T>(shared_from_this(), val));
		}

	private:
		//假如有重复的，删除第一个找到的
		AVLTNodePtr _erase(AVLTNodePtr& ndptr) {
			assert(pf == NULL);//自身需要是根节点
			if (!ndptr) return shared_from_this();
			AVLTNodePtr proot = shared_from_this();//如果要删除的是自身，则需要一个指针来保存root节点
			AVLTreeNode<T>* pos = ndptr->pf;
			if (!(ndptr->pl) && !(ndptr->pr)) {
				//左右都为空，为叶子节点
				if (ndptr->pf != NULL) {
					if (getLR(ndptr.get())) breakLChild(ndptr->pf);
					else breakRChild(ndptr->pf);
				}
				else {
					//整个树就一个要删除的根节点
					return AVLTNodePtr();
				}
			}
			else if (ndptr->pl && !(ndptr->pr)) {
				//只有左子树
				if (ndptr->pf == NULL) {
					proot = ndptr->pl;
					breakLChild(ndptr.get());
				}
				else {
					ndptr->pl->pf = ndptr->pf;
					ndptr->pf->pl = ndptr->pl;
					ndptr->pf = NULL; ndptr->pl.reset();
				}
			}
			else if (!(ndptr->pl) && ndptr->pr) {
				//只有右子树
				if (ndptr->pf == NULL) { 
					proot = ndptr->pr;
					breakRChild(ndptr.get());
				}
				else {
					ndptr->pr->pf = ndptr->pf;
					ndptr->pf->pr = ndptr->pr;
					ndptr->pf = NULL; ndptr->pr.reset();
				}
			}
			else {
				//换左子树的前驱
				AVLTNodePtr tmp = ndptr->pl;
				if (tmp->pr) {
					//左子节点有右子树，找到其前驱
					while (tmp->pr) tmp = tmp->pr;
					tmp->pf->pr = tmp->pl;
					if (tmp->pl) tmp->pl->pf = tmp->pf;
					tmp->pl = ndptr->pl; ndptr->pl->pf = tmp.get();
					pos = tmp->pf;
				}
				else pos = tmp.get();
				tmp->pf = ndptr->pf;
				tmp->pr = ndptr->pr; ndptr->pr->pf = tmp.get();
				if (ndptr->pf != NULL) {
					if (getLR(ndptr.get())) ndptr->pf->pl = tmp;
					else ndptr->pf->pr = tmp;
				}
				else proot = tmp;
				ndptr->pf = NULL; ndptr->pl.reset(); ndptr->pr.reset();
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
					if (HGT(pos->pl->pl) >= HGT(pos->pl->pr)) re = pos->rotateL();
					else {
						pos->pl->rotateR();
						re = pos->rotateL();
					}
				}
				else if (lr >= lh + 2) {
					//右边比左边高了2
					if (HGT(pos->pr->pr) >= HGT(pos->pr->pl)) re = pos->rotateR();
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
				}
				else {
					cghgt = pos->hgt = max(lh, lr) + 1;
					if (curhgt == cghgt) break;
					pos = pos->pf;
				}
			}
			if (re) return re;
			return proot;
		}

		inline size_t getHgt() {
			size_t lh = (pl) ? pl->hgt : 0;
			size_t lr = (pr) ? pr->hgt : 0;
			return max(lh, lr) + 1;
		}

		//左旋转，顺时针
		AVLTNodePtr rotateL() {
			AVLTNodePtr re = pl;
			if (re->pr) re->pr->pf = this;
			pl = re->pr;
			if (pf == NULL) re->pr = shared_from_this();
			else {
				if (getLR(this)) {
					re->pr = pf->pl;
					pf->pl = re;
				}
				else {
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
		//右旋转，逆时针
		AVLTNodePtr rotateR() {
			AVLTNodePtr re = pr;
			if (re->pl) re->pl->pf = this;
			pr = re->pl;
			if (pf == NULL) re->pl = shared_from_this();
			else {
				if (getLR(this)) {
					re->pl = pf->pl;
					pf->pl = re;
				}
				else {
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

	//红黑树。todo待完善
	template<typename T>
	class BRTreeNode :public std::enable_shared_from_this<BRTreeNode<T> > {
	private:
		typedef std::shared_ptr<BRTreeNode<T> > BRTreeNodePtr;
	public:
		BRTreeNode() :pf(NULL), color(false){}
		explicit BRTreeNode(const T& _obj) :obj(_obj), pf(NULL), color(false) {}

		T obj;
		BRTreeNode<T>* pf;//父节点
		BRTreeNodePtr pl;//左子节点
		BRTreeNodePtr pr;//右子节点
		bool color;//颜色，true为红，false为黑

		//插入，因为根节点可能会变，所以返回根节点
		BRTreeNodePtr insert(BRTreeNodePtr& ndptr) {
			assert(ndptr && !color);
			//找到最终要插入的地方的父节点
			BRTreeNode<T>* pos = this, *tmppos = (ndptr->obj < obj) ? pl.get() : pr.get();
			while (tmppos != NULL) {
				pos = tmppos;
				tmppos = (ndptr->obj < pos->obj) ? pos->pl.get() : pos->pr.get();
			}
			if (ndptr->obj == pos->obj) return shared_from_this();//不允许重复
			if (ndptr->obj < pos->obj) setLChild(pos, ndptr);
			else setRChild(pos, ndptr);

			//插入节点的颜色总是红色
			ndptr->color = true;
			
			BRTreeNodePtr re;
			BRTreeNode<T>* end = pf;
			tmppos = ndptr.get();
			while (pos != end) {
				re.reset();
				//父节点是黑色
				if (!(pos->color)) {
					return shared_from_this();
				}
				BRTreeNode<T>* uncle = (getLR(pos) ? pos->pf->pr.get() : pos->pf->pl.get());
				if (uncle!=NULL && uncle->color) {
					//插入节点的父节点和其叔叔节点均为红色的
					pos->color = uncle->color = false;
					pos->pf->color = true;
					tmppos = pos->pf;
					pos = tmppos->pf;
				}
				else {
					//插入节点的父节点是红色，叔叔节点是黑色
					if (getLR(pos)) {
						//父节点是祖父节点的左支
						if (getLR(tmppos)) {
							//插入节点是其父节点的左子节点
							pos->color = false;
							pos->pf->color = true;
							re = pos->pf->rotateL();
							break;
						}
						else {
							//插入节点是其父节点的右子节点
							pos->rotateR();
							tmppos = pos;
							pos = tmppos->pf;
						}
					}
					else {
						//父节点是祖父节点的右支
						if (getLR(tmppos)) {
							//插入节点是其父节点的左子节点
							pos->rotateL();
							tmppos = pos;
							pos = tmppos->pf;
						}
						else {
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
			return shared_from_this();
		}

		//在当前节点为根节点的树中删除一个节点，并返回删除后的根节点
		BRTreeNodePtr erase(BRTreeNodePtr& ndptr) {
			if (!ndptr) return shared_from_this();
			//先确定要删除的节点是自己的子节点
			BRTreeNode<T>* pos = ndptr.get();
			while (pos != NULL) {
				if (pos == this) break;
				pos = pos->pf;
			}
			if (pos == this) return _erase(ndptr);
			return shared_from_this();
		}

		BRTreeNodePtr erase(const T& val) {
			return _erase(binSearch<BRTreeNode<T>, T>(shared_from_this(), val));
		}
	private:
		BRTreeNodePtr _erase(BRTreeNodePtr& ndptr) {
			assert(pf == NULL);//自身需要是根节点
			if (!ndptr) return shared_from_this();
			BRTreeNodePtr proot = shared_from_this();//如果要删除的是自身，则需要一个指针来保存root节点
			BRTreeNode<T>* pos = ndptr->pf;
			if (!(ndptr->pl) && !(ndptr->pr)) {
				//左右都为空，为叶子节点
				if (ndptr->pf != NULL) {
					if (getLR(ndptr.get())) breakLChild(ndptr->pf);
					else breakRChild(ndptr->pf);
				}
				else {
					//整个树就一个要删除的根节点
					return BRTreeNodePtr();
				}
			}
			else if (ndptr->pl && !(ndptr->pr)) {
				//只有左子树
				if (ndptr->pf == NULL) {
					proot = ndptr->pl;
					breakLChild(ndptr.get());
				}
				else {
					ndptr->pl->pf = ndptr->pf;
					ndptr->pf->pl = ndptr->pl;
					ndptr->pf = NULL; ndptr->pl.reset();
				}
			}
			else if (!(ndptr->pl) && ndptr->pr) {
				//只有右子树
				if (ndptr->pf == NULL) {
					proot = ndptr->pr;
					breakRChild(ndptr.get());
				}
				else {
					ndptr->pr->pf = ndptr->pf;
					ndptr->pf->pr = ndptr->pr;
					ndptr->pf = NULL; ndptr->pr.reset();
				}
			}
			else {
				//换左子树的前驱
				BRTreeNodePtr tmp = ndptr->pl;
				if (tmp->pr) {
					//左子节点有右子树，找到其前驱
					while (tmp->pr) tmp = tmp->pr;
					tmp->pf->pr = tmp->pl;
					if (tmp->pl) tmp->pl->pf = tmp->pf;
					tmp->pl = ndptr->pl; ndptr->pl->pf = tmp.get();
					pos = tmp->pf;
				}
				else pos = tmp.get();
				tmp->pf = ndptr->pf;
				tmp->pr = ndptr->pr; ndptr->pr->pf = tmp.get();
				if (ndptr->pf != NULL) {
					if (getLR(ndptr.get())) ndptr->pf->pl = tmp;
					else ndptr->pf->pr = tmp;
				}
				else proot = tmp;
				ndptr->pf = NULL; ndptr->pl.reset(); ndptr->pr.reset();
				tmp->hgt = ndptr->hgt;
			}




		}
		//左旋转，顺时针
		BRTreeNodePtr rotateL() {
			BRTreeNodePtr re = pl;
			if (re->pr) re->pr->pf = this;
			pl = re->pr;
			if (pf == NULL) re->pr = shared_from_this();
			else {
				if (getLR(this)) {
					re->pr = pf->pl;
					pf->pl = re;
				}
				else {
					re->pr = pf->pr;
					pf->pr = re;
				}
			}
			re->pf = pf;
			pf = re.get();
			return re;
		}
		//右旋转，逆时针
		BRTreeNodePtr rotateR() {
			BRTreeNodePtr re = pr;
			if (re->pl) re->pl->pf = this;
			pr = re->pl;
			if (pf == NULL) re->pl = shared_from_this();
			else {
				if (getLR(this)) {
					re->pl = pf->pl;
					pf->pl = re;
				}
				else {
					re->pl = pf->pr;
					pf->pr = re;
				}
			}
			re->pf = pf;
			pf = re.get();
			return re;
		}

	};

	//一些外置的算法---------------------------------------------------------------------------

	//在二叉搜索树中进行查找
	template<typename NodeType, typename ValType>
	std::shared_ptr<NodeType> binSearch(const std::shared_ptr<NodeType>& proot, const ValType& val) {
		std::shared_ptr<NodeType> p = proot;
		while (p) {
			if (p->obj == val) return p;
			if (val < p->obj) p = p->pl;
			else if(val > p->obj) p = p->pr;
			else return std::shared_ptr<NodeType>();
		}
		return std::shared_ptr<NodeType>();
	}

	//以当前节点为根节点，前序遍历，返回一个指针数组。以当前节点为根节点
	template<typename T>
	void DLR(std::shared_ptr<T>& nd, std::vector<std::shared_ptr<T> >& vec) {
		assert(nd);
		vec.push_back(nd);
		if (nd->pl) DLR(nd->pl, vec);
		if (nd->pr) DLR(nd->pr, vec);
	}
	//中序遍历
	template<typename T>
	void LDR(std::shared_ptr<T>& nd, std::vector<std::shared_ptr<T> >& vec) {
		assert(nd);
		if (nd->pl) LDR(nd->pl, vec);
		vec.push_back(nd);
		if (nd->pr) LDR(nd->pr, vec);
	}
	//后续遍历
	template<typename T>
	void LRD(std::shared_ptr<T>& nd, std::vector<std::shared_ptr<T> >& vec) {
		assert(nd);
		if (nd->pl) LRD(nd->pl, vec);
		if (nd->pr) LRD(nd->pr, vec);
		vec.push_back(nd);
	}

	//获取深度，根节点深度为0
	template<typename T>
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

	//获取高度，叶子节点高度为1
	template<typename T>
	size_t getHeight(const T* pnode) {
		assert(pnode != NULL);
		size_t lh = 0, rh = 0;
		if (pnode->pl) lh = getHeight(pnode->pl.get());
		if (pnode->pr) rh = getHeight(pnode->pr.get());
		return max(lh, rh) + 1;
	}
	//获取树中节点个数
	template<typename T>
	size_t getNodeNum(const T* pnode) {
		assert(pnode != NULL);
		size_t num = 1;
		if (pnode->pl) num += getNodeNum(pnode->pl.get());
		if (pnode->pr) num += getNodeNum(pnode->pr.get());
		return num;
	}

	//将一个节点作为左/右子节点，与原左/右子节点断开。插入的节点与其原父节点断开
	template<typename T>
	void setLChild(T* pfather,std::shared_ptr<T>& pchild) {
		assert((pfather!=NULL) && pchild);
		pchild->pf = pfather;
		pfather->pl = pchild;
	}
	template<typename T>
	void setRChild(T* pfather, std::shared_ptr<T>& pchild) {
		assert((pfather != NULL) && pchild);
		pchild->pf = pfather;
		pfather->pr = pchild;
	}

	//与左/右子树断开
	template<typename T>
	void breakLChild(T* pnode) {
		assert((pnode != NULL) && pnode->pl);
		pnode->pl->pf = NULL;
		pnode->pl.reset();
	}
	template<typename T>
	void breakRChild(T* pnode) {
		assert((pnode != NULL) && pnode->pr);
		pnode->pr->pf = NULL;
		pnode->pr.reset();
	}

	//判断是父节点的左节点还是右节点。true表示左。使用前应检查父节点是否为空
	template<typename T>
	bool getLR(const T* pnode) {
		assert(pnode && pnode->pf != NULL);
		if (pnode == pnode->pf->pl.get()) return true;
		if (pnode == pnode->pf->pr.get()) return false;
		assert(0);//不是父节点的左右节点。报错
		return true;
	}

	//分层遍历
	template<typename T>
	void traByLevel(std::shared_ptr<T>& nd, std::vector<std::shared_ptr<T> >& vec) {
		assert(nd);
		size_t pos1 = vec.size(),pos2;
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

	//二叉树的序列化
	template<typename T>
	void SerializeTree(const std::shared_ptr<T>& proot, std::vector<std::shared_ptr<T> >& vec) {
		vec.push_back(proot);
		if (!proot)	return;
		SerializeTree(proot->pl, vec);
		SerializeTree(proot->pr, vec);
	}
	//反序列化
	template<typename T>
	void DeserializeTree(std::shared_ptr<T>& proot, typename std::vector<std::shared_ptr<T> >::iterator& itr) {
		if (*itr) {
			proot = *itr;
			DeserializeTree(proot->pl, ++itr);
			DeserializeTree(proot->pr, ++itr);
		}
	}
	//树的复制
	template<typename T>
	std::shared_ptr<T> copyTree(const std::shared_ptr<T>& proot) {
		std::shared_ptr<T> p = std::make_shared<T>(*proot);
		if (proot->pl) setLChild(p.get(), copyTree(proot->pl));
		if (proot->pr) setRChild(p.get(), copyTree(proot->pr));
		return p;
	}


}


