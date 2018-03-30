#pragma once
#include <ytlib/Common/Util.h>
#include <memory>
#include <vector>


namespace ytlib {

	//全部使用模板化。这里不能使用继承，因为有成员是自身类型的智能指针
	//一个BinTreeNode实例表示一个二叉树节点，也表示以此节点为根节点的一棵二叉树
	//如果根节点被析构，那么整个树中所有子节点将被析构，除非子节点有另外的智能指针指着
	template<typename T>
	class BinTreeNode {
	private:
		typedef std::shared_ptr<BinTreeNode<T> > nodePtr;
	public:
		BinTreeNode():pf(NULL){}
		~BinTreeNode(){}
		BinTreeNode(const T& _obj) :obj(_obj), pf(NULL) {}

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
		~BinSearchTreeNode() {}
		BinSearchTreeNode(const T& _obj) :obj(_obj), pf(NULL) {}

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

		//删除当前节点。
		void erase() {
			if (!pl && !pr) {
				//左右都为空，为叶子节点
				if (pf == NULL) return;
				if (getLR(this)) breakLChild(pf);
				else breakRChild(pf);
			}
			else if (pl && !pr) {
				//只有左子树
				if (pf == NULL) breakLChild(this);
				else {
					pl->pf = pf;
					pf->pl = pl;
					pf = NULL;pl.reset();
				}
			}
			else if (!pl && pr) {
				//只有右子树
				if (pf == NULL) breakRChild(this);
				else {
					pr->pf = pf;
					pf->pr = pr;
					pf = NULL;pr.reset();
				}
			}
			else {
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
			}
		}

	};

	//AVL树
	template<typename T>
	class AVLTreeNode :public std::enable_shared_from_this<AVLTreeNode<T> > {
	private:
		typedef std::shared_ptr<AVLTreeNode<T> > AVLTNodePtr;
	public:
		AVLTreeNode() :pf(NULL), hgt(1){}
		~AVLTreeNode() {}
		AVLTreeNode(const T& _obj) :obj(_obj), pf(NULL), hgt(1) {}

		T obj;
		AVLTreeNode<T>* pf;//父节点
		AVLTNodePtr pl;//左子节点
		AVLTNodePtr pr;//右子节点

		size_t hgt;//节点高度

		//插入，因为根节点可能会变，所以返回根节点
		AVLTNodePtr insert(AVLTNodePtr& ndptr) {
			assert(ndptr);
			AVLTNodePtr re;
			if (ndptr->obj < obj) {
				if (pl) { 
					pl->insert(ndptr);
					size_t Rhgt = pr ? pr->hgt : 0;
					if (2 == (pl->hgt - Rhgt)) {
						//左边比右边高了2
						if (ndptr->obj < pl->obj) re = rotateL();
						else {
							pl->rotateR();
							re = rotateL();
						}
					}
				}
				else {
					setLChild(this, ndptr);
					ndptr->updateHeight();
				}
			}
			else {
				if (pr) {
					pr->insert(ndptr);
					size_t Lhgt = pl ? pl->hgt : 0;
					if (2 == (pr->hgt - Lhgt)) {
						//左边比右边低了2
						if (ndptr->obj > pr->obj) re = rotateR();
						else {
							pr->rotateL();
							re = rotateR();
						}
					}
				}
				else {
					setRChild(this, ndptr);
					ndptr->updateHeight();
				}
			}
			if(re)	return re;
			return shared_from_this();
		}

		//删除当前节点。todo待完善
		void erase() {
			if (!pl && !pr) {
				//左右都为空，为叶子节点
				if (pf == NULL) return;
				if (getLR(this)) breakLChild(pf);
				else breakRChild(pf);
			}
			else if (pl && !pr) {
				//只有左子树
				if (pf == NULL) breakLChild(this);
				else {
					pl->pf = pf;
					pf->pl = pl;
					pf = NULL; pl.reset();
				}
			}
			else if (!pl && pr) {
				//只有右子树
				if (pf == NULL) breakRChild(this);
				else {
					pr->pf = pf;
					pf->pr = pr;
					pf = NULL; pr.reset();
				}
			}
			else {
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
				pf = NULL; pl.reset(); pr.reset();
			}

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
			re->hgt = 0;//这里高度要重新计算
			updateHeight();
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
			re->hgt = 0;
			updateHeight();
			return re;
		}

		//由子节点向上更新高度
		void updateHeight() {
			AVLTreeNode<T>* tmp = this;
			while (tmp != NULL) {
				size_t lh = (tmp->pl) ? tmp->pl->hgt : 0;
				size_t lr = (tmp->pr) ? tmp->pr->hgt : 0;
				size_t h = max(lh, lr) + 1;
				if (tmp->hgt == h && tmp != this) break;
				tmp->hgt = h;
				tmp = tmp->pf;
			}
		}
	};

	//红黑树。todo待完善
	template<typename T>
	class BRTreeNode {

	};

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


	}


}


