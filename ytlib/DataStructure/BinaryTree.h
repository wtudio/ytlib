#pragma once
#include <ytlib/Common/Util.h>
#include <memory>
#include <vector>


namespace ytlib {

	//一个BinTreeNode实例表示一个二叉树节点，也表示以此节点为根节点的一棵二叉树
	//如果根节点被析构，那么整个树中所有子节点将被析构，除非子节点有另外的智能指针指着
	template<typename T>
	class BinTreeNode {
	private:
		typedef std::shared_ptr<BinTreeNode<T> > nodePtr;
	public:
		BinTreeNode():pf(NULL){}
		virtual ~BinTreeNode(){}

		BinTreeNode(const T& _obj) :obj(_obj), pf(NULL) {

		}

		T obj;
		BinTreeNode<T>* pf;//父节点。父节点不可使用智能指针，否则会造成循环引用
		nodePtr pl;//左子节点
		nodePtr pr;//右子节点

		//与左/右子树断开
		void breakLChild() {
			pl->pf = NULL;
			pl.reset();
		}
		void breakRChild() {
			pr->pf = NULL;
			pr.reset();
		}

		//判断是父节点的左节点还是右节点。true表示左。使用前应检查父节点是否为空
		bool getLR() const {
			assert(pf != NULL);
			if (this == pf->pl.get()) return true;
			if (this == pf->pr.get()) return false;
			assert(0);//不是父节点的左右节点。报错
			return true;
		}

		//获取深度，根节点深度为0
		size_t getDepth() const {
			BinTreeNode<T>* tmp = pf;
			size_t count = 0;
			while (tmp!=NULL) {
				++count;
				tmp = tmp->pf;
			}
			return count;
		}

		//获取高度，叶子节点高度为1
		size_t getHeight() const {
			size_t lh = 0, rh = 0;
			if (pl) lh = pl->getHeight();
			if (pr) rh = pr->getHeight();

			return max(lh, rh) + 1;

		}
		//获取树中节点个数
		size_t getNodeNum() const {
			size_t num = 1;
			if (pl) num += pl->getNodeNum();
			if (pr) num += pr->getNodeNum();
			return num;
		}
	};

	//二叉查找树，T需要支持比较运算
	template<typename T>
	class BinSearchTreeNode : public BinTreeNode<T> {
	private:
		typedef std::shared_ptr<BinSearchTreeNode<T> > BSTNodePtr;
	public:
		BinSearchTreeNode():BinTreeNode<T>(){}
		virtual ~BinSearchTreeNode() {}

		BinSearchTreeNode(const T& _obj) :BinTreeNode<T>(_obj) {

		}

		//向当前节点为根节点的二叉查找树中插入一个节点
		virtual void insert(BSTNodePtr& ndptr) {
			assert(ndptr);
			if (ndptr->obj < obj) {
				if (pl) static_cast<BinSearchTreeNode<T>*>(pl.get())->insert(ndptr);
				else setLChild(this,ndptr);
			}
			else {
				if (pr) static_cast<BinSearchTreeNode<T>*>(pr.get())->insert(ndptr);
				else setRChild(this, ndptr);
			}
		}

		//删除当前节点。
		virtual void erase() {
			
			if (!pl && !pr) {
				//左右都为空，为叶子节点
				if (pf == NULL) return;
				if (getLR()) pf->breakLChild();//todo:测试这样调用会不会有问题
				else pf->breakRChild();

			}
			else if (pl && !pr) {
				//只有左子树
				if (pf == NULL) breakLChild();
				else {
					pl->pf = pf;
					pf->pl = pl;
					pf = NULL;
					pl.reset();
				}

			}
			else if (!pl && pr) {
				//只有右子树
				if (pf == NULL) breakRChild();
				else {
					pr->pf = pf;
					pf->pr = pr;
					pf = NULL;
					pr.reset();
				}

			}
			else {
				//换左子树的前驱
				std::shared_ptr<BinTreeNode<T> > tmp = pl;
				if (tmp->pr) {
					//左子节点有右子树，找到其前驱
					while (tmp->pr) {
						tmp = tmp->pr;
					}
					
					tmp->pf->pr = tmp->pl;
					if (tmp->pl) tmp->pl->pf = tmp->pf;
					tmp->pl = pl; pl->pf = tmp.get();

				}

				tmp->pf = pf;
				tmp->pr = pr; pr->pf = tmp.get();
				if (pf) {
					if (getLR()) pf->pl = tmp;
					else pf->pr = tmp;
				}
				pf = NULL;
				pl.reset();
				pr.reset();

			}

		}

	};

#define AVLTPTR(sp) static_cast<AVLTreeNode<T>*>(sp.get())	//将BinTreeNode指针转换为AVL树的指针
	//AVL树
	template<typename T>
	class AVLTreeNode : public BinSearchTreeNode <T>{
	private:
		typedef std::shared_ptr<AVLTreeNode<T> > AVLTNodePtr;
	public:
		AVLTreeNode() :AVLTreeNode<T>(), hgt(1) {}
		virtual ~AVLTreeNode() {}

		AVLTreeNode(const T& _obj) :BinSearchTreeNode<T>(_obj), hgt(1) {

		}

		size_t hgt;//节点高度

		virtual void insert(AVLTNodePtr& ndptr) {
			assert(ndptr);
			if (ndptr->obj < obj) {
				if (pl) { 
					AVLTPTR(pl)->insert(ndptr);

					size_t Rhgt = 0;
					if (pr) Rhgt = AVLTPTR(pr)->hgt;
					if (2 == AVLTPTR(pl)->hgt - Rhgt) {
						if (ndptr->obj < AVLTPTR(pl)->obj) {
							rotateL();
						}
						else {
							AVLTPTR(pr)->rotateR();
							rotateL();
						}
					}

				}
				else {
					setLChild(this, ndptr);
					ndptr->updateHeight();
				}



			}
			else {
				if (pr) AVLTPTR(pr)->insert(ndptr);
				else {
					setRChild(this, ndptr);
					ndptr->updateHeight();
				}
			}

		}

		virtual void erase() {

		}

		//左旋转，顺时针
		void rotateL() {

		}
		//右旋转，逆时针
		void rotateR() {

		}

		//由子节点向上更新高度
		void updateHeight() {
			AVLTreeNode<T>* tmp = this;
			while (tmp->pf != NULL) {
				if (tmp->pf->hgt == (tmp->hgt + 1)) break;
				tmp->pf->hgt = (tmp->hgt + 1);
				tmp = tmp->pf;
			}
		}
	};

	//红黑树
	template<typename T>
	class BRTreeNode {

	};

	//以当前节点为根节点，前序遍历，返回一个指针数组。以当前节点为根节点
	template<typename T>
	void DLR(std::shared_ptr<T>& nd, std::vector<std::shared_ptr<T> >& vec) {
		vec.push_back(nd);
		if (nd->pl) DLR<T>(std::static_pointer_cast<T>(nd->pl), vec);
		if (nd->pr) DLR<T>(std::static_pointer_cast<T>(nd->pr), vec);
	}
	//中序遍历
	template<typename T>
	void LDR(std::shared_ptr<T>& nd, std::vector<std::shared_ptr<T> >& vec) {
		if (nd->pl) LDR<T>(std::static_pointer_cast<T>(nd->pl), vec);
		vec.push_back(nd);
		if (nd->pr) LDR<T>(std::static_pointer_cast<T>(nd->pr), vec);
	}
	//后续遍历
	template<typename T>
	void LRD(std::shared_ptr<T>& nd, std::vector<std::shared_ptr<T> >& vec) {
		if (nd->pl) LRD<T>(std::static_pointer_cast<T>(nd->pl), vec);
		if (nd->pr) LRD<T>(std::static_pointer_cast<T>(nd->pr), vec);
		vec.push_back(nd);
	}

	//将一个节点作为左/右子节点（与原左/右子节点断开。插入的节点与其原父节点断开）
	template<typename T>
	void setLChild(T* pfather,std::shared_ptr<T>& pchild) {
		assert(pfather && pchild);
		pchild->pf = pfather;
		pfather->pl = pchild;
	}
	template<typename T>
	void setRChild(T* pfather, std::shared_ptr<T>& pchild) {
		assert(pfather && pchild);
		pchild->pf = pfather;
		pfather->pr = pchild;
	}

}


